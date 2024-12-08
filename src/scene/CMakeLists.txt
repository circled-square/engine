add_subdirectory(node/)


#node
add_library(engine__scene_node STATIC node.cpp node/collisions.cpp node/script.cpp)
target_link_libraries(engine__scene_node PUBLIC engine__include_dirs glm GAL engine__scene_node_special_node_data)
target_link_libraries(engine__scene_node PRIVATE slogga engine__resources_manager engine__renderer engine__scene)

#scene
add_library(engine__scene STATIC scene.cpp)
target_link_libraries(engine__scene PUBLIC engine__include_dirs GAL glm slogga engine__scene_node)
target_link_libraries(engine__scene PRIVATE imgui engine__resources_manager)
