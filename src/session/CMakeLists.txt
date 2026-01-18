add_library(session STATIC
    ClientSession.cpp
    HostSession.cpp
)

target_include_directories(session PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(session PUBLIC datachannel)