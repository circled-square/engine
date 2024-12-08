#include <engine/scene/node/collisions.hpp>



template<>
struct std::hash<glm::vec3> {
    std::size_t operator()(const glm::vec3& v) const noexcept {
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        std::size_t h3 = std::hash<float>{}(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2); // or use boost::hash_combine
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


    collision_shape collision_shape::from_mesh(const void* mesh_verts_ptr, size_t mesh_verts_size, ptrdiff_t offset, ptrdiff_t stride, std::span<const glm::uvec3> mesh_indices){
        using namespace glm;

        auto get_mesh_vert = [&](size_t i) {
            EXPECTS(i < mesh_verts_size);
            return *reinterpret_cast<const glm::vec3*>(reinterpret_cast<const char*>(mesh_verts_ptr) + offset + (stride * i));
        };

        //TODO: make these into unordered_set<vec3> instead
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

            normals.insert(normalize_without_verse(normal));


            //add edges
            edges.insert(normalize_without_verse(edge_1));
            edges.insert(normalize_without_verse(edge_2));
            edges.insert(normalize_without_verse(edge_3));
        }
        // TODO: possible optimization: precalculate min&max for each axis of the shape itself so we don't need to do it every time (but only need it for the other shape's axes
        // TODO: add support for AABB optimization, possibly skipping edge-edge collisions

        collision_shape ret;
        for(vec3 vert : verts)      ret.verts.push_back(vert);
        for(vec3 normal : normals)  ret.face_normals.push_back(normal);
        for(vec3 edge : edges)      ret.edges.push_back(edge);


        return ret;
    }

    collision_result collision_result::null() { return {glm::vec3(0), std::numeric_limits<float>::quiet_NaN()}; }

    bool collision_result::is_shallow() const { EXPECTS(this->operator bool()); return depth == 0.f; }

    vec3 collision_result::get_min_translation() { EXPECTS(this->operator bool()); return -depth * versor; }

    collision_result::operator bool() const { return !std::isnan(depth); }
}
