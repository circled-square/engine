#ifndef ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP
#define ENGINE_SCENE_RENDERER_MESH_MATERIAL_HPP

#include <engine/resources_manager/rc.hpp>
#include "material/shader.hpp"
#include <GAL/texture.hpp>

namespace engine {
    class material {
        rc<const shader> m_shader;
        //must be the same size as m_shader.uniforms.sampler_names
        std::vector<rc<const gal::texture>> m_textures;
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
