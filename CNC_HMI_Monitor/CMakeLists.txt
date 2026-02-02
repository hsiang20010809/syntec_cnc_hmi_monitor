cmake_minimum_required(VERSION 3.16)
project(CNC_HMI_Monitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 尋找 Qt 套件
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Charts
)

# HMI Client 執行檔
set(CLIENT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/tcpclient.cpp
    src/tcpclient.h
    src/machinedata.cpp
    src/machinedata.h
)

add_executable(CNC_HMI_Client ${CLIENT_SOURCES})

target_link_libraries(CNC_HMI_Client PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
)

# Mock Server 執行檔
set(SERVER_SOURCES
    server/main.cpp
    server/mockserver.cpp
    server/mockserver.h
)

add_executable(CNC_Mock_Server ${SERVER_SOURCES})

target_link_libraries(CNC_Mock_Server PRIVATE
    Qt6::Core
    Qt6::Network
)

# 設定輸出目錄
set_target_properties(CNC_HMI_Client CNC_Mock_Server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
