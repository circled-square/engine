add_subdirectory(material/)

#mesh
add_library(engine__scene_renderer_mesh_material STATIC material.cpp)
target_link_libraries(engine__scene_renderer_mesh_material PUBLIC engine__global GAL
    engine__scene_renderer_mesh_material_shader engine__scene_renderer_mesh_material_materials)
target_link_libraries(engine__scene_renderer_mesh_material PRIVATE engine__resources_manager)
