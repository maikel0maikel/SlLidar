package com.pudutech.lidartest

class LidarHelper {
   private var  nativePtr:Long = 0
   private var mLidarListener:OnLidarListener? = null
    init {
        System.loadLibrary("native-lib")
    }

    fun initLidar():Boolean{
        nativePtr = init()
        return nativePtr!= 0.toLong()
    }

    fun startLidar(){
        if (nativePtr==0.toLong()){
            mLidarListener?.onState(-1)
            return
        }
        start(nativePtr)
    }

    fun stopLidar(){
        if (nativePtr==0.toLong()){
            mLidarListener?.onState(-1)
            return
        }
        stop(nativePtr)
    }


    fun setOnLidarListener(l:OnLidarListener){
        mLidarListener = l
    }

    fun onLidarState(state: Int){
        mLidarListener?.onState(state)
    }

    fun onDataResult(flag:Int,degree:Float,distance:Double,quality:Int){
        mLidarListener?.onDataResult(flag,degree,distance,quality)
    }
    fun onDeviceInfo(model:Int,firmwareVersion:Int,hardwareVersion:Int,serialnum:String){
        mLidarListener?.onDeviceInfo(model,firmwareVersion,hardwareVersion,serialnum)
    }

    private external fun init():Long


    private external fun start(nativePtr:Long)

    private external fun stop(nativePtr:Long)


    interface OnLidarListener{
        fun onState(state:Int)
        fun onDataResult(flag:Int,degree:Float,distance:Double,quality:Int)
        fun onDeviceInfo(model:Int,firmwareVersion:Int,hardwareVersion:Int,serialnum:String)
    }
}