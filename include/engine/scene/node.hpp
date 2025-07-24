#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <span>
#include <optional>
#include <GAL/framebuffer.hpp>
#include "node/script.hpp"
#include "node/node_data_concept.hpp"
#include "node/narrow_phase_collision.hpp"
#include <engine/resources_manager/rc.hpp>
#include <engine/resources_manager/weak.hpp>

/* TODO: the titular node class has too many responsibilities and is too complex, or at least this is what it feels like.
 * Try to think of ways to split its behaviour into different classes. However, complexity has to lie somewhere; if this
 * needs to be a complex class then do not split it, but hopefully this is not the case.
 * A possibility could be to split the generic tree functionality from the engine primitive functionality, somehow.
 */

namespace engine {
    //all type declarations are later defined in this translation unit
    class nodetree_blueprint;
    class const_children_span;

    class noderef {
        rc<node> m_node;

        static noderef deep_copy_internal(rc<const node> o);

    public:
        //this class's responsibilities are the following:
        // - simplifies construction ("get_rm().new_mut_emplace<node>(bla bla bla)" becomes "noderef(bla bla bla)")
        // - makes it possible to set father pointer for children (add_child, deep_copy)
        // - everything in node still accessible through operators *, ->
        // - unlike rc<node> or rc<const node> it is valid to assume that a noderef always points to something, except after it was just moved out of

        noderef() = delete;
        noderef(noderef&& o);
        noderef& operator=(noderef&& o);
        noderef& operator=(const noderef& o);
        noderef(const noderef& o);
        explicit noderef(rc<node> n);
        explicit noderef(std::string name, node_data_variant_t other_data = std::monostate(), const glm::mat4& transform = glm::mat4(1), rc<const stateless_script> script = nullptr);
        //this is expensive (calls deep_copy)
        explicit noderef(rc<const nodetree_blueprint> nt, std::string name = "");
        noderef deep_copy() const;

        operator rc<node>() const;
        operator rc<const node>() const;
        operator weak<node>() const;
        operator weak<const node>() const;

        void add_child(noderef c) const;
        void attach_script(rc<const stateless_script> s) const;
        void process(application_channel_t& app_chan) const;

        node& operator*() const;
        node* operator->() const;
    };

    class node {
        friend class noderef;
        void add_child(const noderef& self, const noderef& c);
        void attach_script(const noderef& self,rc<const stateless_script> s);
        void process(const noderef& self, application_channel_t& app_chan);

        std::vector<noderef> m_children;
        bool m_children_is_sorted;
        weak<node> m_father;
        std::string m_name;
        glm::mat4 m_transform;

        /* Global transform cache:
         * - set_transform calls invalidate_global_transform_cache
         * - invalidate_transform_cache invalidates the node's and all its children's cache, but only if its cache was valid to begin with
         *   (it is assumed that a node that has an invalid cache will never have descendants with a valid cache, and conversely a node with a valid cache will never have ancestors with an invalid cache.)
         * - get_global_transform computes the transform only if the cache is invalid, in which case populates for this and all ancestors
         *
         * Note:
         *      a node will only validate the cache of itself and its ANCESTORS, and
         *      invalidate the cache of itself and its DESCENDANTS
         *
         */
        mutable std::optional<glm::mat4> m_global_transform_cache;
        void invalidate_global_transform_cache() const;

        node_data_variant_t m_other_data;
        rc<const nodetree_blueprint> m_nodetree_bp_reference; // reference to the nodetree blueprint this was built from, if any, to keep its refcount up
        collision_behaviour m_col_behaviour;

        std::optional<script> m_script;

    public:
        //only these chars and alphanumeric chars (std::alnum) are allowed in node names; others are automatically replaced with '_'.
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        explicit node(std::string name, node_data_variant_t other_data, const glm::mat4& transform);
        explicit node(rc<const nodetree_blueprint> nt, std::string name); // this is expensive (calls deep-copy constructor)

        node() = delete;
        node(const node&) = delete;
        node(node&&) = delete;

        // children access
        noderef get_child(std::string_view name);
        const_children_span children() const;
        std::span<const noderef> children();
        void set_children_sorting_preference(bool v); // sets whether the children vector is sorted. sorted -> fast O(logn) search, slow O(n) insert; unsorted -> slow O(n) search, fast O(1) insert.
        // get node with relative path
        noderef get_from_path(std::string_view path);

        // father access
        rc<node> get_father();
        rc<const node> get_father() const;
        rc<node> get_father_checked(); // throws if father is null
        rc<const node> get_father_checked() const; // throws if father is null

        //name access
        const std::string& name() const;
        std::string absolute_path() const;

        //transform access
        const glm::mat4& transform() const;
        void set_transform(const glm::mat4& m);
        const glm::mat4& get_global_transform() const;

        const collision_behaviour& get_collision_behaviour();
        void set_collision_behaviour(collision_behaviour col_behaviour);

        void react_to_collision(collision_result res, node& other);

        //script
        void set_script_state(std::any s);
        void process(application_channel_t& app_chan);
        void pass_collision_to_script(collision_result res, node& ev_src, node& other);

        // special node data access
        template<Resource T> requires NodeData<rc<const T>> bool     has() const;
        template<Resource T> requires NodeData<rc<const T>> const T& get() const;

        template<NodeData T> bool     has() const;
        template<NodeData T> const T& get() const;
        template<NodeData T> T&       get();
    };

    class node_exception : public std::exception {
        std::string m_name, m_child_name;
        mutable std::string m_what;
    public:
        enum class type { NO_SUCH_CHILD, NO_FATHER };
    private:
        type m_type;

    public:
        node_exception(type t, std::string name, std::string child_name = "") : m_type(t), m_name(std::move(name)), m_child_name(std::move(child_name)) {}

        type get_type() { return m_type; }

        virtual const char* what() const noexcept;
    };

    //TODO: move this class outside of this header
    // "nodetree_blueprint" is what we call a preconstructed, immutable node tree (generally loaded from file) which can be copied repeatedly to be instantiated
    class nodetree_blueprint {
        rc<const node> m_root;
        std::string m_name;
    public:
        nodetree_blueprint(rc<const node> root, std::string name) : m_root(std::move(root)), m_name(std::move(name)) {}
        const std::string& name() const { return m_name; }
        rc<const node> root() const { return m_root; }
    };

    class const_children_span {
        std::span<const noderef> m_span;
    public:
        class iterator {
            std::span<const noderef>::iterator m_it;
        public:
            iterator(std::span<const noderef>::iterator it) : m_it(it) {}
            void operator++() { m_it++; }
            bool operator!=(const iterator& o) const { return m_it != o.m_it; }
            rc<const node> operator*() { return m_it->operator rc<const node>(); }
        };

        const_children_span(std::span<const noderef> span) : m_span(span) {}
        iterator begin() { return iterator(m_span.begin()); }
        iterator end() { return iterator(m_span.end()); }
    };
}

#endif // ENGINE_SCENE_NODE_HPP
