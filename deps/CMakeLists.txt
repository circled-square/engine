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
    #CONFIGURE_COMMAND "" BUILD_COMMAND ""
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
endif()

set(IMGUI_SOURCES imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp)
list(TRANSFORM IMGUI_SOURCES PREPEND "${imgui_SOURCE_DIR}/")
add_library(imgui STATIC ${IMGUI_SOURCES})
unset(IMGUI_SOURCES)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/)
target_link_libraries(imgui PRIVATE glfw) #for imgui_impl_glfw


#fetch lua
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG "v5.4.7"
#    CONFIGURE_COMMAND "" BUILD_COMMAND ""
)
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
    FetchContent_MakeAvailable(lua)
endif()

set(LUA_SOURCES lapi.c lauxlib.c lbaselib.c lcode.c lcorolib.c lctype.c ldblib.c ldebug.c ldo.c
    ldump.c lfunc.c lgc.c linit.c liolib.c llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c
    loslib.c lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c ltests.c ltm.c lundump.c
    lutf8lib.c lvm.c lzio.c)

list(TRANSFORM LUA_SOURCES PREPEND ${lua_SOURCE_DIR}/)
add_library(lua STATIC ${LUA_SOURCES})
unset(LUA_SOURCES)
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/)

#fetch stb
#everything is inside this guard to avoid defining target "stb" twice (and to ensure it is only compiled once). I refuse to simply remove it just because GAL currently depends on STB, since that may change and it makes no sense for GAL to export STB's functionality
if(NOT TARGET stb) # stb has not already been included by someone else
    #stb does not support cmake, but it is a header only library which makes it semi-trivial to fetch with FetchContent
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        # CONFIGURE_COMMAND "" BUILD_COMMAND ""
    )
    FetchContent_GetProperties(stb)
    if(NOT stb_POPULATED)
        FetchContent_MakeAvailable(stb)
    endif()

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb.c "#include <stb_image.h> \n #include <stb_image_write.h>") # generate a minimal stb implementation file

    add_library(stb STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb.c)
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC ${stb_SOURCE_DIR}/)
    target_compile_options(stb PRIVATE "-Wno-all") # stb fails to compile with -Wall -Werror in release mode
endif()


#fetch tinygltf
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf GIT_TAG release
    # CONFIGURE_COMMAND "" BUILD_COMMAND ""
)
FetchContent_MakeAvailable(tinygltf)
