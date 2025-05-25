cmake_minimum_required(VERSION 3.16)
project(ChibiViewer VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Define paths to spine-c components
set(SPINE_C_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spine-c)
set(SPINE_C_INCLUDE_DIR ${SPINE_C_ROOT_DIR}/include)
set(SPINE_C_SRC_DIR_CORE ${SPINE_C_ROOT_DIR}/src/spine)
set(SPINE_C_SRC_DIR_EXTENSION ${SPINE_C_ROOT_DIR}/src)

include_directories(${SPINE_C_INCLUDE_DIR})

# List all .c files from spine-c (core logic and Json.c)
file(GLOB SPINE_CORE_AND_JSON_C_SOURCES "${SPINE_C_SRC_DIR_CORE}/*.c")

# Your Qt-specific extension file, now as .cpp
set(SPINE_EXTENSION_CPP_SOURCE "${SPINE_C_SRC_DIR_EXTENSION}/extension.cpp")

# Combine all spine-c source files
set(ALL_SPINE_SOURCES
    ${SPINE_CORE_AND_JSON_C_SOURCES}
    ${SPINE_EXTENSION_CPP_SOURCE}
)

set(CHIBI_VIEWER_SOURCES ChibiViewer.cpp)

# Add the executable
add_executable(ChibiViewer WIN32
    ${CHIBI_VIEWER_SOURCES}
    ${ALL_SPINE_SOURCES}
)

target_link_libraries(ChibiViewer PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(ChibiViewer PROPERTIES OUTPUT_NAME "ChibiViewer")
