add_library(engine__global INTERFACE)
target_include_directories(engine__global INTERFACE ../include/)
target_link_libraries(engine__global INTERFACE slogga engine__utils)

add_subdirectory(application/)
add_subdirectory(scene/)
add_subdirectory(resources_manager/)
add_subdirectory(utils/)

#application
add_library(engine__application STATIC application.cpp)
target_link_libraries(engine__application PUBLIC engine__global engine__scene engine__application_window engine__resources_manager)
target_link_libraries(engine__application PRIVATE imgui GAL)

#resources_manager
add_library(engine__resources_manager STATIC resources_manager.cpp)
target_link_libraries(engine__resources_manager PUBLIC engine__global engine__resources_manager_rc engine__resources_manager_weak)
target_link_libraries(engine__resources_manager PRIVATE engine__resources_manager_template_defs engine__resources_manager_rc_resource dylib GAL)

#scene
add_library(engine__scene STATIC scene.cpp)
target_link_libraries(engine__scene PUBLIC engine__global engine__scene_node engine__scene_bp_collision engine__scene_application_channel engine__scene_yaml_loader)
target_link_libraries(engine__scene PRIVATE engine__resources_manager imgui)

#engine
add_library(engine SHARED entry_point.cpp)
target_link_libraries(engine PUBLIC glm engine__scene)
target_link_libraries(engine PRIVATE GAL engine__global engine__utils engine__application engine__resources_manager engine__scene imgui slogga)
target_include_directories(engine PUBLIC ../include/ ../deps/ankerl_unordered_dense/include/ ../deps/gal/deps/slogga/include/ ../deps/gal/deps/glad/include/ ../deps/gal/include/ ../deps/imgui/)

install(TARGETS engine
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION .
    RUNTIME DESTINATION . #necessary because windows dlls are RUNTIME, not LIBRARY
)


install(DIRECTORY ../include/ TYPE INCLUDE)
