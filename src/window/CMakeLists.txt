add_library(engine__window STATIC window.cpp)
target_link_libraries(engine__window PUBLIC engine__include_dirs glad glm)
target_link_libraries(engine__window PRIVATE glfw slogga)
