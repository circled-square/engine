#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    using glm::mat4;
    static std::string fix_name(std::string s) {
        for(char& c : s) {
            if(!std::isalnum(c) && node::special_chars_allowed_in_node_name.find(c) == std::string_view::npos)
                c = '_';
        }
        return s;
    }

    node::node(std::string name, node_data_variant_t other_data, const mat4& transform)
        : m_children(),
          m_children_is_sorted(true),
          m_father(),
          m_name(fix_name(std::move(name))),
          m_transform(std::move(transform)),
          m_global_transform_cache(),
          m_other_data(std::move(other_data)),
          m_nodetree_bp_reference(),
          m_script() {}

    noderef noderef::deep_copy_internal(rc<const node> o) {
        noderef n = noderef(o->m_name, o->m_other_data, o->m_transform,
                            o->m_script.has_value() ? o->m_script->get_underlying_stateless_script() : nullptr);


        n->m_nodetree_bp_reference = o->m_nodetree_bp_reference;
        n->m_col_behaviour = o->m_col_behaviour;
        n->set_children_sorting_preference(o->m_children_is_sorted);
        for(int i = 0; i < o->m_children.size(); i++) {
            n->m_children.push_back(o->m_children[i].deep_copy());
        }

        // fix the children's father pointers
        for(noderef child : n->m_children) {
            child->m_father = n;
        }

        return n;
    }

    noderef::noderef(std::string name, node_data_variant_t other_data, const glm::mat4& transform, rc<const stateless_script> script)
        : m_node(get_rm().new_mut_emplace<node>(std::move(name), std::move(other_data), transform)) {
        if(script)
            attach_script(std::move(script));
    }

    noderef::noderef(rc<const nodetree_blueprint> nt, std::string name)
        : noderef(noderef::deep_copy_internal(nt->root())) {
        if(!name.empty())
            m_node->m_name = name;
        m_node->m_nodetree_bp_reference = std::move(nt);
    }

    noderef::noderef(noderef&& o) : m_node(std::move(o.m_node)) {}

    noderef& noderef::operator=(noderef&& o) { m_node = std::move(o.m_node); return *this; }

    noderef& noderef::operator=(const noderef& o) { m_node = o.m_node; return *this; }

    noderef::noderef(const noderef& o) : m_node(o.m_node) {}

    noderef::noderef(rc<node> n) : m_node(std::move(n)) {
        EXPECTS(m_node);
    }

    noderef noderef::deep_copy() const {
        return noderef::deep_copy_internal(this->m_node);
    }

    noderef::operator rc<node>() const { return m_node; }

    noderef::operator rc<const node>() const { return m_node; }

    noderef::operator weak<node>() const { return m_node; }

    noderef::operator weak<const node>() const { return (weak<node>)m_node; }

    void noderef::add_child(noderef c) const {
        m_node->add_child(*this, c);
    }

    void noderef::attach_script(rc<const stateless_script> s) const {
        m_node->attach_script(*this, std::move(s));
    }

    void noderef::process(application_channel_t& app_chan) const {
        m_node->process(*this, app_chan);
    }

    node& noderef::operator*() const { return *m_node; }

    node* noderef::operator->() const { return &*m_node; }

    void node::add_child(const noderef& self, const noderef& c) {
        c->m_father = self;

        if(m_children_is_sorted) {
            auto upper_bound = std::upper_bound(self->m_children.begin(), self->m_children.end(), c,
                                                [](const rc<node>& a, const rc<node>& b) { return a->name() < b->name(); });
            self->m_children.emplace(upper_bound, std::move(c));
        } else {
            self->m_children.emplace_back(std::move(c));
        }
    }

    noderef node::get_child(std::string_view name) {
        if(m_children_is_sorted) {
            auto less_than = [](const rc<node>& n, const std::string_view& s) { return n->name() < s; };
            if(auto it = std::lower_bound(m_children.begin(), m_children.end(), name, less_than); it != m_children.end() && (*it)->name() == name) {
                return *it;
            }
        } else {
            if (auto it = std::ranges::find_if(m_children, [&name](const rc<node>& n){ return n->name() == name; }); it != m_children.end()) {
                return *it;
            }
        }
        throw std::runtime_error(std::format("node at path '{}' has no child named '{}'", absolute_path(), name));
    }


    rc<node> node::get_father() {
        return m_father.lock();
    }

    rc<const node> node::get_father() const {
        return m_father.lock();
    }

    rc<node> node::get_father_checked() {
        if(rc<node> f = m_father.lock(); f)
            return std::move(f);
        else
            throw std::runtime_error(std::format("node with name '{}' has no father, but called node::get_father()", name()));
    }


    rc<const node> node::get_father_checked() const {
        if(rc<node> f = m_father.lock(); f)
            return std::move(f);
        else
            throw std::runtime_error(std::format("node with name '{}' has no father, but called node::get_father()", name()));
    }

    const_children_span node::children() const {
        return const_children_span(std::span(m_children.begin(), m_children.end()));
    }
    std::span<const noderef> node::children() { return std::span(m_children.begin(), m_children.end()); }

    void node::set_children_sorting_preference(bool v) {
        if(v && !m_children_is_sorted) {
            std::sort(m_children.begin(), m_children.end(),
                      [](const rc<node>& a, const rc<node>& b) { return a->name() < b->name(); });
        }
        m_children_is_sorted = v;
    }

    std::string node::absolute_path() const {
        rc<const node> f = get_father();
        return f ? f->absolute_path() + "/" + m_name : m_name;
    }
    const std::string& node::name() const { return m_name; }

    const mat4& node::transform() const { return m_transform; }

    void node::set_transform(const glm::mat4& m) {
        invalidate_global_transform_cache();
        this->m_transform = m;
    }

    const mat4& node::get_global_transform() const {
        if(!m_global_transform_cache.has_value()) {
            rc<const node> f = get_father();
            if(f) {
                m_global_transform_cache = f->get_global_transform() * transform();
            } else {
                m_global_transform_cache = transform();
            }
        }
        ENSURES(m_global_transform_cache.has_value());

        return *m_global_transform_cache;
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
                rc<node> father_p = node_cursor->get_father();
                mat4 father_inverse_globtrans = father_p ? glm::inverse(father_p->get_global_transform()) : mat4(1);

                glm::vec3 local_space_min_translation = father_inverse_globtrans * glm::vec4(-res.get_min_translation(), 0);

                node_cursor->set_transform(glm::translate(node_cursor->transform(), -local_space_min_translation));
            }
            if(col_behaviour.passes_events_to_script) {
                EXPECTS(node_cursor->m_script.has_value());
                node_cursor->pass_collision_to_script(res, *this, other);
            }

            //keep recursing up the node tree if the event needs to be passed to the father
            if(col_behaviour.passes_events_to_father == true) {
                if(rc<node> father = node_cursor->get_father(); father) {
                    node_cursor = &*father;
                }
            } else {
                break;
            }
        }
    }

    void node::set_script_state(std::any s) {
        EXPECTS(m_script.has_value());
        m_script->set_state(std::move(s));
    }

    void node::attach_script(const noderef& self, rc<const stateless_script> s) {
        m_script = script(std::move(s), self);
    }

    void node::process(const noderef& self, application_channel_t& app_chan) { if(m_script) m_script->process(self, app_chan); }

    void node::invalidate_global_transform_cache() const {
        if(m_global_transform_cache.has_value()) {
            m_global_transform_cache.reset();
            for(rc<const node> c : children()) {
                c->invalidate_global_transform_cache();
            }
        }
    }

    void node::pass_collision_to_script(collision_result res, node& ev_src, node& other) { if(m_script) m_script->react_to_collision(*this, res, ev_src, other); }

    noderef node::get_from_path(std::string_view path) {
        std::string_view subpath = path;
        node* current_node = this;
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
    template<> bool node::has<collision_shape>() const { return has<rc<const collision_shape>>(); }
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



    nonorphan_node_move_exception::nonorphan_node_move_exception(node& n)
        : std::exception(), m_what(std::format("attempted to move node '{}' which is not an orphan", n.name())) {}

    const char* nonorphan_node_move_exception::what() const noexcept {
        return m_what.c_str();
    }

}

