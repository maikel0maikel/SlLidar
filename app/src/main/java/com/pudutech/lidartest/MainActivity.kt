package com.pudutech.lidartest

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.TextView

class MainActivity : AppCompatActivity() {
    lateinit var lidar:LidarHelper
    lateinit var lidarStateTv:TextView
    lateinit var lidarDataTv:TextView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        lidarStateTv = findViewById(R.id.lidarStateTv)
        lidarDataTv = findViewById(R.id.dataTv)
        lidar = LidarHelper()
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
    fun unInitLidar(view: View) {
        lidar.unInitLidar()
    }


}