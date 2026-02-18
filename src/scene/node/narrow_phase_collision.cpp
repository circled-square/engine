#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/scene/node.hpp>
#include <slogga/asserts.hpp>
#include <climits> // CHAR_BIT

namespace engine {
    using namespace glm;

    //we care not for the orientation of the normal/edge vectors, so we reverse all with y<0 (some with y=0) so vecs inverse to each other are not counted (since everything goes through a hashset)
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

    static bool project_and_update_min(const vec3& ax, const std::vector<vec3>& a_v, const std::vector<vec3> b_v, mat4 b_to_a, hashset<vec3>& dont_repeat, float& min_coll, vec3& min_coll_vec, mat4 trans_b4_saving) {
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
        hashset<vec3> dont_repeat;

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
            face_normal = normalize_without_verse(face_normal);

            if(!project_and_update_min(face_normal, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans))
                return collision_result::null();

            if(min_col == 0.f)
                return collision_result{ min_col_dir, min_col };
        }


        for(vec3 b_edge_b_space : b.edges) {
            vec3 b_edge = b_to_a_space_trans * vec4(b_edge_b_space, 0.0);

            for(vec3 a_edge : a.edges) {
                if(min_col == 0.f)
                    return collision_result { min_col_dir, min_col };

                vec3 axis = normalize_without_verse(glm::cross(vec3(a_edge), vec3(b_edge)));
                if (std::isnan(axis.x)) { continue; }

                if(!project_and_update_min(axis, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans)) {
                    return collision_result::null();
                }
            }
        }

        return collision_result{ min_col_dir, min_col };
    }




    template<AnyOneOf<uvec3, u16vec3> T>
    inline collision_shape from_mesh_impl(stride_span<const vec3> mesh_verts, std::span<const T> mesh_indices, collision_layers_bitmask is_layers, collision_layers_bitmask sees_layers) {
        using namespace glm;

        auto prepare_for_hashtable = [](glm::vec3 v) {
            // normalize_without_verse ensures 2 parallel vectors are considered the same (for our purposes they are)
            // fractional_round ensures 2 almost (but not quite) identical vectors are considered the same
            // these two functions should allow us to dramatically decrease the number of stored edges and normals

            // Note: although the output will have len=~1.0, it is NOT normalized, since fractional_round does not
            // preserve normalization. glm::normalize should be reapplied to the output if normalization is necessary,
            // but it was not deemed to be.
            return fractional_round(normalize_without_verse(v), 128);
        };

        hashset<vec3> verts;
        hashset<vec3> normals;
        hashset<vec3> edges;

        for(T indices : mesh_indices) {
            for(int i = 0; i < 3; i++) {
                verts.insert(mesh_verts[indices[i]]);
            }

            vec3 edge_1 = mesh_verts[indices.y] - mesh_verts[indices.x];
            vec3 edge_2 = mesh_verts[indices.z] - mesh_verts[indices.x];
            vec3 edge_3 = mesh_verts[indices.z] - mesh_verts[indices.y];

            vec3 normal = glm::cross(edge_1, edge_2);

            normals.insert(prepare_for_hashtable(normal));


            //add edges
            edges.insert(prepare_for_hashtable(edge_1));
            edges.insert(prepare_for_hashtable(edge_2));
            edges.insert(prepare_for_hashtable(edge_3));
        }
        // TODO: possible optimization: precalculate min&max for each axis of the shape itself so we don't need to do it every time (but only need it for the other shape's axes
        // TODO: add support for AABB optimization, possibly skipping edge-edge collisions

        collision_shape ret;
        //copy the hashsets into the collision_shape's vectors
        ret.verts.insert(ret.verts.begin(), verts.begin(), verts.end());
        ret.face_normals.insert(ret.face_normals.begin(), normals.begin(), normals.end());
        ret.edges.insert(ret.edges.begin(), edges.begin(), edges.end());
        ret.is_layers = is_layers;
        ret.sees_layers = sees_layers;


        return ret;
    }

    collision_shape collision_shape::from_mesh(stride_span<const glm::vec3> mesh_verts, std::span<const glm::uvec3> mesh_indices, collision_layers_bitmask is_layers, collision_layers_bitmask sees_layers) {
        return from_mesh_impl<glm::uvec3>(mesh_verts, mesh_indices, is_layers, sees_layers);
    }
    collision_shape collision_shape::from_mesh(stride_span<const glm::vec3> mesh_verts, std::span<const glm::u16vec3> mesh_indices, collision_layers_bitmask is_layers, collision_layers_bitmask sees_layers) {
        return from_mesh_impl<glm::u16vec3>(mesh_verts, mesh_indices, is_layers, sees_layers);
    }

    collision_result collision_result::null() { return {glm::vec3(0), std::numeric_limits<float>::quiet_NaN()}; }

    bool collision_result::is_shallow() const { EXPECTS(this->operator bool()); return depth == 0.f; }

    vec3 collision_result::get_min_translation() const { EXPECTS(this->operator bool()); return -depth * versor; }

    collision_result::operator bool() const { return !std::isnan(depth); }

    collision_layers_bitmask collision_layer(int n) {
        static_assert(sizeof(collision_layers_bitmask) * CHAR_BIT == 64);
        EXPECTS(0 <= n && n < 64);

        return collision_layers_bitmask(1) << n;
    }

}
