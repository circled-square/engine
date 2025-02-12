#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/scene/node.hpp>


template<>
struct std::hash<glm::vec3> {
    std::size_t operator()(const glm::vec3& v) const noexcept {
        std::hash<float> h;
        return h(v.x) ^ (h(v.y) << 1) ^ (h(v.z) << 2); // or use boost::hash_combine
    }
};

namespace engine {
    using namespace glm;

    //we care not for the orientation of the normal/edge vectors, so we reverse all with y<0 (some with y=0) so vecs inverse to each other are not counted (since everything goes through a unordered_set)
    inline glm::vec3 normalize_without_verse(glm::vec3 v) {
        v = glm::normalize(v);
        if(v.y < 0 || (v.y == 0 && v.x < 0) || (v.x == 0 && v.y == 0 && v.z < 0))
            v = -v;
        return v;
    }

    inline glm::vec3 fractional_round(glm::vec3 v, float precision) {
        glm::vec3 ret;
        for(int i = 0; i < ret.length(); i++)
            ret[i] = std::roundf(v[i] * precision) / precision;
        return ret;
    }

    static std::optional<float> proj_on_axis_and_find_collision(const vec3& axis, const std::vector<vec3>& a_verts, const std::vector<vec3>& b_verts, mat4 b_to_a_space_trans) {
        float min_a_proj = std::numeric_limits<float>::max();
        float max_a_proj = std::numeric_limits<float>::lowest();
        for(const vec3& v : a_verts) {
            float proj = glm::dot(axis, v);
            if(proj < min_a_proj) min_a_proj = proj;
            if(proj > max_a_proj) max_a_proj = proj;
        }

        float min_b_proj = std::numeric_limits<float>::max();
        float max_b_proj = std::numeric_limits<float>::lowest();
        for(const vec3& v_b_space : b_verts) {
            vec3 v = b_to_a_space_trans * vec4(v_b_space, 1);

            float proj = glm::dot(axis, v);
            if(proj < min_b_proj) min_b_proj = proj;
            if(proj > max_b_proj) max_b_proj = proj;
        }

        if (min_a_proj <= max_b_proj && min_b_proj <= max_a_proj) {
            //collision!
            float start_overlap = std::max(min_a_proj, min_b_proj);
            float end_overlap = std::min(max_a_proj, max_b_proj);
            float overlap = end_overlap - start_overlap;

            float double_mid_a = min_a_proj + max_a_proj;
            float double_mid_b = min_b_proj + max_b_proj;
            if(double_mid_b < double_mid_a)
                overlap = -overlap;

            return overlap;
        } else {
            return std::nullopt;
        }
    }

    static bool project_and_update_min(const vec3& ax, const std::vector<vec3>& a_v, const std::vector<vec3> b_v, mat4 b_to_a, std::unordered_set<vec3>& dont_repeat, float& min_coll, vec3& min_coll_vec, mat4 trans_b4_saving) {
        if(dont_repeat.contains(ax))
            return true;
        dont_repeat.insert(ax);

        std::optional<float> coll = proj_on_axis_and_find_collision(ax, a_v, b_v, b_to_a);
        if(coll) {
            if(std::abs(*coll) < std::abs(min_coll)) {
                min_coll_vec = trans_b4_saving * vec4(ax, 0);
                min_coll = *coll;
            }
            return true;
        }
        return false;
    }

    collision_result check_collision(const collision_shape& a, mat4 a_trans, const collision_shape& b, mat4 b_trans) {
        std::unordered_set<vec3> dont_repeat;

        mat4 b_to_a_space_trans = inverse(a_trans) * b_trans;

        float min_col = std::numeric_limits<float>::max();
        vec3 min_col_dir;

        for(vec3 face_normal : a.face_normals) {
            if(!project_and_update_min(face_normal, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans))
                return collision_result::null();

            if(min_col == 0.f)
                return collision_result{ min_col_dir, min_col };
        }

        for(vec3 face_normal_b_space : b.face_normals) {
            vec3 face_normal = b_to_a_space_trans * vec4(face_normal_b_space, 0.0);

            if(!project_and_update_min(face_normal, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans))
                return collision_result::null();

            if(min_col == 0.f)
                return collision_result{ min_col_dir, min_col };
        }


        for(vec3 a_edge : a.edges) {
            for(vec3 b_edge_b_space : b.edges) {
                if(min_col == 0.f)
                    return collision_result { min_col_dir, min_col };

                vec3 b_edge = b_to_a_space_trans * vec4(b_edge_b_space, 0.0);

                vec3 axis = normalize_without_verse(glm::cross(vec3(a_edge), vec3(b_edge)));
                if (std::isnan(axis.x)) { continue; }

                if(!project_and_update_min(axis, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans)) {
                    return collision_result::null();
                }
            }
        }

        return collision_result{ min_col_dir, min_col };
    }

