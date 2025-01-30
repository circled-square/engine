add_subdirectory(node/)


#node
add_library(engine__scene_node STATIC node.cpp)
target_link_libraries(engine__scene_node PUBLIC engine__include_dirs glm GAL engine__scene_node_special_node_data)
target_link_libraries(engine__scene_node PRIVATE slogga engine__resources_manager engine__renderer)

#bp_collision
add_library(engine__scene_bp_collision STATIC broad_phase_collision.cpp)
target_link_libraries(engine__scene_bp_collision PUBLIC engine__include_dirs glm GAL)
target_link_libraries(engine__scene_bp_collision PRIVATE slogga engine__resources_manager engine__scene_node) # by linking with node we also link with the narrow-phase

#scene
add_library(engine__scene STATIC scene.cpp)
target_link_libraries(engine__scene PUBLIC engine__include_dirs GAL glm slogga engine__scene_node engine__scene_bp_collision)
target_link_libraries(engine__scene PRIVATE slogga engine__resources_manager imgui)
