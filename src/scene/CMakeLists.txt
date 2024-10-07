
#basic_scene
add_library(engine__basic_scene STATIC basic_scene.cpp)
target_link_libraries(engine__basic_scene PUBLIC glm)
target_link_libraries(engine__basic_scene PRIVATE engine__application)
target_include_directories(engine__basic_scene PUBLIC ../../include/)

#node
add_library(engine__scene_node STATIC node.cpp)
target_link_libraries(engine__scene_node PUBLIC glm GAL)
target_link_libraries(engine__scene_node PRIVATE slogga)
target_include_directories(engine__scene_node PUBLIC ../../include/)

#scene
add_library(engine__scene STATIC scene.cpp)
target_link_libraries(engine__scene PUBLIC GAL glm slogga engine__scene_node engine__basic_scene)
target_link_libraries(engine__scene PRIVATE imgui)
target_include_directories(engine__scene PUBLIC ../../include/)
