add_library(GAL__shader STATIC shader.cpp)
target_link_libraries(GAL__shader PUBLIC GAL__types)
target_include_directories(GAL__shader PUBLIC ../include/ ../glad/include)

add_library(GAL__buffer STATIC buffer/buffer.cpp buffer/vertex_buffer.cpp buffer/index_buffer.cpp) # vertex/index_buffer are thin wrappers of buffer
target_link_libraries(GAL__buffer PUBLIC GAL__types glm)
target_include_directories(GAL__buffer PUBLIC ../include/ ../glad/include)

add_library(GAL__vertex_array STATIC vertex_array.cpp)
target_link_libraries(GAL__vertex_array PUBLIC GAL__types GAL__buffer)
target_link_libraries(GAL__vertex_array PRIVATE slogga)
target_include_directories(GAL__vertex_array PUBLIC ../include/ ../glad/include)

add_library(GAL__renderer STATIC renderer/renderer.cpp)
target_link_libraries(GAL__renderer PRIVATE GAL__vertex_array GAL__shader)
target_link_libraries(GAL__renderer PUBLIC GAL__types)
target_include_directories(GAL__renderer PUBLIC ../include/ ../glad/include)

add_library(GAL__texture STATIC texture.cpp)
target_link_libraries(GAL__texture PUBLIC GAL__types glm)
target_link_libraries(GAL__texture PRIVATE slogga)
target_include_directories(GAL__texture PUBLIC ../include/ ../glad/include)

add_library(GAL__image STATIC image.cpp)
target_link_libraries(GAL__image PRIVATE stb)
target_include_directories(GAL__image PUBLIC ../include/ ../glad/include)

add_library(GAL__framebuffer STATIC framebuffer.cpp)
target_link_libraries(GAL__framebuffer PUBLIC GAL__types GAL__texture)
target_link_libraries(GAL__framebuffer PRIVATE slogga)
target_include_directories(GAL__framebuffer PUBLIC ../include/ ../glad/include)

add_library(GAL__types INTERFACE)
target_link_libraries(GAL__types INTERFACE glm glad)
target_include_directories(GAL__types INTERFACE ../include/ ../glad/include)


add_library(GAL__init STATIC init.cpp)
target_link_libraries(GAL__init PUBLIC glad)
target_link_libraries(GAL__init PRIVATE slogga glfw)
target_include_directories(GAL__init PUBLIC ../include/ ../glad/include)

add_library(GAL INTERFACE)
target_link_libraries(GAL INTERFACE GAL__shader GAL__buffer GAL__vertex_array GAL__renderer GAL__texture GAL__image GAL__framebuffer GAL__types GAL__init)
target_include_directories(GAL INTERFACE ../include/)
