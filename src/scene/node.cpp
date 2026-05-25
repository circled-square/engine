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
        // TODO: unideal interface, make it better
        : m_ecs_id(get_rm().ecs().make_new_id())
    {
        set_transform(transform);

        EXPECTS(name != ".."); // special name for father node in paths
        get_rm().ecs().get_component<std::string>(component_names::name).set(m_ecs_id, std::move(name));

        if(!std::holds_alternative<std::monostate>(payload)) {
            set_payload(std::move(payload));
        }

        visit_optional(script, [&](auto& s){ attach_script(s, params); });
    }

    node node::deep_copy(node o, std::optional<std::string> name) {
        node_payload_t pl = get_rm().ecs().get_component<node_payload_t>().get_or(o, std::monostate());

        node n = node::make(name.value_or(std::string(o.name())), std::nullopt, std::monostate(), std::move(pl), o.transform());
        if(o.get_script()) {
            // clone the script AND its state
            n.attach_script(*o.get_script());
        }

        auto& ecs = get_rm().ecs();

        if(auto bp = ecs.get_component<rc<const nodetree_blueprint>>().try_get(o)) {
            ecs.get_component<rc<const nodetree_blueprint>>().set(n, *bp);
        }

        auto& col_behaviour_component = ecs.get_component<node_collision_behaviour>();
        col_behaviour_component.set(n, col_behaviour_component.get(o));

        n.set_children_sorting_preference(o.get_children_sorting_preference());
        auto& children_component = ecs.get_component<children_vector>();

        auto& n_children = children_component.get(n);
        auto& o_children = children_component.get(o);
        n_children.vector.reserve(o_children.vector.size());
        n_children.is_sorted = o_children.is_sorted;

        for(node o_child : o_children.vector) {
            node copied_child = node::deep_copy(o_child);
            n_children.vector.push_back(copied_child);
            ecs.get_component<ecs_id_t>(component_names::father).set(copied_child, n); // fix the child's father pointer
        }

        return n;
    }

    node node::deep_copy(rc<const nodetree_blueprint> nt, std::optional<std::string> name) {
        node ret = node::deep_copy(nt->root(), name.value_or(nt->name()));

        get_rm().ecs().get_component<rc<const nodetree_blueprint>>().set(ret, std::move(nt));

        return ret;
    }

    void node::add_child(node c) {
        auto& ecs = get_rm().ecs();
        //set child's father
        ecs.get_component<ecs_id_t>(component_names::father).set(c, m_ecs_id);

        auto& children = ecs.get_component<children_vector>().get(m_ecs_id);

        if(children.is_sorted) {
            const auto& name_component = ecs.get_component<std::string>(component_names::name);
            auto compare = [&](ecs_id_t a, ecs_id_t b) { return name_component.get_or(a, {}) < name_component.get_or(b, {}); };
            auto upper_bound = std::upper_bound(children.vector.begin(), children.vector.end(), c, compare);
            children.vector.emplace(upper_bound, c);
        } else {
            children.vector.emplace_back(c);
        }
    }

    node node::get_child(std::string_view name) {
        auto& children = get_rm().ecs().get_component<children_vector>().get(m_ecs_id);
        auto& children_v = children.vector;
        if(children.is_sorted) {
            auto less_than = [](ecs_id_t n, const std::string_view& s) { return node(n).name() < s; };
            if(auto it = std::lower_bound(children_v.begin(), children_v.end(), name, less_than); it != children_v.end() && node(*it).name() == name) {
                return node(*it);
            }
        } else {
            if (auto it = std::ranges::find_if(children_v, [&name](ecs_id_t n){ return node(n).name() == name; }); it != children_v.end()) {
                return node(*it);
            }
        }
        throw node_exception(node_exception::type::NO_SUCH_CHILD, this->name(), name);
    }
    node::node_span node::children() {
        auto& children = get_rm().ecs().get_component<children_vector>().get(m_ecs_id);
        return node_span(std::span(children.vector.begin(), children.vector.end()));
    }

    node node::get_father_checked() const {
        if(node father = get_father(); father.ecs_id() != null_ecs_id)
            return father;
        else
            throw node_exception(node_exception::type::NO_FATHER, name());
    }

    void node::set_children_sorting_preference(bool v) {
        auto& children = get_rm().ecs().get_component<children_vector>().get(m_ecs_id);

        if(v && !children.is_sorted) {
            std::sort(children.vector.begin(), children.vector.end(),
                [](ecs_id_t a, ecs_id_t b) { return node(a).name() < node(b).name(); }
            );
        }
        children.is_sorted = v;
    }

    bool node::get_children_sorting_preference() const {
        auto& children = get_rm().ecs().get_component<children_vector>().get(m_ecs_id);
        return children.is_sorted;
    }

    void node::set_transform(const glm::mat4& m) {
        invalidate_global_transform_cache();

        get_rm().ecs().get_component<glm::mat4>(component_names::transform).set(m_ecs_id, m);
    }

    const mat4& node::get_global_transform() const {
        optional_ref<glm::mat4> cache = get_rm().ecs().get_component<glm::mat4>(component_names::global_transform_cache).try_get(m_ecs_id);
        if(cache) {
            return *cache;
        } else {
            node f = get_father();
            glm::mat4 value = f.ecs_id() != null_ecs_id ? f.get_global_transform() * transform() : transform();

            cache = optional_ref<glm::mat4>(value);

            return get_rm().ecs().get_component<glm::mat4>(component_names::global_transform_cache).set(m_ecs_id, value);
        }
    }

    void node::react_to_collision(collision_result res, node other) {
        node node_cursor = m_ecs_id;
        while(true) {
            const auto& col_behaviour = node_cursor.get_collision_behaviour();

            if(col_behaviour.moves_away_on_collision) {
                node father = node_cursor.get_father();
                mat4 father_inverse_globtrans = father.ecs_id() != null_ecs_id ? glm::inverse(father.get_global_transform()) : mat4(1);

                glm::vec3 local_space_min_translation = father_inverse_globtrans * glm::vec4(-res.get_min_translation(), 0);

                node_cursor.set_transform(glm::translate(node_cursor.transform(), -local_space_min_translation));
            }
            if(col_behaviour.passes_events_to_script) {
                EXPECTS(node_cursor.get_script().has_value());

                // pass the collision event to the node's script
                if(node_cursor.get_script())
                    node_cursor.get_script()->react_to_collision(node_cursor, res, *this, other);
            }

            //keep recursing up the node tree if the event needs to be passed to the father
            if(col_behaviour.passes_events_to_father) {
                if(node father = node_cursor.get_father(); father.ecs_id() != null_ecs_id) {
                    node_cursor = father;
                }
            } else {
                break;
            }
        }
    }

    void node::attach_script(stateless_script sc, const std::any& params) {
        attach_script(script(std::move(sc), node(m_ecs_id), params));
    }

    void node::attach_script(script sc) {
        get_rm().ecs().get_component<script>().set(m_ecs_id, std::move(sc));
    }


    void node::invalidate_global_transform_cache() {
        if(get_rm().ecs().get_component<glm::mat4>(component_names::global_transform_cache).uninit_for_entity(m_ecs_id)) {
            for(node c : children()) {
                c.invalidate_global_transform_cache();
            }
        }
    }

    node node::get_descendant_from_path(std::string_view path)  {
        std::string_view subpath = path;
        node current_node = m_ecs_id;
        while(true) {
            size_t separator_position = subpath.find('/');
            if (separator_position == std::string_view::npos) {
                //could not find separator; base case
                return current_node.get_child(std::string(subpath));
            } else {
                std::string_view next_step = subpath.substr(0, separator_position);
                if(next_step == "..") {
                    current_node = current_node.get_father_checked();
                } else {
                    current_node = current_node.get_child(std::string(next_step));
                }
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

