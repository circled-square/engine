#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <span>
#include <optional>
#include "node/script.hpp"
#include "node/node_payload.hpp"
#include "node/narrow_phase_collision.hpp"
#include "node/node_span.hpp"
#include <engine/resources_manager/rc.hpp>
#include <engine/resources_manager/weak.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/api_macro.hpp>

// TODO: split different classes in this header into separate headers

namespace engine {
    // specifies how a node reacts to collisions
    struct node_collision_behaviour {
        bool moves_away_on_collision : 1 = false;
        bool passes_events_to_script : 1 = false;
        bool passes_events_to_father : 1 = false;
    };

    class nodetree_blueprint;

    /* A node in the scene graph.
     * TODO: better doc comment
     */
    class node {
        std::vector<std::unique_ptr<node>> m_children;
        bool m_children_is_sorted;
        node* m_father;

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
        nullable_rc<const nodetree_blueprint> m_nodetree_bp_reference; // reference to the nodetree blueprint this was built from, if any, to keep its refcount up
        node_collision_behaviour m_col_behaviour;

        std::optional<script> m_script;


    public:
        ENGINE_API void add_child(std::unique_ptr<node> c);

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
         * (relatively expensive, especially for unsorted children), and allows the user to create a lot of nodes whose name is irrelevant
         * without creating useless names for them. Do note that a "useless" name suddenly becomes useful the moment there's debugging to do.
         */
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        static std::unique_ptr<node> make(std::string name, std::optional<stateless_script> s = std::nullopt, const std::any& params = std::monostate(), node_payload_t pl = std::monostate(), const glm::mat4& transform = glm::mat4(1)) {
            return std::make_unique<node>(std::move(name), std::move(pl), std::move(transform), s, params);
        }
        static std::unique_ptr<node> make(std::string name, node_payload_t pl, const glm::mat4& transform = glm::mat4(1)) {
            return node::make(std::move(name), std::nullopt, std::monostate(), std::move(pl), transform);
        }

        // expensive
        ENGINE_API static std::unique_ptr<node> deep_copy(rc<const nodetree_blueprint> nt, std::optional<std::string> name = std::nullopt);
        // expensive
        ENGINE_API static std::unique_ptr<node> deep_copy(const node& o, std::optional<std::string> name = std::nullopt);

        node() = delete;
        node(const node&) = delete;
        node(node&&) = delete;
        node& operator=(const node&) = delete;
        node& operator=(node&&) = delete;
        ~node() = default;

        // this must be ENGINE_API because node::make is defined in-header, and it must be public because std::make_unique needs to be able to access it
        ENGINE_API explicit node(std::string name, node_payload_t payload, const glm::mat4& transform, std::optional<stateless_script>, const std::any& params);

        // get child from name
        ENGINE_API node& get_child(std::string_view name);
        // get a span of the node's children
        const_node_span children() const { return const_node_span(std::span(m_children.begin(), m_children.end())); }
        // get a span of the node's children
        node_span children() { return node_span(std::span(m_children.begin(), m_children.end())); }
        // sets whether the children vector is sorted. sorted -> fast O(logn) search, slow O(n) insert; unsorted -> slow O(n) search, fast O(1) insert.
        ENGINE_API void set_children_sorting_preference(bool v);
        // get node with relative path
        ENGINE_API node& get_descendant_from_path(std::string_view path);

        // get father node, possibly returns null
        node* get_father() { return m_father; }
        // get father node, possibly returns null
        const node* get_father() const { return m_father; }
        // get father node, throws if father is null
        ENGINE_API node& get_father_checked();
        // get father node, throws if father is null
        ENGINE_API const node& get_father_checked() const;

        // get this node's name
        const std::string& name() const { return m_name; }
        // get this node's absolute path in the node hierarchy
        std::string absolute_path() const { return m_father != nullptr ? m_father->absolute_path() + "/" + m_name : m_name; }

        // get this node's local transform
        const glm::mat4& transform() const { return m_transform; }
        // set this node's local transform
        ENGINE_API void set_transform(const glm::mat4& m);
        /* get this node's global transform.
         *
         * Note: the global transform is cached and as such should be fairly inexpensive to compute
         */
        ENGINE_API const glm::mat4& get_global_transform() const;

        // get the collision behaviour: should the node move away when it receives a collision event, and/or pass the event to its script and/or to its father?
        const node_collision_behaviour& get_collision_behaviour() { return m_col_behaviour; }
        // set the collision behaviour: should the node move away when it receives a collision event, and/or pass the event to its script and/or to its father?
        void set_collision_behaviour(node_collision_behaviour col_behaviour) { m_col_behaviour = col_behaviour; }


        // handle collision event, recursing up the node tree if necessary
        void react_to_collision(collision_result res, node& other);

        //script
        // instantiates a script and attaches it to a node; params are for the script's constructor
        ENGINE_API void attach_script(stateless_script s, const std::any& params = std::monostate());
        // attach an already-instantiated script to a node;
        ENGINE_API void attach_script(script s);

        // get this node's script
        std::optional<script>& get_script() { return m_script; }
        // get this node's script
        const std::optional<script>& get_script() const { return m_script; }

        // special node data access
        template<NodePayload T> bool     has() const { return std::holds_alternative<T>(m_payload); }
        template<NodePayload T> const T& get() const { EXPECTS(has<T>()); return std::get<T>(m_payload); }
        template<NodePayload T> T&       get()       { EXPECTS(has<T>()); return std::get<T>(m_payload); }

        //allow has<collision_shape> instead of has<rc<collision_shape>>
        template<Resource T> requires NodePayload<rc<const T>> bool     has() const { return has<rc<const collision_shape>>(); }
        template<Resource T> requires NodePayload<rc<const T>> const T& get() const { return *get<rc<const collision_shape>>(); }

        void set_payload(node_payload_t p) {
            m_payload = std::move(p);
        }
        //separate logic for rc<collision_shape>
        void set_payload(rc<collision_shape> p) {
            m_payload = std::move(p);
        }

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

        ENGINE_API const char* what() const noexcept override;
    };

    // "nodetree_blueprint" is what we call a preconstructed, immutable node tree (generally loaded from file) which can be copied repeatedly to be instantiated
    class nodetree_blueprint {
        std::unique_ptr<node> m_root;
        std::string m_name;
    public:
        nodetree_blueprint(std::unique_ptr<node> root, std::string name) : m_root(std::move(root)), m_name(std::move(name)) { EXPECTS(m_root.get()); }
        const std::string& name() const { return m_name; }
        const node& root() const { EXPECTS(m_root.get()); return *m_root; }

        std::unique_ptr<node> into_node() { return std::move(m_root); }
    };
}

#endif // ENGINE_SCENE_NODE_HPP
