#ifndef ENGINE_SCENE_NODE_HPP
#define ENGINE_SCENE_NODE_HPP

#include <string>
#include <optional>
#include "node/script.hpp"
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

        // expensive
        ENGINE_API static node deep_copy(rc<const nodetree_blueprint> nt, std::optional<std::string> name = std::nullopt);
        // expensive
        ENGINE_API static node deep_copy(node o, std::optional<std::string> name = std::nullopt);

        //construct node object from an already valid id
        constexpr node(ecs_id_t id = null_ecs_id) : m_ecs_id(id) {}

        using node_span = map_span<ecs_id_t, node>;

        // this must be ENGINE_API because node::make is defined in-header, and it must be public because std::make_unique needs to be able to access it
        ENGINE_API explicit node(std::string name, const glm::mat4& transform = glm::mat4(1.f));

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
            auto& rm = get_rm();

            auto& ecs = rm.ecs();

            auto& component = ecs.get_component<glm::mat4>(component_names::transform);

            const auto& ret = component.get(m_ecs_id);

            return ret;

            // return get_rm().ecs().get_component<glm::mat4>(component_names::transform).get(m_ecs_id);
        }
        // set this node's local transform
        ENGINE_API void set_transform(const glm::mat4& m);
        /* get this node's global transform.
         *
         * Note: the global transform is cached and as such should be fairly inexpensive to compute
         */
        ENGINE_API const glm::mat4& get_global_transform() const;

        // handle collision event, recursing up the node tree if necessary
        void react_to_collision(collision_result res, node other);

        //script
        // instantiates a script and attaches it to a node; params are for the script's constructor
        // returns self
        ENGINE_API node attach_script(stateless_script s, const std::any& params = std::monostate());

        template<typename T>
        optional_ref<T> get() const {
            return get_rm().ecs().get_component<T>().try_get(m_ecs_id);
        }
        template<typename T>
        node set(T v) {
            get_rm().ecs().get_component<T>().set(m_ecs_id, std::move(v));
            return *this;
        }

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
