cmake_minimum_required(VERSION 3.16)
project(otherSide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EXTERN_DIR "${CMAKE_SOURCE_DIR}/extern")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_TIDY_EXE
    NAMES clang-tidy
)

# ASIO
set(ASIO_INCLUDE_DIR "${EXTERN_DIR}/asio/include")

# libdatachannel (rtc)
set(RTC_INCLUDE_DIR "${EXTERN_DIR}/libdatachannel/include")
set(RTC_LIBRARY_DIR "${EXTERN_DIR}/libdatachannel/build")

# qlexNet
set(QLEXNET_INCLUDE_DIR "${EXTERN_DIR}/qlexNet/include")

#RayLib
set(RAYLIB_DIR "${EXTERN_DIR}/raylib")
set(RAYLIB_INCLUDE_DIR "${RAYLIB_DIR}/include")
set(RAYLIB_LIBRARY_DIR "${RAYLIB_DIR}/lib")

add_subdirectory(
    ${RAYLIB_DIR}
)

# ImGui
set(IMGUI_DIR "${EXTERN_DIR}/imgui")
set(IMGUI_BACKENDS_DIR "${EXTERN_DIR}/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

# rlImGui
set(RLIMGUI_DIR "${EXTERN_DIR}/rlImGui")
set(RLIMGUI_SOURCES
    "${RLIMGUI_DIR}/rlImGui.cpp"
)

add_library(rlImGui STATIC ${RLIMGUI_SOURCES})
target_include_directories(rlImGui PUBLIC ${RLIMGUI_DIR})
target_link_libraries(rlImGui PUBLIC raylib imgui)

include_directories(
    ${ASIO_INCLUDE_DIR}
    ${QLEXNET_INCLUDE_DIR}
    ${RTC_INCLUDE_DIR}
    ${IMGUI_BACKENDS_DIR}
    ${SRC_DIR}
)
link_directories(
    ${RTC_LIBRARY_DIR}
)

add_subdirectory(src/app)
add_subdirectory(src/session)
add_subdirectory(src/message)

add_executable(${PROJECT_NAME} src/main.cpp)

if (CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found : ${CLANG_TIDY_EXE}")
    set_target_properties(app PROPERTIES
        CXX_CLANG_TIDY
        ${CLANG_TIDY_EXE};
        -warnings-as-errors=clang-analyzer-*;
        -warnings-as-errors=bugprone-*;
        -warnings-as-errors=performance-*;
        -header-filter=^${SRC_DIR}/
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        app
        session
        netMessage
)
