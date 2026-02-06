#include <engine/utils/read_file.hpp>

#include <fstream>

namespace engine {
    std::string read_file(const std::string& fname) {
        try {
            std::ifstream fi(fname);
            fi.exceptions(std::ifstream::failbit); // causes exceptions to be thrown in case of errors
            std::string contents((std::istreambuf_iterator<char>(fi)), std::istreambuf_iterator<char>());
            return contents;
        } catch (std::exception& e) {
            throw std::runtime_error(std::format("could not read file {}", fname));
        }
    };
}
