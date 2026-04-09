#include <sstream>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <engine/utils/read_file.hpp>
#include <engine/scene/yaml_loader.hpp>
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>
#include <engine/scene/node.hpp>


//formatter for rapidyaml ConstNodeRef for debugging purposes
template<>
struct std::formatter<ryml::ConstNodeRef, char> {
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) { return ctx.begin(); }

    template<class FmtContext>
    FmtContext::iterator format(ryml::ConstNodeRef v, FmtContext& ctx) const {
        std::ostringstream out;

        out << v;

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

namespace engine {
    inline ryml::ConstNodeRef get_child(ryml::ConstNodeRef n) { return n; }

    inline ryml::ConstNodeRef get_child(ryml::ConstNodeRef n, auto child_name, auto... child_names) {
        EXPECTS_WITH_MSG(n.has_child(child_name), std::format("({}).has_child('{}')", n.id(), child_name));
        return get_child(n[child_name], child_names...);
    }

    inline std::optional<ryml::ConstNodeRef> get_optional_child(ryml::ConstNodeRef n) { return n; }

    inline std::optional<ryml::ConstNodeRef> get_optional_child(ryml::ConstNodeRef n, auto child_name, auto... child_names) {
        if(n.has_child(child_name))
            return get_optional_child(n[child_name], child_names...);
        else
            return std::nullopt;
    }

    inline std::string_view ryml_substr_to_std_string_view(ryml::csubstr s) { return std::string_view(s.begin(), s.end()); }

    inline std::optional<std::string_view> get_optional_val(std::optional<ryml::ConstNodeRef> n) {
        if(n.has_value() && n->has_val())
            return ryml_substr_to_std_string_view(n->val());
        else
            return std::nullopt;
    }

    inline std::string_view get_val(ryml::ConstNodeRef n) {
        EXPECTS_WITH_MSG(n.has_val(), std::format("({}).has_val()", n.id()));
        n.is_val_literal();
        return ryml_substr_to_std_string_view(n.val());
    }

    inline std::string_view get_child_val(ryml::ConstNodeRef n, auto... child_names) {
        return get_val(get_child(n, child_names...));
    }

    inline std::optional<std::string_view> get_optional_child_val(ryml::ConstNodeRef n, auto... child_names) {
        return get_optional_val(get_optional_child(n, child_names...));
    }

    template <class T>
    inline T as_num(std::string_view sv) {
        T ret;
        std::from_chars_result res = std::from_chars(sv.begin(), sv.end(), ret);

        if (res.ec == std::errc::invalid_argument || res.ptr != sv.end()) {
            throw std::invalid_argument{"invalid_argument"};
        } else if (res.ec == std::errc::result_out_of_range) {
            throw std::out_of_range{"out_of_range"};
        }

        return ret;
    }
    template<typename T, size_t N>
    inline std::array<T, N> children_as_array(ryml::ConstNodeRef n) {
        EXPECTS(n.num_children() == N);

        std::array<T, N> ret;
        auto iter = n.cchildren().begin();
        for(size_t i = 0; i < N; i++) {
            ret[i] = as_num<T>(get_val(*iter));
            ++iter;
        }

        return ret;
    }

    template<typename T, size_t N>
    inline glm::vec<N, T> children_as_glm_vec(ryml::ConstNodeRef n) {
        auto arr(children_as_array<float, 3>(n));
        return glm::vec3(arr[0], arr[1], arr[2]);
    }

    template<typename payload_t>
    payload_t simple_dfs(payload_t father_payload, ryml::ConstNodeRef n, const auto& func) {
        auto payload = func(father_payload, n);
        if(n.has_child("children")) {
            for(ryml::ConstNodeRef c : get_child(n, "children").cchildren()) {
                simple_dfs<payload_t>(payload, c, func);
            }
        }

        return payload;
    }

    scene load_scene_from_yaml(const char* filename) {
        std::string file_contents = read_file(filename);
        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(file_contents.c_str()));

        // read from the tree:
        ryml::ConstNodeRef root = get_child(tree.crootref(), "scene", "root");
        node* root_payload = nullptr;

        node* root_raw_ptr = simple_dfs(root_payload, root, [](node* father, ryml::ConstNodeRef n) {
            auto name = get_optional_child_val(n, "name").value_or("");
            auto script = get_optional_child_val(n, "script")
                .and_then([](std::string_view s){
                    auto script_path = s.substr(0, s.find(":"));
                    auto script_name = std::string(s.substr(s.find(":") + 1));

                    stateless_script script = stateless_script::from(get_rm().load<dylib::library>(script_path), script_name.c_str());
                    return std::optional(script);
                });

            glm::mat4 transform = get_optional_child(n, "transform")
                .and_then([](ryml::ConstNodeRef trans_node) -> std::optional<glm::mat4> {
                    glm::vec3 pos = get_optional_child(trans_node, "position")
                        .transform(children_as_glm_vec<float, 3>)
                        .value_or({0, 0, 0});

                    return (std::optional<glm::mat4>)get_optional_child(trans_node, "look_at")
                        .transform([&](ryml::ConstNodeRef look_at_node) {
                            glm::vec3 center = get_optional_child(look_at_node, "center")
                                .transform(children_as_glm_vec<float, 3>)
                                .value_or({0, 0, 0});
                            glm::vec3 up = get_optional_child(look_at_node, "up")
                                .transform(children_as_glm_vec<float, 3>)
                                .value_or({0, 1, 0});

                            return glm::inverse(glm::lookAt(pos, center, up));
                        })
                        .or_else([&]() { return std::optional{ glm::translate(glm::mat4(1), pos) }; })
                        .value();
                })
                .value_or(glm::mat4(1));

            // TODO: handle viewport payload as well
            node_payload_t payload = get_optional_child(n, "payload")
                .and_then([](ryml::ConstNodeRef pl_node) {
                    return get_optional_child_val(pl_node, "type")
                        .and_then([](std::string_view type_str){
                            if(type_str == "camera")
                                return std::optional(node_payload_t(camera()));
                            else
                                return std::optional<node_payload_t>{};
                        });
                })
                .value_or(std::monostate());

            std::optional<rc<const nodetree_blueprint>> blueprint = get_optional_child_val(n, "load")
                .transform([](std::string_view bp_path) {
                    return get_rm().load<nodetree_blueprint>(bp_path);
            });

            // HANDLE SCRIPT PARAMS
            // HANDLE WHATNOT


            std::unique_ptr<node> owning;
            if(blueprint.has_value()) {
                owning = node::deep_copy(*blueprint, std::string(name));
                owning->set_transform(transform * owning->transform());
            } else {
                owning = node::make(std::string(name), std::move(script), std::monostate(), std::move(payload), transform);
            }

            node* ret = owning.get();

            if(father) {
                father->add_child(std::move(owning));
            } else {
                EXPECTS(name.length() == 0);
                owning.release();
            }

            return ret;
        });

        std::unique_ptr<node> scene_root(root_raw_ptr);
        scene s(filename, std::move(scene_root));

        return s;
    }
}