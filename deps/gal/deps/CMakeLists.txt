#glad
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)
target_compile_options(glad PRIVATE -Wno-pedantic)

#glm
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # allow overriding of options from variables
set(GLM_ENABLE_CXX_20 ON)
add_subdirectory(glm/)

#slogga
add_subdirectory(slogga/)

#stb
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb.c "#include <stb_image.h> \n #include <stb_image_write.h>") # generate a minimal stb implementation file
add_library(stb STATIC ${CMAKE_CURRENT_BINARY_DIR}/stb.c)
target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION STB_IMAGE_WRITE_IMPLEMENTATION)
target_include_directories(stb PUBLIC stb/)
target_compile_options(stb PRIVATE "-Wno-all") # stb fails to compile with -Wall -Werror in release mode
