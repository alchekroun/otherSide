add_library(netMessage STATIC
    DCMessageManager.cpp
    NetMessage.cpp
)

target_include_directories(app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
)

# target_link_libraries(app
#     PUBLIC
#         imgui
#         rlImGui
# )