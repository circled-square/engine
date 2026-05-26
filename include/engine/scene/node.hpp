#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <optional>
#include "node/script.hpp"
#include "node/node_payload.hpp"
#include "node/narrow_phase_collision.hpp"
#include "node/node_span.hpp"
#include "node/collision_behaviour.hpp"
#include <engine/resources_manager/rc.hpp>
#include <engine/resources_manager/weak.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/api_macro.hpp>
#include <slogga/log.hpp>

// TODO: split different classes in this header into separate headers

namespace engine {
    class nodetree_blueprint;

    /* A node in the scene graph.
     * TODO: better doc comment
     */
    class node {
        ecs_id_t m_ecs_id;

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
        void invalidate_global_transform_cache();


    public:

        ENGINE_API void add_child(node c);

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
         *
         * The only other special case is the string ".." which is reserved for referring to the node's father in paths.
         */
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        static node make(std::string name, std::optional<stateless_script> s = std::nullopt, const std::any& params = std::monostate(), node_payload_t pl = std::monostate(), const glm::mat4& transform = glm::mat4(1)) {
            return node(std::move(name), std::move(pl), std::move(transform), s, params);
        }
        static node make(std::string name, node_payload_t pl, const glm::mat4& transform = glm::mat4(1)) {
            return node::make(std::move(name), std::nullopt, std::monostate(), std::move(pl), transform);
        }

        // expensive
        ENGINE_API static node deep_copy(rc<const nodetree_blueprint> nt, std::optional<std::string> name = std::nullopt);
        // expensive
        ENGINE_API static node deep_copy(node o, std::optional<std::string> name = std::nullopt);

        //construct node object from an already valid id
        constexpr node(ecs_id_t id = null_ecs_id) : m_ecs_id(id) {}

        using node_span = map_span<ecs_id_t, node>;

        // this must be ENGINE_API because node::make is defined in-header, and it must be public because std::make_unique needs to be able to access it
        ENGINE_API explicit node(std::string name, node_payload_t payload, const glm::mat4& transform, std::optional<stateless_script> script, const std::any& params);


        // get child from name
        ENGINE_API node get_child(std::string_view name);
        // get a span of the node's children
        ENGINE_API node_span children();
        // sets whether the children vector is sorted. sorted -> fast O(logn) search, slow O(n) insert; unsorted -> slow O(n) search, fast O(1) insert.
        ENGINE_API void set_children_sorting_preference(bool v);
        ENGINE_API bool get_children_sorting_preference() const;
        // get node with relative path
        ENGINE_API node get_descendant_from_path(std::string_view path);

        // get father node, possibly returns null
        node get_father() const { return node{ get_rm().ecs().get_component<ecs_id_t>(component_names::father).get(m_ecs_id) }; }
        // get father node, throws if father is null
        ENGINE_API node get_father_checked() const;

        // get this node's name
        std::string_view name() const { return get_rm().ecs().get_component<std::string>(component_names::name).get_or(m_ecs_id, std::string()); }
        // get this node's absolute path in the node hierarchy
        std::string absolute_path() const { return get_father().ecs_id() != null_ecs_id ? std::format("{}/{}", get_father().absolute_path(), name()) : std::string(name()); }

        // get this node's local transform
        const glm::mat4& transform() const {
            // slogga::stdout_log("[{}].transform()", m_ecs_id);
            return get_rm().ecs().get_component<glm::mat4>(component_names::transform).get(m_ecs_id);
        }
        // set this node's local transform
        ENGINE_API void set_transform(const glm::mat4& m);
        /* get this node's global transform.
         *
         * Note: the global transform is cached and as such should be fairly inexpensive to compute
         */
        ENGINE_API const glm::mat4& get_global_transform() const;

        // get the collision behaviour: should the node move away when it receives a collision event, and/or pass the event to its script and/or to its father?
        const node_collision_behaviour& get_collision_behaviour() { return get_rm().ecs().get_component<node_collision_behaviour>().get(m_ecs_id); }
        // set the collision behaviour: should the node move away when it receives a collision event, and/or pass the event to its script and/or to its father?
        void set_collision_behaviour(node_collision_behaviour col_behaviour) { get_rm().ecs().get_component<node_collision_behaviour>().set(m_ecs_id, col_behaviour); }


        // handle collision event, recursing up the node tree if necessary
        void react_to_collision(collision_result res, node other);

        //script
        // instantiates a script and attaches it to a node; params are for the script's constructor
        ENGINE_API void attach_script(stateless_script s, const std::any& params = std::monostate());
        // attach an already-instantiated script to a node;
        ENGINE_API void attach_script(script s);

        // TODO: treat each payload type as a separate ECS component, and have get/set just fetch it; also have set/get be a generic functions that also works for other components such as script, collision_behaviour,

        // get this node's script
        optional_ref<script> get_script() const { return get_rm().ecs().get_component<script>().try_get(m_ecs_id); }

        // special node data access
        // const node_payload_t& get_payload() const { return get_rm().ecs().get_component<node_payload_t>("payload").get_or(m_ecs_id, node_payload_t(std::monostate()));}
        template<NodePayload T> bool has() const {
            auto& pl = get_rm().ecs().get_component<node_payload_t>().get_or(m_ecs_id, std::monostate());
            return std::holds_alternative<T>(pl);
        }
        template<NodePayload T> T& get() const {
            EXPECTS(has<T>());
            return std::get<T>(get_rm().ecs().get_component<node_payload_t>().get(m_ecs_id));
        }

        //allow has<collision_shape> instead of has<rc<collision_shape>>
        template<Resource T> requires NodePayload<rc<const T>> bool     has() const { return has<rc<const collision_shape>>(); }
        template<Resource T> requires NodePayload<rc<const T>> const T& get() const { return *get<rc<const collision_shape>>(); }

        void set_payload(node_payload_t p) {
            get_rm().ecs().get_component<node_payload_t>().set(m_ecs_id, std::move(p));
        }
        //separate logic for rc<collision_shape>
        void set_payload(rc<collision_shape> p) { get_rm().ecs().get_component<node_payload_t>().set(m_ecs_id, std::move(p)); }

        ecs_id_t ecs_id() const { return m_ecs_id; }
        operator ecs_id_t() const { return ecs_id(); }
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
        //allow construction from string_views since this is the prevalent use case
        node_exception(type t, std::string_view name, std::string_view child_name = "") : node_exception(t, std::string(name), std::string(child_name)) {}

        type get_type() { return m_type; }

        ENGINE_API const char* what() const noexcept override;
    };

    // "nodetree_blueprint" is what we call a preconstructed, immutable node tree (generally loaded from file) which can be copied repeatedly to be instantiated
        //TODO: nodetree_blueprint should be immutable unless by a call through into_node(); right now we have no const_node class but we definitely need one
    class nodetree_blueprint {
        node m_root;
        std::string m_name;
    public:
        nodetree_blueprint(node root, std::string name) : m_root(std::move(root)), m_name(std::move(name)) {}
        const std::string& name() const { return m_name; }
        /*const_*/node root() const { return m_root; }

        node into_node() { return std::move(m_root); }
    };
}

#endif // ENGINE_SCENE_NODE_HPP
