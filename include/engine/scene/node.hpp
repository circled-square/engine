#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <span>
#include <optional>
#include <GAL/framebuffer.hpp>
#include "node/script.hpp"
#include "node/node_payload.hpp"
#include "node/narrow_phase_collision.hpp"
#include <engine/resources_manager/rc.hpp>
#include <engine/resources_manager/weak.hpp>

// TODO: consider splitting different classes in this header into separate headers

namespace engine {
    class nodetree_blueprint;
    class const_node_span;


    /* A node in the scene graph.
     * TODO: better doc comment
     */
    class node {
        friend class const_node;
        rc<node_data> m_node_data;

        static node deep_copy_internal(rc<const node_data> o);

    public:
        // constructors and assignment operators
        node() = delete;
        node(node&& o);
        node& operator=(node&& o);
        node& operator=(const node& o);
        node(const node& o);
        explicit node(std::string name, node_payload_t payload = std::monostate(), const glm::mat4& transform = glm::mat4(1), rc<const stateless_script> script = nullptr);
        //this is expensive (calls deep_copy)
        explicit node(rc<const nodetree_blueprint> nt, std::string name = "");
        node deep_copy() const;

        operator rc<node_data>() const;
        operator rc<const node_data>() const;
        operator weak<node_data>() const;
        operator weak<const node_data>() const;

        void add_child(node c) const;

        node_data& operator*() const;
        node_data* operator->() const;
    };

    class const_node {
        node m_node;
    public:
        const_node() = delete;
        const_node(const_node&& o) : m_node(std::move(o.m_node)) {}
        const_node& operator=(const_node&& o) { m_node = std::move(o.m_node); return *this; }
        const_node& operator=(const_node& o) { m_node = o.m_node; return *this; }
        const_node(const const_node& o) :m_node(o.m_node) {}
        const_node(node o) : m_node(std::move(o)) {}

        explicit const_node(std::string name, node_payload_t payload = std::monostate(), const glm::mat4& transform = glm::mat4(1), rc<const stateless_script> script = nullptr) : m_node(name, payload, transform, script) {}
        //this is expensive (calls deep_copy)
        explicit const_node(rc<const nodetree_blueprint> nt, std::string name = "") : m_node(nt, name) {}
        node deep_copy() const { return m_node.deep_copy(); }

        operator rc<const node_data>() const { return m_node; }
        operator weak<const node_data>() const { return m_node; }

        const node_data& operator*() const { return *m_node; }
        const node_data* operator->() const { return m_node.operator->(); }
    };

    struct node_collision_behaviour {
        bool moves_away_on_collision : 1 = false;
        bool passes_events_to_script : 1 = false;
        bool passes_events_to_father : 1 = false;
    };

    class node_data {
        friend class node;

        std::vector<node> m_children;
        bool m_children_is_sorted;
        weak<node_data> m_father;
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

        node_payload_t m_payload;
        rc<const nodetree_blueprint> m_nodetree_bp_reference; // reference to the nodetree blueprint this was built from, if any, to keep its refcount up
        node_collision_behaviour m_col_behaviour;

        std::optional<script> m_script;
    public:
        static void add_child(const node& self, node c);

        /*
         * Only these chars and alphanumeric chars (std::alnum) are allowed in node names; others are automatically replaced with '_'.
         *
         * This is, mainly, to avoid problems like assigning file names as node names and ending up with node names containing '/',
         * which is reserved for use as a separator in node paths (e.g. '/root_node/child_node').
         *
         * This character list may be expanded in the future, but characters in it should be considered "stable" and users can expect them
         * to remain in the list.
         *
         * Do note that it is allowed by the engine to create multiple nodes with the same exact name and path; this is not recommended since
         * it makes retrieving that node by name unpredictable; this behaviour for now is kept since it allows the engine to skip this check
         * (relatively expensive, especially for unsorted children),and allows the user to create a lot of nodes whose name is irrelevant
         * without creating useless names for them. Do note that a "useless" name suddenly becomes useful the moment there's debugging to do.
         */
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        explicit node_data(std::string name, node_payload_t payload, const glm::mat4& transform);
        // this is expensive (calls deep-copy constructor)
        explicit node_data(rc<const nodetree_blueprint> nt, std::string name);

