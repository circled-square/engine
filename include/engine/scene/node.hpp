#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <variant>
#include <span>
#include <GAL/framebuffer.hpp>
#include "../renderer.hpp"

namespace engine {
    using framebuffer = gal::framebuffer<rc<gal::texture>>;

    //later defined in this translation unit
    class nodetree;

    //currently unimplemented
    class collision_shape {};

    // a viewport node is composed of the fbo to which its descendants should be rendered, and the shader to be applied when rendering the resulting texture.
    // the internal fbo texture's size is kept consistent to the resolution it should be rendered to (through the use of output_resolution_changed).
    class viewport {
        //members are mutable because output_resolution_changed does not fundamentally change what they are and it needs to be const
        mutable framebuffer m_fbo;
        mutable material m_postfx_material;
        std::optional<glm::vec2> m_dynamic_size_relative_to_output;
        const camera* m_active_camera = nullptr;
    public:
        //note: the postfx material at this stage contains a null pointer to a texture.
        viewport(framebuffer fbo, rc<const shader> postfx_shader, std::optional<glm::vec2> dynamic_size_relative_to_output = std::nullopt);
        viewport(rc<const shader> postfx_shader, glm::vec2 dynamic_size_relative_to_output);
        viewport(viewport&& o);

        explicit viewport(const viewport& o);

        // this function can be called from a const&, but returns a mut&. this is because
        framebuffer& fbo() { return m_fbo; }
        const framebuffer& fbo() const { return m_fbo; }
        material& postfx_material() { return m_postfx_material; }
        const material& postfx_material() const { return m_postfx_material; }
        std::optional<glm::vec2> dynamic_size_relative_to_output() const { return m_dynamic_size_relative_to_output; }

	void bind_draw() const { m_fbo.bind_draw(); }

        //can be set to null
        void set_active_camera(const camera* c) { m_active_camera = c; }
        //can return null
        const camera* get_active_camera() const { return m_active_camera; }

        // note: it is recommended to resize viewports sparingly, since it requires allocating a new texture and "leaking" to the gc the old one.
        void output_resolution_changed(glm::ivec2 native_resolution) const;
    };

    struct null_node_data {};

    class node {
        std::vector<node> m_children;
        bool m_children_is_sorted;
        node* m_father;
        std::string m_name;
        glm::mat4 m_transform;

    public:
        using node_variant_t = std::variant<camera, mesh, collision_shape, viewport, null_node_data>;

    private:
        node_variant_t m_other_data;
        rc<const nodetree> m_nodetree_reference; // reference to the nodetree this was built from, if any, to keep its refcount up

        node(std::vector<node> children, bool children_is_sorted, node* father, std::string name, glm::mat4 transform, node_variant_t other_data, rc<const nodetree> nodetree_reference);

    public:
        //only chars in special_chars_allowed_in_node_name and alphanumeric chars (see std::alnum) are allowed in node names; others are automatically replaced with '_'.
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        explicit node(std::string name, node_variant_t other_data = null_node_data(), glm::mat4 transform = glm::mat4(1));
        node(node&& o);
        //this is an expensive deep-copy
        explicit node(const node& o);
        node& operator=(node&& o);

        node(const rc<const nodetree>& nt, std::string name = std::string()); // this is expensive (calls deep-copy constructor)

        // children access
        void add_child(node c);
        node& get_child(std::string_view name);
        std::span<const node> children() const;
        std::span<node> children();
        void set_children_sorting_preference(bool v); // sets whether the children vector is sorted. sorted -> fast O(logn) retrieval, slow O(n) insertion; unsorted -> slow O(n) retrieval, fast O(1) insertion.
        // get node with relative path
        node& get_from_path(std::string_view path);

        // father access
        node& get_father();
        node* try_get_father();

        //name access
        const std::string& name() const;
        std::string absolute_path() const;

        //transform access
        const glm::mat4& transform() const;
        glm::mat4& transform();

        // special node data access
        template<class T> bool 	   has() const { return std::holds_alternative<T>(m_other_data); }
        template<class T> const T& get() const { EXPECTS(has<T>()); return std::get<T>(m_other_data); }
        template<class T> T&       get()       { EXPECTS(has<T>()); return std::get<T>(m_other_data); }
    };

//    std::strong_ordering operator<=>(const std::string_view& s, const node& n);
//    std::strong_ordering operator<=>(const node& n, const std::string_view& s);

//    bool operator==(const node& n, const std::string_view& s);

    // "nodetree" is what we call a preconstructed, immutable subtree of the node tree, generally loaded from file
    class nodetree {
        node m_root;
        std::string m_name;
    public:
        nodetree(node root, std::string name) : m_root(std::move(root)), m_name(std::move(name)) {}
        const std::string& name() const { return m_name; }
        const node& root() const { return m_root; }
    };
}

#endif // ENGINE_SCENE_NODE_HPP