    collision_shape collision_shape::from_mesh(const void* mesh_verts_ptr, size_t mesh_verts_size, ptrdiff_t offset, ptrdiff_t stride, std::span<const glm::uvec3> mesh_indices, collision_layers_bitmask is_layers, collision_layers_bitmask sees_layers) {
        using namespace glm;

        auto get_mesh_vert = [&](size_t i) {
            EXPECTS(i < mesh_verts_size);
            return *reinterpret_cast<const glm::vec3*>(reinterpret_cast<const std::byte*>(mesh_verts_ptr) + offset + (stride * i));
        };
        auto normalize_and_round = [](glm::vec3 v) {
            // normalize_without_verse ensures 2 parallel vectors are considered the same (for our purposes they are)
            // fractional_round ensures 2 almost (but not quite) identical vectors are considered the same
            // these two functions should allow us to dramatically decrease the number of stored edges and normals
            // glm::normalize is reapplied to the output because fractional_round unfortunately denormalizes vectors. (may be unnecessary)
            return glm::normalize(fractional_round(normalize_without_verse(v), 256));
        };

        std::unordered_set<vec3> verts;
        std::unordered_set<vec3> normals;
        std::unordered_set<vec3> edges;

        for(uvec3 indices : mesh_indices) {
            for(int i = 0; i < 3; i++) {
                verts.insert(get_mesh_vert(indices[i]));
            }

            vec3 edge_1 = get_mesh_vert(indices.y) - get_mesh_vert(indices.x);
            vec3 edge_2 = get_mesh_vert(indices.z) - get_mesh_vert(indices.x);
            vec3 edge_3 = get_mesh_vert(indices.z) - get_mesh_vert(indices.y);

            vec3 normal = glm::cross(edge_1, edge_2);

            normals.insert(normalize_and_round(normal));


            //add edges
            edges.insert(normalize_and_round(edge_1));
            edges.insert(normalize_and_round(edge_2));
            edges.insert(normalize_and_round(edge_3));
        }
        // TODO: possible optimization: precalculate min&max for each axis of the shape itself so we don't need to do it every time (but only need it for the other shape's axes
        // TODO: add support for AABB optimization, possibly skipping edge-edge collisions

        collision_shape ret;
        for(vec3 vert : verts)      ret.verts.push_back(vert);
        for(vec3 normal : normals)  ret.face_normals.push_back(normal);
        for(vec3 edge : edges)      ret.edges.push_back(edge);
        ret.is_layers = is_layers;
        ret.sees_layers = sees_layers;


        return ret;
    }

    collision_result collision_result::null() { return {glm::vec3(0), std::numeric_limits<float>::quiet_NaN()}; }

    bool collision_result::is_shallow() const { EXPECTS(this->operator bool()); return depth == 0.f; }

    vec3 collision_result::get_min_translation() const { EXPECTS(this->operator bool()); return -depth * versor; }

    collision_result::operator bool() const { return !std::isnan(depth); }


    void notify_of_collision(node& event_src, node& other_node, collision_result collision) {
        node* this_node_p = &event_src;
        while(true) {
            auto& col_behaviour = this_node_p->get_collision_behaviour();

            if(col_behaviour.moves_away_on_collision) {
                rc<node> father_p = this_node_p->get_father();
                glm::mat4 father_globtrans = father_p ? father_p->get_global_transform() : glm::mat4(1);
                glm::mat4 father_inverse_globtrans = father_p ? glm::inverse(father_globtrans) : glm::mat4(1);

                glm::vec3 local_space_translation_versor = father_inverse_globtrans * glm::vec4(collision.get_min_translation(), 0);
                local_space_translation_versor /= glm::length(local_space_translation_versor);

                glm::vec3 local_space_min_translation = local_space_translation_versor * collision.depth;

                this_node_p->set_transform(glm::translate(this_node_p->transform(), local_space_min_translation));
            }
            if(col_behaviour.passes_events_to_script) {
                this_node_p->pass_collision_to_script(collision, event_src, other_node);
            }

            //keep recursing up the node tree if the event needs to be passed to the father
            if(col_behaviour.passes_events_to_father == true) {
                this_node_p = &*this_node_p->get_father_checked();
            } else {
                break;
            }
        }
    }
}
