add_library(engine__utils_read_file STATIC read_file.cpp)
target_link_libraries(engine__utils_read_file PUBLIC engine__global)

add_library(engine__utils_hash STATIC hash.cpp)
target_link_libraries(engine__utils_hash PUBLIC engine__global unordered_dense glm) # also linked to glm because it provides hash function for glm vecs

add_library(engine__utils INTERFACE)
target_link_libraries(engine__utils INTERFACE engine__utils_read_file engine__utils_hash)
