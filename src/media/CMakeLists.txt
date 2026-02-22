add_library(media STATIC
    H264/H264Encoder.cpp
    H264/H264Decoder.cpp
    FakeVideoSource.cpp
)

target_link_libraries(media
    PUBLIC
        PkgConfig::AVCODEC
        PkgConfig::AVUTIL
        PkgConfig::SWSCALE
)
