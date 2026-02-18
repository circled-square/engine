add_library(GAL__shader STATIC shader.cpp)
target_link_libraries(GAL__shader PUBLIC GAL__types)
target_link_libraries(GAL__shader PRIVATE glad)
target_include_directories(GAL__shader PUBLIC ../include/)

add_library(GAL__buffer STATIC buffer/buffer.cpp buffer/vertex_buffer.cpp buffer/index_buffer.cpp) # vertex/index_buffer are thin wrappers of buffer
target_link_libraries(GAL__buffer PUBLIC GAL__types glm)
target_link_libraries(GAL__buffer PRIVATE glad)
target_include_directories(GAL__buffer PUBLIC ../include/)

add_library(GAL__vertex_array STATIC vertex_array.cpp)
target_link_libraries(GAL__vertex_array PUBLIC GAL__types GAL__buffer)
target_link_libraries(GAL__vertex_array PRIVATE slogga glad)
target_include_directories(GAL__vertex_array PUBLIC ../include/)

add_library(GAL__renderer STATIC renderer/renderer.cpp)
target_link_libraries(GAL__renderer PRIVATE GAL__vertex_array GAL__shader)
target_link_libraries(GAL__renderer PUBLIC GAL__types glad)
target_include_directories(GAL__renderer PUBLIC ../include/)

add_library(GAL__texture STATIC texture.cpp)
target_link_libraries(GAL__texture PUBLIC GAL__types glm)
target_link_libraries(GAL__texture PRIVATE slogga glad)
target_include_directories(GAL__texture PUBLIC ../include/)

add_library(GAL__image STATIC image.cpp)
target_link_libraries(GAL__image PRIVATE stb GAL__types)
target_include_directories(GAL__image PUBLIC ../include/)

add_library(GAL__framebuffer STATIC framebuffer.cpp)
target_link_libraries(GAL__framebuffer PUBLIC GAL__types GAL__texture)
target_link_libraries(GAL__framebuffer PRIVATE slogga glad)
target_include_directories(GAL__framebuffer PUBLIC ../include/)

add_library(GAL__types STATIC types.cpp)
target_link_libraries(GAL__types PUBLIC glm)
target_link_libraries(GAL__types PRIVATE glad)
target_include_directories(GAL__types PUBLIC ../include/)



add_library(GAL STATIC init.cpp)
target_link_libraries(GAL PUBLIC glm)
target_link_libraries(GAL PRIVATE slogga glad GAL__shader GAL__buffer GAL__vertex_array GAL__renderer GAL__texture GAL__image GAL__framebuffer GAL__types)
target_include_directories(GAL PUBLIC ../include/)

if(${GAL_AS_SHARED_LIB})
    target_compile_definitions(GAL__types PUBLIC GAL_BEING_COMPILED GAL_AS_SHARED_LIB)
endif()
