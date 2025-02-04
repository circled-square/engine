#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    static std::string fix_name(std::string s) {
        for(char& c : s) {
            if(!std::isalnum(c) && node::special_chars_allowed_in_node_name.find(c) == std::string_view::npos)
                c = '_';
        }
        return s;
    }

    node::node(std::string name, node_data_variant_t other_data, glm::mat4 transform, std::optional<script> script)
        : m_children(),
          m_children_is_sorted(true),
          m_father(nullptr),
          m_name(fix_name(std::move(name))),
          m_transform(std::move(transform)),
          m_other_data(std::move(other_data)),
          m_nodetree_reference(),
          m_script(std::move(script)) {
        if(m_script)
            m_script->attach(*this);
    }

    node::node(node&& o)
        : m_children(std::move(o.m_children)),
          m_children_is_sorted(o.m_children_is_sorted),
          m_father(o.m_father),
          m_name(std::move(o.m_name)),
          m_transform(std::move(o.m_transform)),
          m_other_data(std::move(o.m_other_data)),
          m_nodetree_reference(std::move(o.m_nodetree_reference)),
          m_col_behaviour(std::move(o.m_col_behaviour)),
          m_script(std::move(o.m_script))
    {
        o.m_father = nullptr;
        // fix the children's father pointers
        for(detail::internal_node& child : m_children) {
            child->m_father = this;
        }
    }

    node::node(const node& o)
        : m_children(o.m_children),
          m_children_is_sorted(o.m_children_is_sorted),
          m_father(nullptr),
          m_name(o.m_name),
          m_transform(o.m_transform),
          m_other_data(o.m_other_data),
          m_nodetree_reference(o.m_nodetree_reference),
          m_col_behaviour(o.m_col_behaviour),
          m_script(o.m_script)
    {
        // fix the children's father pointers
        for(detail::internal_node& child : m_children) {
            child->m_father = this;
        }
    }

    node& node::operator=(node&& o) {
        m_children = std::move(o.m_children);
        m_children_is_sorted = o.m_children_is_sorted;

        // fix the children's father pointers
        for(detail::internal_node& child : m_children) {
            child->m_father = this;
        }

        m_father = o.m_father;
        o.m_father = nullptr;

        m_name = std::move(o.m_name);
        m_transform = std::move(o.m_transform);
        m_other_data = std::move(o.m_other_data);
        m_nodetree_reference = std::move(o.m_nodetree_reference);
        m_col_behaviour = std::move(o.m_col_behaviour);
        m_script = std::move(o.m_script);

        return *this;
    }

    node::node(const rc<const nodetree_blueprint>& nt, std::string name) : node(nt->root()) {
        m_nodetree_reference = std::move(nt);
        if(!name.empty()) m_name = fix_name(name);
    }


    inline void insert_sorted(std::vector<detail::internal_node>& vec, node item) {
        auto upper_bound = std::upper_bound(vec.begin(), vec.end(), item,
            [](const node& n, const detail::internal_node& in) { return n.name() < in->name(); });
        vec.insert(upper_bound, detail::internal_node{std::move(item)});
    }


    void node::add_child(node c) {
        c.m_father = this;

        if(m_children_is_sorted) {
            insert_sorted(m_children, std::move(c));
        } else {
            m_children.emplace_back(std::move(c));
        }
    }

    node& node::get_child(std::string_view name) {
        if(m_children_is_sorted) {
            auto less_than = [](const detail::internal_node& n, const std::string_view& s) { return n->name() < s; };
            if(auto it = std::lower_bound(m_children.begin(), m_children.end(), name, less_than); it != m_children.end() && (*it)->name() == name) {
                return **it;
            }
        } else {
            if (auto it = std::ranges::find_if(m_children, [&name](const detail::internal_node& n){ return n->name() == name; }); it != m_children.end()) {
                return **it;
            }
        }
        throw std::runtime_error(std::format("node at path '{}' has no child named '{}'", absolute_path(), name));
    }

    node* node::try_get_father() { return m_father; }

    node& node::get_father() {
        if(m_father)
            return *m_father;
        else
            throw std::runtime_error(std::format("node with name '{}' has no father, but called node::father()", name()));
    }
    const_node_span node::children() const { return m_children; }
    node_span node::children() { return m_children; }

    void node::set_children_sorting_preference(bool v) {
        if(v && !m_children_is_sorted) {
            std::sort(m_children.begin(), m_children.end(), [](const detail::internal_node& a, const detail::internal_node& b) { return a->name() < b->name(); });
            m_children_is_sorted = true;
        } else if(!v && m_children_is_sorted) {
            m_children_is_sorted = false;
        }
    }

    std::string node::absolute_path() const {
        return m_father ? m_father->absolute_path() + "/" + m_name : m_name;
    }
    const std::string& node::name() const { return m_name; }

    const glm::mat4 &node::transform() const { return m_transform; }

    glm::mat4 &node::transform() { return m_transform; }

    glm::mat4 node::compute_global_transform() {
        node* f = try_get_father();
        if(f) {
            return f->compute_global_transform() * transform();
        } else {
            return transform();
        }
    }

    const collision_behaviour& node::get_collision_behaviour() { return m_col_behaviour; }

    void node::set_collision_behaviour(collision_behaviour col_behaviour) {
        m_col_behaviour = col_behaviour;
    }

    void node::react_to_collision(collision_result res, node& other) {
        node* node_cursor = this;
        while(true) {

            auto& col_behaviour = node_cursor->get_collision_behaviour();

            if(col_behaviour.moves_away_on_collision) {
                node* father_p = node_cursor->try_get_father();
                glm::mat4 father_inverse_globtrans = father_p ? glm::inverse(father_p->compute_global_transform()) : glm::mat4(1);

                glm::vec3 local_space_min_translation = father_inverse_globtrans * glm::vec4(-res.get_min_translation(), 0);

                node_cursor->transform() = glm::translate(node_cursor->transform(), -local_space_min_translation);
            }
            if(col_behaviour.passes_events_to_script) {
                EXPECTS(node_cursor->m_script.has_value());
                node_cursor->pass_collision_to_script(res, *this, other);
            }

            //keep recursing up the node tree if the event needs to be passed to the father
            if(col_behaviour.passes_events_to_father == true) {
                node_cursor = &node_cursor->get_father(); //we do not use try_get_father because we do assume the father exists
            } else {
                break;
            }
        }
    }

    void node::attach_script(rc<const stateless_script> s) {
        m_script = script(std::move(s));
        m_script->attach(*this);
    }

    void node::process(application_channel_t& app_chan) { if(m_script) m_script->process(*this, app_chan); }

    void node::pass_collision_to_script(collision_result res, node& ev_src, node& other) { if(m_script) m_script->react_to_collision(*this, res, ev_src, other); }

    node& node::get_from_path(std::string_view path) {
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
    template<> bool     node::has<collision_shape>() const { return has<rc<const collision_shape>>(); }
    template<> const collision_shape& node::get<collision_shape>() const {
        const rc<const collision_shape>& p = get<rc<const collision_shape>>();
        EXPECTS(p);
        return *p;
    }

    template<NodeData T> bool     node::has() const { return std::holds_alternative<T>(m_other_data); }
    template<NodeData T> const T& node::get() const { EXPECTS(has<T>()); return std::get<T>(m_other_data); }
    template<NodeData T> T&       node::get()       { EXPECTS(has<T>()); return std::get<T>(m_other_data); }


#define INSTANTIATE_NODE_TEMPLATES(TYPE) \
    template TYPE& node::get<TYPE>(); \
    template const TYPE& node::get<TYPE>() const; \
    template bool node::has<TYPE>() const;

    CALL_MACRO_FOR_EACH(INSTANTIATE_NODE_TEMPLATES, NODE_DATA_CONTENTS)
}

