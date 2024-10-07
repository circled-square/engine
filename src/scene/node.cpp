#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    //for binary search & ordering of children nodes by name
    static std::strong_ordering operator<=>(const std::string_view& s, const node& n) { return s <=> n.name(); }
    static std::strong_ordering operator<=>(const node& n, const std::string_view& s) { return n.name() <=> s; }
    static bool operator==(const node& n, const std::string_view& s) { return n.name() == s; }
    static std::strong_ordering operator<=>(const node& a, const node& b) { return a.name() <=> b.name(); }


    node::node(std::vector<node> children, bool children_is_sorted, node *father, std::string name, glm::mat4 transform, node_variant_t other_data, rc<const nodetree> nodetree_reference)
        : m_children(std::move(children)),
          m_children_is_sorted(children_is_sorted),
          m_father(father),
          m_name(std::move(name)),
          m_transform(transform),
          m_other_data(std::move(other_data)),
          m_nodetree_reference(std::move(nodetree_reference))
    {
        // fix name
        for(char&c : m_name)
            if(!std::isalnum(c) && special_chars_allowed_in_node_name.find(c) == std::string_view::npos)
                c = '_';


        // fix children's father pointers
        for (node& child : m_children)
            child.m_father = this;
    }

    node::node(std::string name, node_variant_t other_data, glm::mat4 transform)
        : node(std::vector<node>(), true, nullptr, std::move(name), transform, std::move(other_data), rc<const nodetree>()) {}

    node::node(node&& n)
    : node(std::move(n.m_children), n.m_children_is_sorted, n.m_father, std::move(n.m_name), n.m_transform, std::move(n.m_other_data), std::move(n.m_nodetree_reference))
    {
        n.m_father = nullptr;
    }

    node::node(const node& o) : node(o.m_children, o.m_children_is_sorted, nullptr, o.m_name, o.m_transform, o.m_other_data, o.m_nodetree_reference) {}

    node& node::operator=(node&& o) {
        m_children = std::move(o.m_children);
        m_children_is_sorted = o.m_children_is_sorted;
        m_father = o.m_father;
        m_name = std::move(o.m_name);
        m_transform = o.m_transform;

        return *this;
    }

    node::node(const rc<const nodetree>& nt, std::string name) : node(nt->root()) {
        m_nodetree_reference = std::move(nt);
        if(!name.empty()) m_name = name;
    }

    inline void insert_sorted(std::vector<node>& vec, node item) {
        auto upper_bound = std::upper_bound(vec.begin(), vec.end(), item);
        vec.insert(upper_bound, std::move(item));
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
            auto it = std::lower_bound(m_children.begin(), m_children.end(), name);

            if (it != m_children.end() && it->name() == name) {
                return *it;
            }
        } else {
            auto it = std::find(m_children.begin(), m_children.end(), name);
            if (it != m_children.end()) {
                return *it;
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
    std::span<const node> node::children() const { return m_children; }
    std::span<node> node::children() { return m_children; }

    void node::set_children_sorting_preference(bool v) {
        if(v && !m_children_is_sorted) {
            std::sort(m_children.begin(), m_children.end());
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
}
