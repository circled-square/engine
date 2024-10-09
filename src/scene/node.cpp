#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    static std::string fix_name(std::string s) {
        for(char& c : s) {
            if(!std::isalnum(c) && node::special_chars_allowed_in_node_name.find(c) == std::string_view::npos)
                c = '_';
        }
        return s;
    }

    node::node(std::string name, special_node_data_variant_t other_data, glm::mat4 transform, std::optional<script> script)
        : m_children(),
          m_children_is_sorted(true),
          m_father(nullptr),
          m_name(fix_name(std::move(name))),
          m_transform(std::move(transform)),
          m_other_data(std::move(other_data)),
          m_nodetree_reference(rc<const nodetree>()),
          m_script(std::move(script)) {
        if(m_script)
            m_script->attach(*this);
    }

    node::node(node&& o)
        : m_children(std::move(o.m_children)),
          m_children_is_sorted(o.m_children_is_sorted),
          m_father(o.m_father),
          m_name(std::move(o.m_name)),
          m_transform(std::move(o.m_transform)),
          m_other_data(std::move(o.m_other_data)),
          m_nodetree_reference(std::move(o.m_nodetree_reference)),
          m_script(std::move(o.m_script))
    {
        o.m_father = nullptr;
    }

    node::node(const node& o)
        : m_children(o.m_children),
          m_children_is_sorted(o.m_children_is_sorted),
          m_father(nullptr),
          m_name(o.m_name),
          m_transform(o.m_transform),
          m_other_data(o.m_other_data),
          m_nodetree_reference(o.m_nodetree_reference),
          m_script(o.m_script)
    {}

    node& node::operator=(node&& o) {
        m_children = std::move(o.m_children);
        m_children_is_sorted = o.m_children_is_sorted;

        m_father = o.m_father;
        o.m_father = nullptr;

        m_name = std::move(o.m_name);
        m_transform = std::move(o.m_transform);
        m_other_data = std::move(o.m_other_data);
        m_nodetree_reference = std::move(o.m_nodetree_reference);
        m_script = std::move(o.m_script);

        return *this;
    }

    node::node(const rc<const nodetree>& nt, std::string name) : node(nt->root()) {
        m_nodetree_reference = std::move(nt);
        if(!name.empty()) m_name = fix_name(name);
    }


    inline void insert_sorted(std::vector<detail::internal_node>& vec, node item) {
        auto upper_bound = std::upper_bound(vec.begin(), vec.end(), item,
            [](const node& n, const detail::internal_node& in) { return n.name() < in->name(); });
        vec.insert(upper_bound, detail::internal_node{std::move(item)});
    }


    void node::add_child(node c) {
        c.m_father = this;

        if(m_children_is_sorted) {
            insert_sorted(m_children, std::move(c));
        } else {
            m_children.push_back(std::move(c));
        }
    }

    node& node::get_child(std::string_view name) {
        if(m_children_is_sorted) {
            auto it = std::lower_bound(m_children.begin(), m_children.end(), name,
                [](const detail::internal_node& n, const std::string_view& s) { return n->name() < s; });

            if (it != m_children.end() && (*it)->name() == name) {
                return **it;
            }
        } else {
            auto it = std::find_if(m_children.begin(), m_children.end(), [&name](const detail::internal_node& n){ return n->name() == name; });
            if (it != m_children.end()) {
                return **it;
            }
        }
        throw std::runtime_error(std::format("node at path '{}' has no child named '{}'", absolute_path(), name));
    }

    node* node::try_get_father() { return m_father; }

    node& node::get_father() {
        if(m_father)
            return *m_father;
        else
            throw std::runtime_error(std::format("node with name '{}' has no father, but called node::father()", name()));
    }
    const_node_span node::children() const { return m_children; }
    node_span node::children() { return m_children; }

    void node::set_children_sorting_preference(bool v) {
        if(v && !m_children_is_sorted) {
            std::sort(m_children.begin(), m_children.end(), [](const detail::internal_node& a, const detail::internal_node& b) { return a->name() < b->name(); });
            m_children_is_sorted = true;
        } else if(!v && m_children_is_sorted) {
            m_children_is_sorted = false;
        }
    }

    std::string node::absolute_path() const {
        return m_father ? m_father->absolute_path() + "/" + m_name : m_name;
    }
    const std::string& node::name() const { return m_name; }

    const glm::mat4 &node::transform() const { return m_transform; }

    glm::mat4 &node::transform() { return m_transform; }

    node& node::get_from_path(std::string_view path) {
        std::string_view subpath = path;
        node* current_node = this;
        while(true) {
            size_t separator_position = subpath.find('/');
            if (separator_position == std::string_view::npos) {
                //could not find separator; base case
                return current_node->get_child(std::string(subpath));
            } else {
                std::string_view next_step = subpath.substr(0, separator_position);
                current_node = &current_node->get_child(std::string(next_step));
                subpath.remove_prefix(separator_position + 1);
            }
        }
    }

    template<SpecialNodeData T> bool     node::has() const { return std::holds_alternative<T>(m_other_data); }
    template<SpecialNodeData T> const T& node::get() const { EXPECTS(has<T>()); return std::get<T>(m_other_data); }
    template<SpecialNodeData T> T&       node::get()       { EXPECTS(has<T>()); return std::get<T>(m_other_data); }

    template<SpecialNodeData T> class node_templates_instantiator {
        T& (node::*_get)() = &node::get<T>;
        const T& (node::*_get_const)() const = &node::get<T>;
        bool(node::*_has)() const = &node::has<T>;
    };

    constexpr static map_pack<node_templates_instantiator, std::variant, std::tuple, special_node_data_variant_t> __instantiate_node_templates;



    viewport::viewport(framebuffer fbo, rc<const shader> postfx_shader, std::optional<glm::vec2> dynamic_size_relative_to_output)
        : m_fbo(std::move(fbo)), m_postfx_material(std::move(postfx_shader), {m_fbo.get_texture()}),
          m_dynamic_size_relative_to_output(dynamic_size_relative_to_output) {}

    viewport::viewport(rc<const shader> postfx_shader, glm::vec2 dynamic_size_relative_to_output)
        : viewport(framebuffer(rc<gal::texture>()), std::move(postfx_shader), dynamic_size_relative_to_output) {}

    viewport::viewport(viewport &&o)
        : m_fbo(std::move(o.m_fbo)), m_postfx_material(std::move(o.m_postfx_material)),
        m_dynamic_size_relative_to_output(o.m_dynamic_size_relative_to_output) {}

    inline viewport copy(const viewport &o) {
        rc<gal::texture> new_texture = get_rm().new_mut_from<gal::texture>(gal::texture::empty(o.fbo().resolution(), 4));
        return viewport(framebuffer(std::move(new_texture)), o.postfx_material().get_shader(), o.dynamic_size_relative_to_output());
    }

    viewport::viewport(const viewport &o)
        : viewport(copy(o)) {}

    framebuffer &viewport::fbo() { return m_fbo; }

    const framebuffer &viewport::fbo() const { return m_fbo; }

    material &viewport::postfx_material() { return m_postfx_material; }

    const material &viewport::postfx_material() const { return m_postfx_material; }

    std::optional<glm::vec2> viewport::dynamic_size_relative_to_output() const { return m_dynamic_size_relative_to_output; }

    void viewport::bind_draw() const { m_fbo.bind_draw(); }

    void viewport::set_active_camera(const camera *c) { m_active_camera = c; }

    const camera *viewport::get_active_camera() const { return m_active_camera; }

    void viewport::output_resolution_changed(glm::ivec2 output_resolution) const {
        if(!m_dynamic_size_relative_to_output)
            return;

        glm::ivec2 new_resolution = (glm::vec2)output_resolution * (*m_dynamic_size_relative_to_output);
        if(new_resolution != m_fbo.resolution()) {
            // a texture with 0 texels causes the fbo to throw a framebuffer_construction_exception
            if (new_resolution.x > 0 && new_resolution.y > 0) {
                rc<gal::texture> new_texture = get_rm().new_mut_from<gal::texture>(gal::texture::empty(new_resolution, 4));
                m_postfx_material.get_texture(0) = new_texture;
                m_fbo.link_texture(std::move(new_texture));
            }
        }
    }

    void viewport::operator=(viewport &&o) {
        m_fbo = std::move(o.m_fbo);
        m_postfx_material = std::move(o.m_postfx_material);
        m_dynamic_size_relative_to_output = std::move(o.m_dynamic_size_relative_to_output);
        m_active_camera = std::move(o.m_active_camera);
    }
}

