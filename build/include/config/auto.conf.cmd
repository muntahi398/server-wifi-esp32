deps_config := \
	/home/muntahi/esp/esp-idf/components/app_trace/Kconfig \
	/home/muntahi/esp/esp-idf/components/aws_iot/Kconfig \
	/home/muntahi/esp/esp-idf/components/bt/Kconfig \
	/home/muntahi/esp/esp-idf/components/driver/Kconfig \
	/home/muntahi/esp/esp-idf/components/esp32/Kconfig \
	/home/muntahi/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/muntahi/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/muntahi/esp/esp-idf/components/ethernet/Kconfig \
	/home/muntahi/esp/esp-idf/components/fatfs/Kconfig \
	/home/muntahi/esp/esp-idf/components/freertos/Kconfig \
	/home/muntahi/esp/esp-idf/components/heap/Kconfig \
	/home/muntahi/esp/esp-idf/components/libsodium/Kconfig \
	/home/muntahi/esp/esp-idf/components/log/Kconfig \
	/home/muntahi/esp/esp-idf/components/lwip/Kconfig \
	/home/muntahi/esp/esp-idf/components/mbedtls/Kconfig \
	/home/muntahi/esp/esp-idf/components/mdns/Kconfig \
	/home/muntahi/esp/esp-idf/components/openssl/Kconfig \
	/home/muntahi/esp/esp-idf/components/pthread/Kconfig \
	/home/muntahi/esp/esp-idf/components/spi_flash/Kconfig \
	/home/muntahi/esp/esp-idf/components/spiffs/Kconfig \
	/home/muntahi/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/muntahi/esp/esp-idf/components/vfs/Kconfig \
	/home/muntahi/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/muntahi/esp/esp-idf/Kconfig.compiler \
	/home/muntahi/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/muntahi/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/muntahi/esp/simple_wifi/main/Kconfig.projbuild \
	/home/muntahi/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/muntahi/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