        node_data() = delete;
        node_data(const node_data&) = delete;
        node_data(node_data&&) = delete;

        // get child from name
        node get_child(std::string_view name);
        // get a span of the node's children
        const_node_span children() const;
        // get a span of the node's children
        std::span<const node> children();
        // sets whether the children vector is sorted. sorted -> fast O(logn) search, slow O(n) insert; unsorted -> slow O(n) search, fast O(1) insert.
        void set_children_sorting_preference(bool v);
        // get node with relative path
        node get_from_path(std::string_view path);

        // get father node, possibly returns null
        rc<node_data> get_father();
        // get father node, possibly returns null
        rc<const node_data> get_father() const;
        // get father node, throws if father is null
        rc<node_data> get_father_checked();
        // get father node, throws if father is null
        rc<const node_data> get_father_checked() const;

        // get this node's name
        const std::string& name() const;
        // get this node's absolute path in the node hierarchy
        std::string absolute_path() const;

        // get this node's local transform
        const glm::mat4& transform() const;
        // set this node's local transform
        void set_transform(const glm::mat4& m);
        /* get this node's global transform.
         *
         * Note: the global transform is cached and as such should be fairly inexpensive to compute
         */
        const glm::mat4& get_global_transform() const;

        // get the collision behaviour: should the node move away when it receives a collision event, and/or pass the event to its script and/or to its father?
        const node_collision_behaviour& get_collision_behaviour();
        // set the collision behaviour: should the node move away when it receives a collision event, and/or pass the event to its script and/or to its father?
        void set_collision_behaviour(node_collision_behaviour col_behaviour);

        // handle collision event, recursing up the node tree if necessary
        void react_to_collision(collision_result res, node_data& other);

        //script
        // attach a script to a node; requires a node because the script construction must be able to access everything (not just the node data)
        static void attach_script(const node& self, rc<const stateless_script> s);
        // explicitly set the script's state
        void set_script_state(std::any s);
        // get this node's script
        std::optional<script>& get_script();
        // get this node's script
        const std::optional<script>& get_script() const;

        // special node data access
        // TODO: what is this lol
        template<Resource T> requires NodePayload<rc<const T>> bool     has() const;
        template<Resource T> requires NodePayload<rc<const T>> const T& get() const;

        template<NodePayload T> bool     has() const;
        template<NodePayload T> const T& get() const;
        template<NodePayload T> T&       get();
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

    // "nodetree_blueprint" is what we call a preconstructed, immutable node tree (generally loaded from file) which can be copied repeatedly to be instantiated
    class nodetree_blueprint {
        rc<const node_data> m_root;
        std::string m_name;
    public:
        nodetree_blueprint(rc<const node_data> root, std::string name) : m_root(std::move(root)), m_name(std::move(name)) {}
        const std::string& name() const { return m_name; }
        rc<const node_data> root() const { return m_root; }
    };

    /* const_node_span is used for iterating on the children of a node in an immutable manner.
     * we cannot simply use std::span<const const_node> because we cannot construct a span of const_node over a vector of node
     */
    class const_node_span {
        std::span<const node> m_span;
    public:
        class iterator {
            std::span<const node>::iterator m_it;
        public:
            iterator(std::span<const node>::iterator it) : m_it(it) {}
            void operator++() { m_it++; }
            bool operator!=(const iterator& o) const { return m_it != o.m_it; }
            const_node operator*() { return *m_it; }
        };

        const_node_span(std::span<const node> span) : m_span(span) {}
        iterator begin() { return iterator(m_span.begin()); }
        iterator end() { return iterator(m_span.end()); }
    };
}

#endif // ENGINE_SCENE_NODE_HPP
