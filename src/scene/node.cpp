#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    using glm::mat4;
    static std::string fix_name(std::string s) {
        for(char& c : s) {
            if(!std::isalnum(c) && node_data::special_chars_allowed_in_node_name.find(c) == std::string_view::npos)
                c = '_';
        }
        return s;
    }

    node_data::node_data(std::string name, node_payload_t payload, const mat4& transform)
        : m_children(),
          m_children_is_sorted(true),
          m_father(),
          m_name(fix_name(std::move(name))),
          m_transform(std::move(transform)),
          m_global_transform_cache(),
          m_payload(std::move(payload)),
          m_nodetree_bp_reference(),
          m_script() {}

    node node::deep_copy_internal(rc<const node_data> o) {
        EXPECTS(o);

        node n = node(o->m_name, o->m_payload, o->m_transform,
                            o->m_script.has_value() ? o->m_script->get_underlying_stateless_script() : nullptr);


        n->m_nodetree_bp_reference = o->m_nodetree_bp_reference;
        n->m_col_behaviour = o->m_col_behaviour;
        n->set_children_sorting_preference(o->m_children_is_sorted);
        for(int i = 0; i < o->m_children.size(); i++) {
            n->m_children.push_back(o->m_children[i].deep_copy());
        }

        // fix the children's father pointers
        for(node& child : n->m_children) {
            child->m_father = n;
        }

        ENSURES(n.m_node_data);

        return n;
    }

    node::node(std::string name, node_payload_t payload, const glm::mat4& transform, rc<const stateless_script> script)
        : m_node_data(get_rm().new_emplace<node_data>(std::move(name), std::move(payload), transform)) {
        if(script)
            node_data::attach_script(*this, std::move(script));
        ENSURES(m_node_data);
    }

    node::node(rc<const nodetree_blueprint> nt, std::string name)
        : node(node::deep_copy_internal(nt->root())) {
        if(!name.empty())
            m_node_data->m_name = name;
        m_node_data->m_nodetree_bp_reference = std::move(nt);
        ENSURES(m_node_data);
    }

    node::node(node&& o) : m_node_data(std::move(o.m_node_data)) {
        EXPECTS(m_node_data);
    }

    node& node::operator=(node&& o) {
        m_node_data = std::move(o.m_node_data);
        EXPECTS(m_node_data);
        return *this;
    }

    node& node::operator=(const node& o) {
        m_node_data = o.m_node_data;
        EXPECTS(m_node_data);
        return *this;
    }

    node::node(const node& o) : m_node_data(o.m_node_data) {
        EXPECTS(m_node_data);
    }

    node node::deep_copy() const {
        EXPECTS(m_node_data);
        node ret = node::deep_copy_internal(this->m_node_data);
        ENSURES(ret.m_node_data);
        return ret;
    }

    node::operator rc<node_data>() const { return m_node_data; }
    node::operator rc<const node_data>() const { return m_node_data; }

    node::operator weak<node_data>() const { return m_node_data; }
    node::operator weak<const node_data>() const { return m_node_data; }
    void node::add_child(node c) const {
        node_data::add_child(*this, c);
    }

    node_data& node::operator*() const { return *m_node_data; }

    node_data* node::operator->() const { return &*m_node_data; }

    void node_data::add_child(const node& self, node c) {
        c->m_father = self;

        if(self->m_children_is_sorted) {
            auto compare = [](auto& a, auto& b) { return a->name() < b->name(); };
            auto upper_bound = std::upper_bound(self->m_children.begin(), self->m_children.end(), c, compare);
            self->m_children.emplace(upper_bound, std::move(c));
        } else {
            self->m_children.emplace_back(std::move(c));
        }
    }

    node node_data::get_child(std::string_view name) {
        if(m_children_is_sorted) {
            auto less_than = [](const rc<node_data>& n, const std::string_view& s) { return n->name() < s; };
            if(auto it = std::lower_bound(m_children.begin(), m_children.end(), name, less_than); it != m_children.end() && (*it)->name() == name) {
                return *it;
            }
        } else {
            if (auto it = std::ranges::find_if(m_children, [&name](const rc<node_data>& n){ return n->name() == name; }); it != m_children.end()) {
                return *it;
            }
        }
        throw node_exception(node_exception::type::NO_SUCH_CHILD, m_name, (std::string)name);
    }


    rc<node_data> node_data::get_father() {
        return m_father.lock();
    }

    rc<const node_data> node_data::get_father() const {
        return m_father.lock();
    }

    rc<node_data> node_data::get_father_checked() {
        if(rc<node_data> f = m_father.lock(); f)
            return std::move(f);
        else
            throw node_exception(node_exception::type::NO_SUCH_CHILD, m_name);
    }


    rc<const node_data> node_data::get_father_checked() const {
        if(rc<node_data> f = m_father.lock(); f)
            return std::move(f);
        else
            throw node_exception(node_exception::type::NO_SUCH_CHILD, m_name);
    }

    const_node_span node_data::children() const {
        return const_node_span(std::span(m_children.begin(), m_children.end()));
    }
    std::span<const node> node_data::children() { return std::span(m_children.begin(), m_children.end()); }

    void node_data::set_children_sorting_preference(bool v) {
        if(v && !m_children_is_sorted) {
            std::sort(m_children.begin(), m_children.end(),
                      [](const rc<node_data>& a, const rc<node_data>& b) { return a->name() < b->name(); });
        }
        m_children_is_sorted = v;
    }

    std::string node_data::absolute_path() const {
        rc<const node_data> f = get_father();
        return f ? f->absolute_path() + "/" + m_name : m_name;
    }
    const std::string& node_data::name() const { return m_name; }

    const mat4& node_data::transform() const { return m_transform; }

    void node_data::set_transform(const glm::mat4& m) {
        invalidate_global_transform_cache();
        this->m_transform = m;
    }

    const mat4& node_data::get_global_transform() const {
        if(!m_global_transform_cache.has_value()) {
            rc<const node_data> f = get_father();
            if(f) {
                m_global_transform_cache = f->get_global_transform() * transform();
            } else {
                m_global_transform_cache = transform();
            }
        }
        ENSURES(m_global_transform_cache.has_value());

        return *m_global_transform_cache;
    }

    const node_collision_behaviour& node_data::get_collision_behaviour() { return m_col_behaviour; }

    void node_data::set_collision_behaviour(node_collision_behaviour col_behaviour) {
        m_col_behaviour = col_behaviour;
    }

    void node_data::react_to_collision(collision_result res, node_data& other) {
        node_data* node_cursor = this;
        while(true) {
            auto& col_behaviour = node_cursor->get_collision_behaviour();

            if(col_behaviour.moves_away_on_collision) {
                rc<node_data> father_p = node_cursor->get_father();
                mat4 father_inverse_globtrans = father_p ? glm::inverse(father_p->get_global_transform()) : mat4(1);

                glm::vec3 local_space_min_translation = father_inverse_globtrans * glm::vec4(-res.get_min_translation(), 0);

                node_cursor->set_transform(glm::translate(node_cursor->transform(), -local_space_min_translation));
            }
            if(col_behaviour.passes_events_to_script) {
                EXPECTS(node_cursor->m_script.has_value());

                // pass the collision event to the node's script
                if(node_cursor->m_script)
                    node_cursor->m_script->react_to_collision(*node_cursor, res, *this, other);
                //node_cursor->pass_collision_to_script(res, *this, other);
            }

            //keep recursing up the node tree if the event needs to be passed to the father
            if(col_behaviour.passes_events_to_father == true) {
                if(rc<node_data> father = node_cursor->get_father(); father) {
                    node_cursor = &*father;
                }
            } else {
                break;
            }
        }
    }

    void node_data::set_script_state(std::any s) {
        EXPECTS(m_script.has_value());
        m_script->set_state(std::move(s));
    }

    std::optional<script>& node_data::get_script() { return m_script; }

    const std::optional<script>& node_data::get_script() const { return m_script; }

    void node_data::attach_script(const node& self, rc<const stateless_script> s) {
        self->m_script = script(std::move(s), self);
    }

    void node_data::invalidate_global_transform_cache() const {
        if(m_global_transform_cache.has_value()) {
            m_global_transform_cache.reset();
            for(const rc<const node_data>& c : children()) {
                c->invalidate_global_transform_cache();
            }
        }
    }

    node node_data::get_descendant_from_path(std::string_view path) {
        std::string_view subpath = path;
        node_data* current_node = this;
        while(true) {
            size_t separator_position = subpath.find('/');
            if (separator_position == std::string_view::npos) {
                //could not find separator; base case
                return current_node->get_child(std::string(subpath));
            } else {
                std::string_view next_step = subpath.substr(0, separator_position);
                current_node = &*current_node->get_child(std::string(next_step));
                subpath.remove_prefix(separator_position + 1);
            }
        }
    }
    template<> bool node_data::has<collision_shape>() const { return has<rc<const collision_shape>>(); }
    template<> const collision_shape& node_data::get<collision_shape>() const {
        const rc<const collision_shape>& p = get<rc<const collision_shape>>();
        EXPECTS(p);
        return *p;
    }

    template<NodePayload T> bool     node_data::has() const { return std::holds_alternative<T>(m_payload); }
    template<NodePayload T> const T& node_data::get() const { EXPECTS(has<T>()); return std::get<T>(m_payload); }
    template<NodePayload T> T&       node_data::get()       { EXPECTS(has<T>()); return std::get<T>(m_payload); }
    void node_data::set_payload(node_payload_t p) {
        m_payload = std::move(p);
    }

#define INSTANTIATE_NODE_TEMPLATES(TYPE) \
    template TYPE& node_data::get<TYPE>(); \
    template const TYPE& node_data::get<TYPE>() const; \
    template bool node_data::has<TYPE>() const;

    CALL_MACRO_FOR_EACH(INSTANTIATE_NODE_TEMPLATES, NODE_PAYLOAD_CONTENTS)

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

