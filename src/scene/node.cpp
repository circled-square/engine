#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/format_glm.hpp>
#include <slogga/log.hpp>

namespace engine {
    using glm::mat4;
    static std::string fix_name(std::string s) {
        for(char& c : s) {
            if(std::isalnum(c) != 0 && node::special_chars_allowed_in_node_name.contains(c)) {
                c = '_';
            }
        }
        return s;
    }

    node::node(std::string name, node_payload_t payload, const glm::mat4& transform, std::optional<stateless_script> script, const std::any& params)
        : m_children_is_sorted(true),
          m_father(nullptr),
          m_name(fix_name(std::move(name))),
          m_payload(std::move(payload)),
          m_ecs_id(get_rm().get_ecs().make_new_id({"transform"})) // TODO: unideal interface, make it better
    {
        set_transform(transform);
        visit_optional(script, [&](auto& s){ attach_script(s, params); });
    }

    node::~node() {
        get_rm().get_ecs().release_id(m_ecs_id);
    }



    std::unique_ptr<node> node::deep_copy(const node& o, std::optional<std::string> name) {
        std::unique_ptr<node> n = node::make(name.value_or(o.m_name), std::nullopt, std::monostate(), o.m_payload, o.transform());
        if(o.get_script().has_value()) {
            // clone the script AND its state
            n->attach_script(*o.get_script());
        }

        n->m_nodetree_bp_reference = o.m_nodetree_bp_reference;
        n->m_col_behaviour = o.m_col_behaviour;
        n->set_children_sorting_preference(o.m_children_is_sorted);
        for(int i = 0; i < o.m_children.size(); i++) {
            n->m_children.push_back(node::deep_copy(*o.m_children[i])); //NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // i < o.m_children.size()
        }

        // fix the children's father pointers
        for(std::unique_ptr<node>& child : n->m_children) {
            child->m_father = n.get();
        }

        return n;
    }

    std::unique_ptr<node> node::deep_copy(rc<const nodetree_blueprint> nt, std::optional<std::string> name) {
        std::unique_ptr<node> ret = node::deep_copy(nt->root(), name.value_or(nt->name()));
        ret->m_nodetree_bp_reference = nt;

        return ret;
    }

    void node::add_child(std::unique_ptr<node> c) {
        c->m_father = this;

        if(m_children_is_sorted) {
            auto compare = [](auto& a, auto& b) { return a->name() < b->name(); };
            auto upper_bound = std::upper_bound(m_children.begin(), m_children.end(), c, compare);
            m_children.emplace(upper_bound, std::move(c));
        } else {
            m_children.emplace_back(std::move(c));
        }
    }

    node& node::get_child(std::string_view name) {
        if(m_children_is_sorted) {
            auto less_than = [](const std::unique_ptr<node>& n, const std::string_view& s) { return n->name() < s; };
            if(auto it = std::lower_bound(m_children.begin(), m_children.end(), name, less_than); it != m_children.end() && (*it)->name() == name) {
                return **it;
            }
        } else {
            if (auto it = std::ranges::find_if(m_children, [&name](const std::unique_ptr<node>& n){ return n->name() == name; }); it != m_children.end()) {
                return **it;
            }
        }
        throw node_exception(node_exception::type::NO_SUCH_CHILD, m_name, (std::string)name);
    }

    node& node::get_father_checked() {
        if(m_father != nullptr)
            return *m_father;
        else
            throw node_exception(node_exception::type::NO_FATHER, m_name);
    }


    const node& node::get_father_checked() const {
        if(m_father != nullptr)
            return *m_father;
        else
            throw node_exception(node_exception::type::NO_FATHER, m_name);
    }

    void node::set_children_sorting_preference(bool v) {
        if(v && !m_children_is_sorted) {
            std::sort(m_children.begin(), m_children.end(),
                [](const std::unique_ptr<node>& a, const std::unique_ptr<node>& b) { return a->name() < b->name(); }
            );
        }
        m_children_is_sorted = v;
    }

    void node::set_transform(const glm::mat4& m) {
        invalidate_global_transform_cache();

        get_rm().get_ecs().get_component<glm::mat4>("transform").set(m_ecs_id, m);
        // m_transform = m;
    }

    const mat4& node::get_global_transform() const {
        const glm::mat4* cache = get_rm().get_ecs().get_component<glm::mat4>("global_transform_cache").try_get(m_ecs_id);

        if(cache == nullptr) {
            const node* f = get_father();
            glm::mat4 value = f != nullptr ? f->get_global_transform() * transform() : transform();

            cache = &get_rm().get_ecs().get_component<glm::mat4>("global_transform_cache").set(m_ecs_id, value);
        }

        return *cache;
    }

    void node::react_to_collision(collision_result res, node& other) {
        node* node_cursor = this;
        while(true) {
            const auto& col_behaviour = node_cursor->get_collision_behaviour();

            if(col_behaviour.moves_away_on_collision) {
                const node* father_p = node_cursor->get_father();
                mat4 father_inverse_globtrans = father_p != nullptr ? glm::inverse(father_p->get_global_transform()) : mat4(1);

                glm::vec3 local_space_min_translation = father_inverse_globtrans * glm::vec4(-res.get_min_translation(), 0);

                node_cursor->set_transform(glm::translate(node_cursor->transform(), -local_space_min_translation));
            }
            if(col_behaviour.passes_events_to_script) {
                EXPECTS(node_cursor->m_script.has_value());

                // pass the collision event to the node's script
                if(node_cursor->m_script)
                    node_cursor->m_script->react_to_collision(*node_cursor, res, *this, other);
            }

            //keep recursing up the node tree if the event needs to be passed to the father
            if(col_behaviour.passes_events_to_father) {
                if(node* father = node_cursor->get_father(); father) {
                    node_cursor = father;
                }
            } else {
                break;
            }
        }
    }

    void node::attach_script(stateless_script sc, const std::any& params) {
        m_script = script(std::move(sc), *this, params);
    }

    void node::attach_script(script sc) {
        m_script = std::move(sc);
    }


    void node::invalidate_global_transform_cache() const {
        if(get_rm().get_ecs().get_component<glm::mat4>("global_transform_cache").uninit_for_entity(m_ecs_id)) {
            for(const node& c : children()) {
                c.invalidate_global_transform_cache();
            }
        }
    }

    node& node::get_descendant_from_path(std::string_view path) {
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
    const char* node_exception::what() const noexcept {
        if(m_what.empty()) {
            switch(m_type) {
            case type::NO_SUCH_CHILD:
                m_what = std::format("node at path '{}' has no child named '{}'", m_name, m_child_name);
                break;
            case type::NO_FATHER:
                m_what = std::format("node with name '{}' has no father, but called node::get_father_checked()", m_name);
            }
        }
        return m_what.c_str();
    }
}

