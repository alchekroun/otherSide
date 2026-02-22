add_library(media STATIC
    VideoDecoder.cpp
    FFmpegH264Encoder.cpp
)

target_link_libraries(media
    PUBLIC
        PkgConfig::AVCODEC
        PkgConfig::AVUTIL
        PkgConfig::SWSCALE
)
