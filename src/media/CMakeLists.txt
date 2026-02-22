add_library(media STATIC
    src/VideoDecoder.cpp
    src/FFmpegH264Encoder.cpp
)

target_link_libraries(media
    PUBLIC
        PkgConfig::AVCODEC
        PkgConfig::AVUTIL
        PkgConfig::SWSCALE
)

target_include_directories(media
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
