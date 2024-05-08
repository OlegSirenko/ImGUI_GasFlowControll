cmake_minimum_required(VERSION 3.27)
project(GasFlowControlMonitoringApp)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(imgui)

include_directories(modules)
#include_directories(libs)
#include_directories(libs/include)

#find_library(GAMENETWORKING_LIB NAMES GameNetworkingSockets PATHS ${PROJECT_SOURCE_DIR}/libs/)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon_preporation/favimage.rc")

add_executable(GasFlowControlMonitoringApp main.cpp
        modules/PlotWindow.cpp
        modules/ControlPanel.cpp
        modules/PID.cpp
#        modules/SocketModule.cpp
        ${app_icon_resource_windows}
        resources/bruceFontEmbedded_utf_8.cpp
        resources/ExoFontEmbedded_utf8.cpp
        modules/mainMenu.cpp
)

add_executable(CompileFonts imgui/misc/fonts/binary_to_compressed_c.cpp)

set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(GasFlowControlMonitoringApp PUBLIC imgui imgui-win32 imgui-dx11 d3dcompiler dwmapi)

#target_link_options(GasFlowControlMonitoringApp PRIVATE -mwindows)

