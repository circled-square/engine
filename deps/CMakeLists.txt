add_subdirectory(ankerl_unordered_dense/) #ankerl::unordered_dense::{map,set}
add_subdirectory(gal/)
add_subdirectory(glfw/)
add_subdirectory(slogga/)
add_subdirectory(tinygltf/)

#imgui
add_library(imgui STATIC 
    #imgui files
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
    #imgui backend files
    imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC imgui/)
target_link_libraries(imgui PRIVATE glfw) #for imgui_impl_glfw

#stb
if(NOT TARGET stb) #everything is inside this guard to avoid redefining target "stb". this is done because GAL currently depends on STB, but that may change in the future, and GAL does not export STB's functionality
    # generate a minimal stb implementation file
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb.c "#include <stb_image.h> \n #include <stb_image_write.h>") 

    add_library(stb STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb.c)
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC stb/)
    target_compile_options(stb PRIVATE "-Wno-all") # stb fails to compile with -Wall -Werror in release mode
endif()

