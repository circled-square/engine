add_library(engine__application_window STATIC window.cpp)
target_link_libraries(engine__application_window PUBLIC engine__global glad glm)
target_link_libraries(engine__application_window PRIVATE glfw)
