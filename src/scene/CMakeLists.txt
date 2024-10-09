#node
add_library(engine__scene_node STATIC node.cpp)
target_link_libraries(engine__scene_node PUBLIC glm GAL)
target_link_libraries(engine__scene_node PRIVATE slogga)
target_include_directories(engine__scene_node PUBLIC ../../include/)

#scene
add_library(engine__scene STATIC scene.cpp)
target_link_libraries(engine__scene PUBLIC GAL glm slogga engine__scene_node)
target_link_libraries(engine__scene PRIVATE imgui)
target_include_directories(engine__scene PUBLIC ../../include/)
