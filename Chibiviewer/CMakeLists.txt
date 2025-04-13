cmake_minimum_required(VERSION 3.16)
project(ChibiViewer VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
add_executable(ChibiViewer WIN32 ChibiViewer.cpp)
target_link_libraries(ChibiViewer PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(ChibiViewer PROPERTIES OUTPUT_NAME "ChibiViewer")

set(SOURCE_FILES
    ChibiViewer.cpp
)
include_directories(${Qt6Core_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS})
