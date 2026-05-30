function(select_board_sources board_srcs image_srcs sensor_srcs sensor_includes)
    set(_board_srcs)
    set(_image_srcs)
    set(_sensor_srcs)
    set(_sensor_includes)

    if(CONFIG_INKPLATE_BOARD_INKPLATE10)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate10.cpp")
        list(APPEND _image_srcs "src/graphics/Image.cpp")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE6)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate6.cpp" "src/features/I2S.cpp")
        list(APPEND _image_srcs "src/graphics/Image.cpp")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate6.cpp" "src/features/I2S.cpp")
        list(APPEND _image_srcs "src/graphics/Image.cpp")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate6Color.cpp")
        list(APPEND _image_srcs "src/graphics/ImageColor.cpp")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE13)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate13.cpp")
        list(APPEND _image_srcs "src/graphics/ImageColor.cpp")
    elseif(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
        list(APPEND _board_srcs "src/panels/waveshare/Waveshare13.cpp" "src/features/SPI.cpp")
        list(APPEND _image_srcs "src/graphics/ImageColor.cpp")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE5)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate5.cpp")
        list(APPEND _image_srcs "src/graphics/Image.cpp")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE4)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate4.cpp")
        list(APPEND _image_srcs "src/graphics/Image.cpp")
        list(APPEND _sensor_srcs
            "src/features/APDS9960/APDS9960.cpp"
            "src/features/APDS9960/libs/APDS9960Driver.cpp"
            "src/features/BME680/BME680.cpp"
            "src/features/BME680/libs/BME680Driver.cpp"
            "src/features/BQ27441/BQ27441.cpp"
            "src/features/BQ27441/libs/BQ27441Driver.cpp"
            "src/features/LSM6DS3/LSM6DS3.cpp"
            "src/features/LSM6DS3/libs/LSM6DS3Driver.cpp")
        list(APPEND _sensor_includes
            "include/features/APDS9960"
            "include/features/APDS9960/libs"
            "include/features/BME680"
            "include/features/BME680/libs"
            "include/features/BQ27441"
            "include/features/BQ27441/libs"
            "include/features/LSM6DS3"
            "include/features/LSM6DS3/libs")
    elseif(CONFIG_INKPLATE_BOARD_INKPLATE2)
        list(APPEND _board_srcs "src/panels/inkplate/Inkplate2.cpp")
        list(APPEND _image_srcs "src/graphics/ImageColor.cpp")
    endif()

    set(${board_srcs} ${_board_srcs} PARENT_SCOPE)
    set(${image_srcs} ${_image_srcs} PARENT_SCOPE)
    set(${sensor_srcs} ${_sensor_srcs} PARENT_SCOPE)
    set(${sensor_includes} ${_sensor_includes} PARENT_SCOPE)
endfunction()

set(SERVICE_SRCS
    "src/services/app/ConnectivityCoordinator.cpp"
    "src/services/ble/BleProvisioningService.cpp"
    "src/services/display/DisplayService.cpp"
    "src/services/device/DeviceIdentity.cpp"
    "src/services/device/StatusPayloadJson.cpp"
    "src/services/http/HttpServer.cpp"
    "src/services/network/HttpClient.cpp"
    "src/services/network/MdnsService.cpp"
    "src/services/storage/Storage.cpp"
    "src/services/time/TimeSyncService.cpp"
    "src/services/wifi/WifiManager.cpp")

set(DECODER_SRCS
    "src/graphics/decoders/BMP.cpp"
    "src/graphics/decoders/JPEG.cpp"
    "src/graphics/decoders/PNG.cpp"
    "src/graphics/decoders/pngle/pngle.c"
    "src/graphics/decoders/pngle/miniz.c"
    "src/graphics/qrcodegen/qrcodegen.c")

set(DECODER_INCLUDES
    "include/graphics/decoders"
    "include/graphics/decoders/pngle")

set(GRAPHICS_SRCS
    "src/graphics/Graphics.cpp"
    "src/graphics/Adafruit_GFX/Adafruit_GFX.cpp"
    "src/graphics/Shapes.cpp")

set(GRAPHICS_INCLUDES
    "include/graphics"
    "include/graphics/Adafruit_GFX"
    "include/graphics/qrcodegen")

select_board_sources(BOARD_SRCS IMAGE_SRCS SENSOR_SRCS SENSOR_INCLUDES)

set(COMPONENT_SRCS
    "src/Display.cpp"
    ${BOARD_SRCS}
    ${SERVICE_SRCS}
    ${GRAPHICS_SRCS}
    ${IMAGE_SRCS}
    ${DECODER_SRCS}
    ${SENSOR_SRCS})

set(COMPONENT_REQUIRES
    "esp_driver_gpio"
    "esp_driver_spi"
    "esp_timer"
    "esp_wifi"
    "nvs_flash"
    "esp_http_client"
    "esp_http_server"
    "esp_netif"
    "mdns"
    "bt")

set(COMPONENT_INCLUDE_DIRS
    "include"
    "include/features"
    "include/panels"
    "include/panels/inkplate"
    "include/panels/waveshare"
    ${GRAPHICS_INCLUDES}
    ${DECODER_INCLUDES}
    ${SENSOR_INCLUDES})

if(CONFIG_INKPLATE_BOARD_INKPLATE2)
    list(APPEND COMPONENT_SRCS
        "src/features/SPI.cpp"
        "src/features/I2C.cpp")
    list(APPEND COMPONENT_REQUIRES
        "esp_driver_i2c")
elseif(CONFIG_WAVESHARE_BOARD_WAVESHARE13)
    list(APPEND COMPONENT_SRCS
        "src/panels/PanelCommon.cpp")
    list(APPEND COMPONENT_REQUIRES
        "esp_adc")
else()
    list(APPEND COMPONENT_SRCS
        "src/features/I2C.cpp"
        "src/features/RTC.cpp"
        "src/features/Frontlight.cpp"
        "src/features/PCAL.cpp"
        "src/features/TPS.cpp"
        "src/features/SPI.cpp"
        "src/features/SDCard.cpp"
        "src/panels/PanelCommon.cpp"
        "src/features/TouchCypress.cpp"
        "src/features/TouchElan.cpp")
    list(APPEND COMPONENT_REQUIRES
        "esp_driver_i2c"
        "esp_driver_i2s"
        "esp_adc"
        "fatfs"
        "sdmmc")
endif()

idf_component_register(
    SRCS ${COMPONENT_SRCS}
    REQUIRES ${COMPONENT_REQUIRES}
    INCLUDE_DIRS ${COMPONENT_INCLUDE_DIRS})
