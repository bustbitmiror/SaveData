cmake_minimum_required(VERSION 3.14)

project(clientSaveData LANGUAGES CXX)



set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Test Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Test Network REQUIRED)



set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

include_directories(${TEST_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    "${TEST_DIR}/*.h"
    "${TEST_DIR}/*.cpp"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)



add_executable(clientSaveData ${SOURCES})

target_include_directories(clientSaveData PRIVATE ${TEST_DIR})
target_include_directories(clientSaveData PRIVATE ${INCLUDE_DIR})
target_include_directories(clientSaveData PRIVATE ${SOURCE_DIR})


target_link_libraries(clientSaveData Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Network)
