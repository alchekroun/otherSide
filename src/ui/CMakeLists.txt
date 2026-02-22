add_library(ui STATIC
    panel/MessagePanel.cpp
    panel/VideoPanel.cpp
)

target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ui PUBLIC raylib imgui)