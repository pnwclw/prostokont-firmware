if(CONFIG_INKPLATE_BOARD_INKPLATE10)
    set(BOARD_SRC "src/panels/inkplate/Inkplate10.cpp")
    set(IMAGE_SRCS "src/graphics/Image.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE6)
    set(BOARD_SRC "src/panels/inkplate/Inkplate6.cpp" "src/features/I2S.cpp")
    set(IMAGE_SRCS "src/graphics/Image.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK)
    set(BOARD_SRC "src/panels/inkplate/Inkplate6.cpp" "src/features/I2S.cpp")
    set(IMAGE_SRCS "src/graphics/Image.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
    set(BOARD_SRC "src/panels/inkplate/Inkplate6Color.cpp" "src/features/SPI.cpp")
    set(IMAGE_SRCS "src/graphics/ImageColor.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE13)
    set(BOARD_SRC "src/panels/inkplate/Inkplate13.cpp" "src/features/SPI.cpp")
    set(IMAGE_SRCS "src/graphics/ImageColor.cpp")
elseif(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
    set(BOARD_SRC "src/panels/waveshare/Waveshare13.cpp" "src/features/SPI.cpp")
    set(IMAGE_SRCS "src/graphics/ImageColor.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE5)
    set(BOARD_SRC "src/panels/inkplate/Inkplate5.cpp")
    set(IMAGE_SRCS "src/graphics/Image.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE4)
    set(BOARD_SRC "src/panels/inkplate/Inkplate4.cpp")
    set(IMAGE_SRCS "src/graphics/Image.cpp")
    set(SENSOR_SRCS
        "src/features/APDS9960/APDS9960.cpp"
        "src/features/APDS9960/libs/APDS9960Driver.cpp"
        "src/features/BME680/BME680.cpp"
        "src/features/BME680/libs/BME680Driver.cpp"
        "src/features/BQ27441/BQ27441.cpp"
        "src/features/BQ27441/libs/BQ27441Driver.cpp"
        "src/features/LSM6DS3/LSM6DS3.cpp"
        "src/features/LSM6DS3/libs/LSM6DS3Driver.cpp")
    set(SENSOR_INCLUDES
        "include/features/APDS9960"
        "include/features/APDS9960/libs"
        "include/features/BME680"
        "include/features/BME680/libs"
        "include/features/BQ27441"
        "include/features/BQ27441/libs"
        "include/features/LSM6DS3"
        "include/features/LSM6DS3/libs")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE2)
    set(BOARD_SRC "src/panels/inkplate/Inkplate2.cpp")
    set(IMAGE_SRCS "src/graphics/ImageColor.cpp")
endif()

set(OTA_SRCS
    "src/services/ota/OTA.cpp"
)

set(OTA_INCLUDES
    "include/services/ota/OTA.hpp"
)

set(SERVICE_SRCS
    "src/services/app/ConnectivityCoordinator.cpp"
    "src/services/ble/BleProvisioningService.cpp"
    "src/services/display/DisplayService.cpp"
    "src/services/device/DeviceIdentity.cpp"
    "src/services/http/HttpServer.cpp"
    "src/services/network/HttpClient.cpp"
    "src/services/network/MdnsService.cpp"
    "src/services/storage/Storage.cpp"
    "src/services/time/TimeSyncService.cpp"
    "src/services/wifi/WifiManager.cpp"
)

set(DECODER_INCLUDES
    "include/graphics/decoders"
    "include/graphics/decoders/pngle"
)

set(DECODER_SRCS
    "src/graphics/decoders/BMP.cpp"
    "src/graphics/decoders/JPEG.cpp"
    "src/graphics/decoders/PNG.cpp"
    "src/graphics/decoders/pngle/pngle.c"
    "src/graphics/decoders/pngle/miniz.c"
    "src/graphics/qrcodegen/qrcodegen.c"
)

set(GRAPHICS_SRCS
    "src/graphics/Graphics.cpp"
    "src/graphics/Adafruit_GFX/Adafruit_GFX.cpp"
    "src/graphics/Shapes.cpp"
)

set(GRAPHICS_INCLUDES
    "include/graphics"
    "include/graphics/Adafruit_GFX"
    "include/graphics/qrcodegen"
)
if(CONFIG_INKPLATE_BOARD_INKPLATE2)
idf_component_register(SRCS "src/features/SPI.cpp"
                            "src/Display.cpp" "src/features/I2C.cpp"
                            ${BOARD_SRC}
                            ${SERVICE_SRCS}
                            ${GRAPHICS_SRCS}
                            ${IMAGE_SRCS}
                            ${DECODER_SRCS}
                       REQUIRES "esp_driver_gpio"
                                "esp_driver_spi"
                                "esp_timer"
                                "esp_wifi" "nvs_flash" "esp_http_client" "esp_http_server" "esp_netif" "mdns"
                                "bt"
                       INCLUDE_DIRS "include"
                                    "include/features"
                                    "include/panels"
                                    "include/panels/inkplate"
                                    "include/panels/waveshare"
                                    ${GRAPHICS_INCLUDES}
                                    ${DECODER_INCLUDES}
                                    )
elseif(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
idf_component_register(SRCS "src/Display.cpp"
                            "src/panels/PanelCommon.cpp"
                            ${BOARD_SRC}
                            ${SERVICE_SRCS}
                            ${GRAPHICS_SRCS}
                            ${IMAGE_SRCS}
                            ${DECODER_SRCS}
                       REQUIRES "esp_driver_gpio"
                                "esp_driver_spi"
                                "esp_timer"
                                "esp_adc"
                                "esp_wifi" "nvs_flash" "esp_http_client" "esp_http_server" "esp_netif" "mdns"
                                "bt"
                       INCLUDE_DIRS "include"
                                    "include/features"
                                    "include/panels"
                                    "include/panels/waveshare"
                                    ${GRAPHICS_INCLUDES}
                                    ${DECODER_INCLUDES}
                                    )
else()
idf_component_register(SRCS "src/features/I2C.cpp" "src/features/RTC.cpp" "src/features/Frontlight.cpp" 
                            "src/features/PCAL.cpp" "src/features/TPS.cpp" "src/features/SPI.cpp"
                            "src/features/SDCard.cpp"
                            "src/panels/PanelCommon.cpp" "src/features/TouchCypress.cpp" "src/features/TouchElan.cpp"
                            "src/Display.cpp"
                            ${SERVICE_SRCS}
                            ${BOARD_SRC}
                            ${SENSOR_SRCS}
                            ${GRAPHICS_SRCS}
                            ${IMAGE_SRCS}
                            ${DECODER_SRCS}
                       REQUIRES "esp_driver_i2c" "esp_driver_i2s" "esp_driver_gpio" "esp_timer" "esp_adc"
                                "esp_driver_spi" "fatfs" "sdmmc"
                                "esp_wifi" "nvs_flash" "esp_http_client" "esp_http_server" "esp_netif" "mdns"
                                "bt"
                       INCLUDE_DIRS "include" 
                                    "include/features"
                                    "include/panels"
                                    "include/panels/inkplate"
                                    "include/panels/waveshare"
                                    ${GRAPHICS_INCLUDES}
                                    ${SENSOR_INCLUDES}
                                    ${DECODER_INCLUDES}
                                    )
endif()
