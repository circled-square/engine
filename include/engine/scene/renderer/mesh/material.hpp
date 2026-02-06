#ifndef ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP
#define ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP

#include <engine/resources_manager/rc.hpp>
#include <engine/utils/meta.hpp>
#include <engine/utils/hash.hpp>
#include "material/shader.hpp"
#include <GAL/texture.hpp>
#include <engine/utils/lin_algebra.hpp>

namespace engine {
    namespace internal {
        template<typename T>
        using vec_variant_helper = std::variant<T, glm::vec<2,T>, glm::vec<3,T>, glm::vec<4,T>>;
    }

    template<typename... Ts>
    using vec_variant = merge_variants<internal::vec_variant_helper<Ts>...>;

    template<typename T>
    using mat_variant = std::variant<glm::mat<2,2,T>, glm::mat<2,3,T>, glm::mat<2,4,T>, glm::mat<3,2,T>, glm::mat<3,3,T>, glm::mat<3,4,T>, glm::mat<4,2,T>, glm::mat<4,3,T>, glm::mat<4,4,T>>;


    using uniform_value_variant = merge_variants<mat_variant<float>, vec_variant<float, double, gal::sint, gal::uint, bool>>;



    class material {
        rc<const shader> m_shader;
        //must be the same size as m_shader.uniforms.sampler_names
        std::vector<rc<const gal::texture>> m_textures;
        std::vector<std::pair<std::string, uniform_value_variant>> m_custom_uniforms;
    public:
        material() = delete;
        material(material&& o) = default;
        material(const material& o) = default;

        material(rc<const shader> shader, std::vector<rc<const gal::texture>> textures);
        material(rc<const shader> shader, rc<const gal::texture> texture);

        const rc<const engine::shader>& get_shader() const;
        const std::vector<rc<const gal::texture>>& get_textures() const;
        rc<const gal::texture>& get_texture(size_t);
        const rc<const gal::texture>& get_texture(size_t) const;

        void bind_and_set_uniforms(mvp_matrices mvp, glm::ivec2 output_resolution, float frame_time) const;

        std::vector<std::pair<std::string, uniform_value_variant>>& get_custom_uniforms() { return m_custom_uniforms; }
        const std::vector<std::pair<std::string, uniform_value_variant>>& get_custom_uniforms() const { return m_custom_uniforms; }

        material& operator=(material&& o);
    };
}

#endif // ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP
