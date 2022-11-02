cmake_minimum_required(VERSION 3.14)

project(serverSaveData LANGUAGES CXX)



set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)



set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)



add_executable(serverSaveData ${SOURCES})


target_include_directories(serverSaveData PRIVATE ${INCLUDE_DIR})
target_include_directories(serverSaveData PRIVATE ${SOURCE_DIR})


target_link_libraries(serverSaveData Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)
