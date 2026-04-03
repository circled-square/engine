#!/bin/bash

rm -fr -- tinygltf/models tinygltf/examples tinygltf/experimental tinygltf/tests tinygltf/tools tinygltf/wasm tinygltf/benchmark

rm -fr -- ankerl_unordered_dense/data ankerl_unordered_dense/doc ankerl_unordered_dense/example ankerl_unordered_dense/scripts ankerl_unordered_dense/test

rm -fr glfw/docs/* glfw/examples glfw/tests
touch glfw/docs/CMakeLists.txt

rm -fr imgui/docs imgui/examples imgui/misc

# delete all source files except the couple we use
find murmurhash/src -type f -not -name 'MurmurHash3.cpp' -not -name 'MurmurHash3.h' -delete

rm -fr dylib/example dylib/tests

find rapidyaml/ -type d \( -name 'changelog' -o -name 'test' -o -name 'samples' -o -name 'img' -o -name 'doc' -o -name 'tools' -o -name 'script' -o -name 'fuzz' -o -name 'ci' -o -name 'src_extra' -o -name 'bm' -o -name 'api' \) -exec rm -rf "{}" +
rm -f rapidyaml/.gitmodules rapidyaml/.gitignore rapidyaml/.lgtm.yml rapidyaml/.readthedocs.yaml rapidyaml/ROADMAP.md
