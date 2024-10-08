#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <variant>
#include <span>
#include <GAL/framebuffer.hpp>
#include "../renderer.hpp"
#include "../concepts.hpp"
#include "node/script.hpp"

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

        framebuffer& fbo();
        const framebuffer& fbo() const;
        material& postfx_material();
        const material& postfx_material() const;
        std::optional<glm::vec2> dynamic_size_relative_to_output() const;

        void bind_draw() const;

        //can be set to null
        void set_active_camera(const camera* c);
        //can return null
        const camera* get_active_camera() const;

        // note: it is recommended to resize viewports sparingly, since it requires allocating a new texture and "leaking" to the gc the old one.
        void output_resolution_changed(glm::ivec2 native_resolution) const;

        void operator=(viewport&& o);
    };

    struct null_node_data {};

    namespace detail { class internal_node; }
    class node_span;
    class const_node_span;

    class node {
        std::vector<detail::internal_node> m_children;
        bool m_children_is_sorted;
        node* m_father;
        std::string m_name;
        glm::mat4 m_transform;

        special_node_data_variant_t m_other_data;
        rc<const nodetree> m_nodetree_reference; // reference to the nodetree this was built from, if any, to keep its refcount up

        std::optional<script> m_script;

        friend class detail::internal_node;
        node& operator=(node&& o); // useful for sorting children nodes, but it is bad for it to be exposed like this
    public:
        //only chars in special_chars_allowed_in_node_name and alphanumeric chars (see std::alnum) are allowed in node names; others are automatically replaced with '_'.
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        explicit node(std::string name, special_node_data_variant_t other_data = null_node_data(), glm::mat4 transform = glm::mat4(1), std::optional<script> script = {});

        node(node&& o);
        //this is an expensive deep-copy
        explicit node(const node& o);

        // this is expensive (calls deep-copy constructor)
        node(const rc<const nodetree>& nt, std::string name = std::string());

        // children access
        void add_child(node c);
        node& get_child(std::string_view name);
        const_node_span children() const;
        node_span children();
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

        //script
        void attach_script(rc<const stateless_script> s) {
            m_script = script(std::move(s));
            m_script->attach(*this);
        }
        void process(application_channel_t& app_chan) { if(m_script) m_script->process(*this, app_chan); }

        // special node data access
        template<SpecialNodeData T> bool     has() const;
        template<SpecialNodeData T> const T& get() const;
        template<SpecialNodeData T> T&       get();
    };

    namespace detail {
        // this class is a wrapper around node which allows move assignment, exposing to the stdlib and allowing it to keep the children vector sorted
        class internal_node {
            node v;
        public:
            internal_node(internal_node&& o) : v(std::move(o.v)) {}
            internal_node(const internal_node& o) : v(o.v) {}
            internal_node(node&& o) : v(std::move(o)) {}
            internal_node& operator=(internal_node&& o) {
                v = std::move(o.v);
                return *this;
            }
            node& operator*() { return v; }
            const node& operator*() const { return v; }
            node* operator->() { return &v; }
            const node* operator->() const { return &v; }        };
    }

    class node_span {
        std::vector<detail::internal_node>& m_vec;
    public:
        struct const_iterator {
            const detail::internal_node* p;
            const_iterator(const detail::internal_node* p) : p(p){ }
            void operator++() { p++; }
            const node& operator*() { return **p; }
            bool operator!=(const const_iterator& o) { return p != o.p; }
        };
        struct iterator {
            detail::internal_node* p;
            iterator(detail::internal_node* p) : p(p){ }
            void operator++() { p++; }
            node& operator*() { return **p; }
            bool operator!=(iterator& o) { return p != o.p; }
            operator const_iterator() { return const_iterator{p}; }
        };

        node_span(std::vector<detail::internal_node>& vec) : m_vec(vec) {}
        node_span(node_span& o) : m_vec(o.m_vec) {}

        node& operator[](size_t i) { return *m_vec[i]; }
        const node& operator[](size_t i) const { return *m_vec[i]; }

        iterator begin() { return m_vec.data(); }
        const_iterator begin() const { return m_vec.data(); }
        iterator end() { return m_vec.data() + m_vec.size(); }
        const_iterator end() const { return m_vec.data() + m_vec.size(); }
    };
    class const_node_span {
        const std::vector<detail::internal_node>& m_vec;
    public:
        const_node_span(const std::vector<detail::internal_node>& vec) : m_vec(vec) {}
        const_node_span(const_node_span& o) : m_vec(o.m_vec) {}

        const node& operator[](size_t i) const { return *m_vec[i]; }

        node_span::const_iterator begin() const { return m_vec.data(); }
        node_span::const_iterator end() const { return m_vec.data() + m_vec.size(); }
    };

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
