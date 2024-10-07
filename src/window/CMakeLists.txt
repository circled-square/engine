add_library(engine__window STATIC window.cpp)
target_link_libraries(engine__window PUBLIC glad glm)
target_link_libraries(engine__window PRIVATE glfw slogga)
target_include_directories(engine__window PUBLIC ../../include/)
