cmake_minimum_required(VERSION 3.16)
project(otherSide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EXTERN_DIR "${CMAKE_SOURCE_DIR}/extern")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# ASIO
set(ASIO_INCLUDE_DIR "${EXTERN_DIR}/asio/include")

# libdatachannel (rtc)
set(RTC_INCLUDE_DIR "${EXTERN_DIR}/libdatachannel/include")
set(RTC_LIBRARY_DIR "${EXTERN_DIR}/libdatachannel/build")

# qlexNet
set(QLEXNET_INCLUDE_DIR "${EXTERN_DIR}/qlexNet/include")

include_directories(
    ${ASIO_INCLUDE_DIR}
    ${QLEXNET_INCLUDE_DIR}
    ${RTC_INCLUDE_DIR}
    ${SRC_DIR}
)
link_directories(${RTC_LIBRARY_DIR})


# Regrouper les sources
set(SOURCES
    src/main.cpp
)

add_executable(otherSide ${SOURCES})

# Link libdatachannel
target_link_libraries(otherSide
    PRIVATE
        datachannel
)
