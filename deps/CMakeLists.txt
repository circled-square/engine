include(FetchContent)

#fetch gal
FetchContent_Declare(
    gal
    GIT_REPOSITORY https://github.com/circled-square/GAL.git
)
FetchContent_MakeAvailable(gal)

#fetch glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

#fetch slogga
FetchContent_Declare(
    slogga
    GIT_REPOSITORY https://github.com/circled-square/SLOGGA.git
)
FetchContent_MakeAvailable(slogga)

#fetch imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    CONFIGURE_COMMAND "" BUILD_COMMAND ""
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
endif()

set(IMGUI_FILES "${imgui_SOURCE_DIR}/imgui.cpp" "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp" "${imgui_SOURCE_DIR}/imgui_widgets.cpp")
set(IMGUI_BACKENDS_FILES "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp" "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")
add_library(imgui STATIC ${IMGUI_FILES} ${IMGUI_BACKENDS_FILES})
unset(IMGUI_FILES)
unset(IMGUI_BACKENDS_FILES)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/)
target_link_libraries(imgui PRIVATE glfw) #for imgui_impl_glfw


#fetch stb
#everything is inside this guard to avoid defining target "stb" twice (and to ensure it is only compiled once). I refuse to simply remove it just because GAL currently depends on STB, since that may change and it makes no sense for GAL to export STB's functionality
if(NOT TARGET stb) # stb has not already been included by someone else
    #stb does not support cmake, but it is a header only library which makes it semi-trivial to fetch with FetchContent
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        CONFIGURE_COMMAND "" BUILD_COMMAND ""
    )
    FetchContent_GetProperties(stb)
    if(NOT stb_POPULATED)
        FetchContent_MakeAvailable(stb)
    endif()

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb.c "#include <stb_image.h> \n #include <stb_image_write.h>") # generate a minimal stb implementation file

    add_library(stb STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb.c)# ${CMAKE_CURRENT_BINARY_DIR}/stb_image.c ${CMAKE_CURRENT_BINARY_DIR}/stb_image_write.c)
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC ${stb_SOURCE_DIR}/)
    target_compile_options(stb PRIVATE "-Wno-all") # stb fails to compile with -Wall -Werror in release mode
endif()


#fetch tinygltf
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf GIT_TAG release
    CONFIGURE_COMMAND "" BUILD_COMMAND ""
)
FetchContent_MakeAvailable(tinygltf)
