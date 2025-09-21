add_library(duino_bus STATIC
    src/Bus.cpp
    src/BusLog.cpp
    src/CorePacketHandler.cpp
    src/Packer.cpp
    src/Packet.cpp
    src/PacketDecoder.cpp
    src/PacketEncoder.cpp
    src/PicoUsbBus.cpp
    src/Unpacker.cpp
)

target_include_directories(duino_bus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(duino_bus
    duino_log
    duino_led
    duino_util
    pico_stdlib
)
pico_enable_stdio_usb(duino_bus 1)
