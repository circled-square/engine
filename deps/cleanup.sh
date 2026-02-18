#!/bin/bash

rm -fr -- tinygltf/models tinygltf/examples tinygltf/experimental tinygltf/tests tinygltf/tools tinygltf/wasm tinygltf/.github

rm -fr -- ankerl_unordered_dense/data ankerl_unordered_dense/doc ankerl_unordered_dense/example ankerl_unordered_dense/scripts ankerl_unordered_dense/test ankerl_unordered_dense/.github

rm -fr glfw/docs/* glfw/examples glfw/tests glfw/.github
touch glfw/docs/CMakeLists.txt

rm -fr imgui/docs imgui/examples imgui/misc

# delete all source files except the couple we use
find murmurhash/src -type f -not -name 'MurmurHash3.cpp' -not -name 'MurmurHash3.h' -delete

rm -fr stb/data stb/deprecated stb/docs stb/stb_image_resize_test stb/tests/ stb/tools stb/.github
rm -f stb/stb_c_lexer.h stb/stb_connected_components.h stb/stb_divide.h stb/stb_ds.h stb/stb_dxt.h stb/stb_easy_font.h stb/stb_herringbone_wang_tile.h stb/stb_hexwave.h stb/stb_image_resize2.h stb/stb_include.h stb/stb_leakcheck.h stb/stb_perlin.h stb/stb_rect_pack.h stb/stb_sprintf.h stb/stb_textedit.h stb/stb_tilemap_editor.h stb/stb_truetype.h stb/stb_vorbis.c stb/stb_voxel_render.h

rm -fr dylib/example dylib/tests
