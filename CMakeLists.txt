cmake_minimum_required(VERSION 3.8)
project(simple_wifi)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set(CMAKE_CXX_STANDARD 11)

include_directories(/home/muntahi/esp/esp-idf/components/newlib/include

        /home/muntahi/esp/esp-idf/components/freertos/include
        /home/muntahi/esp/esp-idf/components/esp32/include
        /home/muntahi/esp/esp-idf/components/nvs_flash/include
        /home/muntahi/esp/esp-idf/components/lwip/include/lwip
        #/home/muntahi/esp/esp-idf/components/lwip/include/lwip/lwip

        #/home/muntahi/esp/esp-idf/components/spiffs/spiffs/src


        /home/muntahi/esp/esp-idf/components/tcpip_adapter/include/
        /home/muntahi/esp/esp-idf/components/driver/include/
        /home/muntahi/esp/esp-idf/components/soc/esp32/include/


        /home/muntahi/esp/esp-idf/components/log/include
        /home/muntahi/esp/simple_wifi/build/include
        /home/muntahi/esp/simple_wifi/main)



add_executable(simple_wifi main/simple_wifi.cpp main/apa102.cpp)
