package com.pudutech.lidartest

import android.Manifest
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.annotation.RequiresApi

class MainActivity : AppCompatActivity() {
    lateinit var lidar:LidarHelper
    lateinit var lidarStateTv:TextView
    lateinit var lidarDataTv:TextView
    lateinit var lidarDeviceInfoTv:TextView
    private lateinit var canBus: CanBus
    @RequiresApi(Build.VERSION_CODES.M)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)==0){
            requestPermissions(Array<String>(1) {
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            },10)
        }
        lidarStateTv = findViewById(R.id.lidarStateTv)
        lidarDataTv = findViewById(R.id.dataTv)
        lidarDeviceInfoTv = findViewById(R.id.lidarDeviceInfoTv)
        lidar = LidarHelper()
        lidar.setOnLidarListener(lidarListener)
        canBus = CanBus(this@MainActivity)
    }

    private var lidarListener = object:LidarHelper.OnLidarListener{
        override fun onState(state: Int) {
            lidarStateTv.post {
                when (state){
                    -1-> lidarStateTv.text = "雷达状态：创建失败"
                    0 -> lidarStateTv.text = "雷达状态：创建成功"
                    1-> lidarStateTv.text = "雷达状态：连接成功"
                    2->lidarStateTv.text = "雷达状态：连接失败"
                    3->lidarStateTv.text = "雷达状态：获取雷达设备信息成功"
                    4->lidarStateTv.text = "雷达状态：获取雷达设备信息失败"
                    6->{
                        lidarStateTv.text = "雷达状态：已停止工作"
                        lidarDataTv.text = ""
                    }
                }

            }
        }

        override fun onDataResult(flag: Int, degree: Float, distance: Double, quality: Int) {
            lidarDataTv.post{
                lidarDataTv.text = "flag=$flag,degree=$degree,distance=$distance,quality=$quality"
            }
        }

        override fun onDeviceInfo(model: Int, firmwareVersion: Int, hardwareVersion: Int, serialnum: String) {
            Log.e("zbq","java receive deviceInfo:$model,$firmwareVersion,$hardwareVersion,$serialnum")
        }
    }
    fun initLidar(view: View) {
        lidar.initLidar()
    }
    fun startLidar(view: View) {
        lidar.startLidar()
    }
    fun stopLidar(view: View) {
        lidar.stopLidar()
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    fun startCan(view: View) {
        canBus.startCan()
    }
    fun stopCan(view: View) {
        canBus.stopCan()
    }

}