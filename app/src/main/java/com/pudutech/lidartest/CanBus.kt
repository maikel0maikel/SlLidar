package com.pudutech.lidartest

import android.content.Context
import android.net.LocalSocket
import android.net.LocalSocketAddress
import android.os.SystemClock
import android.text.TextUtils
import android.util.Log
import android.util.Pair
import kotlinx.coroutines.*
import java.io.*
import java.util.*
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicReference

class CanBus(val appContext: Context) {
    private val TAG = "zbq"
    private var isConnect = false
    private var socketOutputStream: OutputStream? = null
    private var socketInputStream: InputStream? = null
    private var connectJob:Job? = null
    private var sendJob:Job? = null
    private var receiveJob:Job? = null

    private var hasSendStart:AtomicBoolean = AtomicBoolean(false)
    private var hasReceiveStart:AtomicBoolean = AtomicBoolean(false)

    fun startCan() {
        connectCan();
    }

    private fun connectCan() {
        connectJob = GlobalScope.launch {
            val use_can_service_usb = android.os.Build.VERSION.SDK_INT > 22
            Log.i(TAG, "release can_service from assets, use_can_service_usb:$use_can_service_usb")
            val path = appContext.filesDir.absolutePath
            try {
                val inputStream =
                    appContext.assets.open(if (use_can_service_usb) "can_service_usb" else "can_service_socket")
                val file = File("$path/can_service")
                if (!file.exists())
                    file.createNewFile()
                val fileOutputStream = FileOutputStream(file)
                val buffer = ByteArray(1024)
                while (true) {
                    val len = inputStream.read(buffer)
                    if (len <= 0) {
                        break
                    }
                    fileOutputStream.write(buffer, 0, len)
                }
                fileOutputStream.flush()
                fileOutputStream.close()
                inputStream.close()

            } catch (e: FileNotFoundException) {
                Log.i(TAG, "can_service can not be overwrite, maybe busy")
            } catch (e: IOException) {
                Log.e(TAG, "put can_service fail error:${e.message}")
            }
            Log.i(TAG, "release can_service success")

            //killMapifyIfAny(use_can_service_usb)
            killUnusedExe("can_service")

            execCommand("chmod 777 $path/can_service", use_can_service_usb)
            //这里如果必须得等一下，否则会导致后面can_service启动失败，可能是因为cam_service占用问题
            val current_time = SystemClock.elapsedRealtime()
            execCommand("$path/can_service $current_time", use_can_service_usb)
            Log.i(TAG, "connecting can_service")
            val socket = LocalSocket()
            if (withTimeoutOrNull(5000) {
                    while (isActive) {
                        try {
                            socket.connect(LocalSocketAddress("/pudu/can_service${if (use_can_service_usb) current_time else ""}"))
                            socketOutputStream = socket.outputStream
                            socketInputStream = socket.inputStream
                            Log.d(TAG, "connect succeed")
                            isConnect = true
                            startReceive()
                            startSend()
                            break
                        } catch (e: IOException) {
                            Log.e(TAG, "connect fail error:${e.message}" )
                        }
                        delay(500)
                    }
                    true
                } == true) {
            } else {
                Log.e(TAG, "connect CAN service timeout")
                isConnect = false
            }
        }
    }

    private fun killUnusedExe(exe: String) {
        val res = execCommand("ps -A | grep $exe", false)
        Log.e(TAG, "killUnusedExe $exe")
        if (res?.first == 0 && res.second!!.contains(exe)) {
            val strings = res!!.second?.split("\n")?.toTypedArray()
            for (i in strings!!.indices) {
                val line = strings[i]
                // get pid
                val fields = line.split("\\s").toTypedArray()
                Log.i(TAG, "fields: " + Arrays.toString(fields))
                if (fields.size <= 1) {
                    continue
                }
                var pidStr: String? = null
                var j = 1
                while (j < fields.size && pidStr == null) {
                    if (TextUtils.isEmpty(fields[j].trim { it <= ' ' })) {
                        j++
                        continue
                    }
                    pidStr = fields[j]
                    j++
                }
                try {
                    val pid = pidStr!!.toInt()
                    execCommand("kill $pid", true)
                } catch (ex: java.lang.Exception) {
                    Log.e(TAG, "parse pid failed", ex)
                }
            }
        }
    }



    fun stopCan() {
        runBlocking {
            if (receiveJob!=null){
                receiveJob?.cancelAndJoin()
                receiveJob = null
            }
        }
        runBlocking {
            if (sendJob!=null){
                sendJob?.cancelAndJoin()
                sendJob = null
            }
        }
    }

    fun execCommand(command: String, use_su: Boolean): Pair<Int, String?>? {
        Log.d(TAG, "execCommand:" + command + if (use_su) " with su" else " with sh")
        val cmdStrings = arrayOf(if (use_su) "su" else "sh", "-c", command)
        val runtime = Runtime.getRuntime()
        try {
            val proc = runtime.exec(cmdStrings)
            waitFor(30, TimeUnit.SECONDS, proc)
            val inputStream = proc.inputStream
            val scanner = Scanner(inputStream)
            val sb = StringBuilder()
            while (scanner.hasNextLine()) {
                sb.append(scanner.nextLine()).append("\n")
            }
            inputStream.close()
            val exitValue = proc.exitValue()
            Log.d(TAG, "su exit value = $exitValue $sb")
            return Pair.create(exitValue, sb.toString())
        } catch (e: Throwable) {
            Log.w(TAG, e)
        }
        return Pair.create(1, null)
    }

    @Throws(InterruptedException::class)
    fun waitFor(
        timeout: Long,
        unit: TimeUnit,
        process: Process): Boolean {
        val startTime = System.nanoTime()
        var rem = unit.toNanos(timeout)
        do {
            try {
                process.exitValue()
                return true
            } catch (ex: IllegalThreadStateException) {
                if (rem > 0) Thread.sleep(
                        Math.min(TimeUnit.NANOSECONDS.toMillis(rem) + 1, 100)
                )
            }
            rem = unit.toNanos(timeout) - (System.nanoTime() - startTime)
        } while (rem > 0)
        return false
    }
    val bytes = ByteArray(8)
    private fun startReceive(){
        if (hasReceiveStart.get()){
            return
        }
        hasReceiveStart.set(true)
       receiveJob = GlobalScope.launch {
           while (isActive){
               try {
                   val count = socketInputStream?.read(bytes, 0, 8)
                   if (count == 8) {
                       onRecv(bytes[0].toInt(), bytes.toUByteArray())
                   } else {
                       Log.e(TAG, "recv CAN size error:$count")
                   }
               } catch (e: Exception) {
                   Log.e(TAG, "recv exception: ${e.message}" )
                   break
               }
           }
       }
    }

    private fun onRecv(id: Int, bytes: UByteArray){
        Log.d(TAG, "recv:${id.toUByte()} ${bytes.toHexString()}")
    }
    private fun startSend(){

    }

    @ExperimentalUnsignedTypes
    fun UByteArray.toHexString() = joinToString(" ") { it.toString(16).padStart(2, '0') }
}