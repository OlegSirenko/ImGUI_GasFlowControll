cmake_minimum_required(VERSION 3.27)
project(GasFlowControlMonitoringApp)

set(CMAKE_CXX_STANDARD 17)
set (BOOST_ROOT "C:/boost_1_82_0/") # Путь к библиотеке Boost


add_subdirectory(imgui)
find_package(Boost REQUIRED)
include_directories(modules)

if (Boost_FOUND)
    include_directories (SYSTEM ${Boost_INCLUDE_DIR}) # подключаем заголовочные файлы
endif ()

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
        modules/ServerModule.cpp
)

add_executable(CompileFonts imgui/misc/fonts/binary_to_compressed_c.cpp)


set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(GasFlowControlMonitoringApp PUBLIC ws2_32 wsock32 imgui imgui-win32 imgui-dx11 d3dcompiler dwmapi ${Boost_LIBRARIES})

#target_link_options(GasFlowControlMonitoringApp PRIVATE -mwindows)

