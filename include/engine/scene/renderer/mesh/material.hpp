#ifndef ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP
#define ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP

#include <engine/resources_manager/rc.hpp>
#include <engine/utils/meta.hpp>
#include <engine/utils/hash.hpp>
#include "material/shader.hpp"
#include <GAL/texture.hpp>

namespace engine {
    template<template<int, typename> class Vec, typename T>
    using vec_variant = std::variant<T, Vec<2,T>, Vec<3,T>, Vec<4,T>>;

    template<template<int, int, typename> class Mat, typename T>
    using mat_variant = std::variant<Mat<2,2,T>, Mat<3,3,T>, Mat<4,4,T>>;

    template<class T>
    using matvec_variant = merge_variants<mat_variant<glm::mat, T>, vec_variant<glm::vec, T>>;

    using uniform_value_variant = merge_variants<matvec_variant<float>, matvec_variant<double>, matvec_variant<int>, matvec_variant<unsigned>, matvec_variant<bool>>;



    class material {
        rc<const shader> m_shader;
        //must be the same size as m_shader.uniforms.sampler_names
        std::vector<rc<const gal::texture>> m_textures;
        std::vector<std::pair<std::string, uniform_value_variant>> m_custom_uniforms;
    public:
        material() = delete;
        material(material&& o);
        material(const material& o);
        material(rc<const shader> shader, std::vector<rc<const gal::texture>> textures);
        material(rc<const shader> shader, rc<const gal::texture> texture);

        const rc<const engine::shader>& get_shader() const;
        const std::vector<rc<const gal::texture>>& get_textures() const;
        rc<const gal::texture>& get_texture(size_t);
        const rc<const gal::texture>& get_texture(size_t) const;

        void bind_and_set_uniforms(glm::mat4 mvp, glm::ivec2 output_resolution, float frame_time) const;

        material& operator=(material&& o);
    };
}

#endif // ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP
