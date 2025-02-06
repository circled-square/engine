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

    struct null_node_data {};

    class node {
        std::vector<node> m_children;
        bool m_children_is_sorted;
        bool m_currently_sorting_children;
        node* m_father;
        std::string m_name;
        glm::mat4 m_transform;

        node_data_variant_t m_other_data;
        rc<const nodetree_blueprint> m_nodetree_bp_reference; // reference to the nodetree blueprint this was built from, if any, to keep its refcount up
        collision_behaviour m_col_behaviour;

        std::optional<script> m_script;
    public:
        //only chars in special_chars_allowed_in_node_name and alphanumeric chars (see std::alnum) are allowed in node names; others are automatically replaced with '_'.
        static constexpr std::string_view special_chars_allowed_in_node_name = "_-.,!?:; @#%^&*()[]{}<>|~";

        explicit node(std::string name, node_data_variant_t other_data = null_node_data(), glm::mat4 transform = glm::mat4(1), std::optional<script> script = {});

        node() = delete;

        //move construction is only allowed if the argument is an orphan node; otherwise a nonorphan_node_move_exception is thrown
        node(node&& o);
        //move assignment is only allowed if the argument is an orphan node; otherwise a nonorphan_node_move_exception is thrown
        node& operator=(node&& o);

        //this is an expensive deep-copy
        explicit node(const node& o);

        // this is expensive (calls deep-copy constructor)
        node(const rc<const nodetree_blueprint>& nt, std::string name = std::string());

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

    //thrown when a move constructor/assignment-operator is called with as argument a non-orphan node
    class nonorphan_node_move_exception : public std::exception {
        std::string m_what;
    public:
        nonorphan_node_move_exception() = delete;
        nonorphan_node_move_exception(const nonorphan_node_move_exception&) = default;
        nonorphan_node_move_exception(nonorphan_node_move_exception&&) = default;
        nonorphan_node_move_exception(node& n)
            : std::exception(), m_what(std::format("attempted to move node '{}' which is not an orphan", n.name())) {}
        virtual const char* what() const noexcept {
            return m_what.c_str();
        }
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
