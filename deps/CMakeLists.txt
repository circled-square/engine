add_subdirectory(ankerl_unordered_dense/ EXCLUDE_FROM_ALL) #ankerl::unordered_dense::{map,set}
install(DIRECTORY "ankerl_unordered_dense/include/" TYPE INCLUDE)

add_subdirectory(dylib/ EXCLUDE_FROM_ALL)
add_subdirectory(glfw/ EXCLUDE_FROM_ALL)

set(GAL_AS_SHARED_LIB ON)
set(SLOGGA_AS_SHARED_LIB ON)
add_subdirectory(gal/)
install(DIRECTORY "gal/include/" TYPE INCLUDE)
install(DIRECTORY "gal/deps/glm/glm" TYPE INCLUDE)

# add_subdirectory(slogga/)
install(DIRECTORY "gal/deps/slogga/include/" TYPE INCLUDE)

# imgui
add_library(imgui STATIC 
    #imgui files
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
    #imgui backend files
    imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
)
target_compile_options(imgui PRIVATE "-Wno-all") # no useless warnings inside of imgui
target_include_directories(imgui PUBLIC imgui/)
target_link_libraries(imgui PRIVATE glfw) #for imgui_impl_glfw
install(FILES imgui/imgui.h imgui/imconfig.h DESTINATION include/)

# stb
if(NOT TARGET stb) #everything is inside this guard to avoid redefining target "stb". this is done because GAL currently depends on STB, but that may change in the future, and GAL does not export STB's functionality
    # generate a minimal stb implementation file
    if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/stb.c")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb.c "#include <stb_image.h> \n #include <stb_image_write.h>")
    endif()

    add_library(stb STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb.c)
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC stb/)
    target_compile_options(stb PRIVATE "-Wno-all") # stb fails to compile with -Wall -Werror in release mode
endif()


# tiny_gltf
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tinygltf.cpp")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tinygltf.cpp "#include <tiny_gltf.h>")
endif()

add_library(tinygltf STATIC ${CMAKE_CURRENT_BINARY_DIR}/tinygltf.cpp)
target_compile_definitions(tinygltf PRIVATE TINYGLTF_IMPLEMENTATION)
target_include_directories(tinygltf PUBLIC tinygltf/)
set_target_properties(tinygltf PROPERTIES CXX_STANDARD 20) # tinygltf uses literal operators (operator ""_...) but puts a space between the quotes and the operator name, which is deprecated in c++23 and causes warnings
target_link_directories(tinygltf PUBLIC stb)

set_target_properties(imgui dylib PROPERTIES CXX_CLANG_TIDY "") # no clangtidy errors on code we do not control
