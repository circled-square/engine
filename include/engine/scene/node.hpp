#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <variant>
#include <span>
#include <optional>
#include <GAL/framebuffer.hpp>
#include "renderer.hpp"
#include "node/node_data_concept.hpp"
#include "node/script.hpp"
#include "node/narrow_phase_collision.hpp"
#include "node/viewport.hpp"
#include "node/camera.hpp"

/* TODO: the titular node class has too many responsibilities and is too complex, or at least this is what it feels like.
 * Try to think of ways to split its behaviour into different classes. However, complexity has to lie somewhere; if this
 * needs to be a complex class then do not split it, but hopefully this is not the case.
 * A possibility could be to split the generic tree functionality from the engine primitive functionality, somehow.
 */

namespace engine {
    //all type declarations are later defined in this translation unit
    class nodetree_blueprint;

    namespace detail { class internal_node; }
    class node_span;
    class const_node_span;

    struct null_node_data {};

    class node {
        friend class detail::internal_node; // node but with move assignment, allows storing children as a sorted vector
        node& operator=(node&& o); // for sorting children nodes
        std::vector<detail::internal_node> m_children;
        bool m_children_is_sorted;
        node* m_father;
        std::string m_name;
        glm::mat4 m_transform;

        node_data_variant_t m_other_data;
        rc<const nodetree_blueprint> m_nodetree_reference; // reference to the nodetree this was built from, if any, to keep its refcount up
        collision_behaviour m_col_behaviour;

        std::optional<script> m_script;

    public:
        //only chars in special_chars_allowed_in_node_name and alphanumeric chars (see std::alnum) are allowed in node names; others are automatically replaced with '_'.
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        explicit node(std::string name, node_data_variant_t other_data = null_node_data(), glm::mat4 transform = glm::mat4(1), std::optional<script> script = {});

        node(node&& o);
        //this is an expensive deep-copy
        explicit node(const node& o);

        // this is expensive (calls deep-copy constructor)
        node(const rc<const nodetree_blueprint>& nt, std::string name = std::string());

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
        /* TODO: Create a global transform cache:
         * - add an attribute
         *     std::optional<glm::mat4> m_global_transform_cache;
         * - and a private method
         *     void invalidate_global_transform_cache();
         * and a setter for transform, which calls invalidate_global_transform_cache;
         * a node on which invalidate_global_transform_cache is called checks whether
         * the cache is currently valid:
         *   - if it is, it invalidates it and calls invalidate_global_transform_cache
         *     on all its children;
         *   - if it isn't it does nothing; it is assumed that a node that has an
         *     invalid cache will never have descendants with a valid cache, and conversely
         *     a node with a valid cache will never have ancestors with an invalid cache.
         * compute_global_transform would then only need to compute the transform if
         * the global transform cache is invalid, and in that case populate the cache
         * for all nodes higher than *this in the hierarchy.
         *
         * Note:
         *      a node will only validate the cache of itself and its ANCESTORS, and
         *      invalidate the cache of itself and its DESCENDANTS
         *
         * BEFORE implementing this check whether this just adds complexity without
         * actually improving performance; keep in mind the average hierarchy to
         * traverse is not too complex, since most of the complexity comes from bones
         * which only need a global_transform to be computed if they have a mesh child.
         * Also keep in mind that currently we already compute global transforms for all
         * nodes when we render (possibly multiple times), and we could reduce this to only
         * mesh nodes and their children while also saving the result for the next use.
         *
         * This saves computation also for nodes that are always still, like environment
         * nodes or nodes only useful for grouping nodes while still allowing those nodes
         * to move should they need to (imagine an elevator), and conversely if a node that
         * generally moves stops moving its global transform will not need to be recomputed
         * until it starts moving again.
         *
         * When you're at that point rename compute_global_transform to get_global_transform
         */
        glm::mat4 compute_global_transform();

        const collision_behaviour& get_collision_behaviour();
        void set_collision_behaviour(collision_behaviour col_behaviour);

        void react_to_collision(collision_result res, node& other);

        //script
        void attach_script(rc<const stateless_script> s);
        void process(application_channel_t& app_chan);
        void pass_collision_to_script(collision_result res, node& ev_src, node& other);

        // special node data access
        template<Resource T> requires NodeData<rc<const T>> bool     has() const;
        template<Resource T> requires NodeData<rc<const T>> const T& get() const;

        template<NodeData T> bool     has() const;
        template<NodeData T> const T& get() const;
        template<NodeData T> T&       get();
    };

    namespace detail {
        // this class is a wrapper around node which allows move assignment, exposing
        // it to the stdlib and allowing to keep the children vector sorted
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
            const node* operator->() const { return &v; }
        };
    }

    //TODO: move this class (and its const variant) outside of this header
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

    //TODO: move this class outside of this header
    // "nodetree_blueprint" is what we call a preconstructed, immutable node tree (generally loaded from file) which can be copied repeatedly to be instantiated
    class nodetree_blueprint {
        node m_root;
        std::string m_name;
    public:
        nodetree_blueprint(node root, std::string name) : m_root(std::move(root)), m_name(std::move(name)) {}
        const std::string& name() const { return m_name; }
        const node& root() const { return m_root; }
    };
}

#endif // ENGINE_SCENE_NODE_HPP
