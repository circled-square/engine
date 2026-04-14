# glad
add_library(glad STATIC glad/src/glad.c glad/include/glad/glad.h glad/include/KHR/khrplatform.h)
target_include_directories(glad PUBLIC glad/include)
target_compile_options(glad PRIVATE -Wno-pedantic)

# glm
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # allow overriding of options from variables
set(GLM_ENABLE_CXX_20 ON)
set(GLM_QUIET ON) # glm is quite verbose by default
set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
add_subdirectory(glm/)

# slogga
add_subdirectory(slogga/)

# stb_image
# generate a minimal stb implementation file
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/stb_image.c")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb_image.c "#include<stb_image.h>\n#include<stb_image_write.h>")
endif()
add_library(stb_image STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb_image.c)
target_compile_definitions(stb_image PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION)
target_include_directories(stb_image PUBLIC stb/)
# target_compile_options(stb_image PRIVATE "-Wno-all") # stb fails to compile with -Wall -Werror in release mode
set_property(TARGET stb_image PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(glm glm-header-only glad stb_image PROPERTIES CXX_CLANG_TIDY "") # no clangtidy errors on code we do not control