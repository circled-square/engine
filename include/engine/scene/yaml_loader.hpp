#ifndef YAML_LOADER_HPP
#define YAML_LOADER_HPP

#include <engine/utils/api_macro.hpp>
#include <memory>

namespace engine {
    class node;

    ENGINE_API std::unique_ptr<node> yaml_example();
}
#endif // YAML_LOADER_HPP
