cmake_minimum_required(VERSION 3.10.2)

include_directories(
        include
        src
        src/hal
        src/arch/linux
)

aux_source_directory(. SOURCE)



add_library(
             native-lib
             SHARED
             src/rplidar_driver.cpp
             src/hal/thread.cpp
             src/arch/linux/net_serial.cpp
             src/arch/linux/net_socket.cpp
             src/arch/linux/timer.cpp
             ${SOURCE} )




find_library(
              log-lib

              log )

target_link_libraries(
                       native-lib

                       ${log-lib} )