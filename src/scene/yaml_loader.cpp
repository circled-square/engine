#include <sstream>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <engine/utils/read_file.hpp>
#include <engine/scene/yaml_loader.hpp>
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>
#include <engine/scene/node.hpp>


//formatter for rapidyaml ConstNodeRef
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
        return ryml_substr_to_std_string_view(n.val());
    }

    inline std::string_view get_child_val(ryml::ConstNodeRef n, auto... child_names) {
        return get_val(get_child(n, child_names...));
    }

    inline std::optional<std::string_view> get_optional_child_val(ryml::ConstNodeRef n, auto... child_names) {
        return get_optional_val(get_optional_child(n, child_names...));
    }

    template<typename payload_t>
    payload_t simple_dfs(payload_t father_payload, ryml::ConstNodeRef n, const auto& func) {
        auto payload = func(father_payload, n);
        if(n.has_child("children")) {
            for(ryml::ConstNodeRef c : get_child(n, "children").cchildren()) {
                simple_dfs<payload_t>(payload, c, func);
            }
        }

        return std::move(payload);
    }
    std::unique_ptr<node> yaml_example() {
        std::string file_contents = read_file("assets/example.yml");
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
            // HANDLE GLTF
            // HANDLE WHATNOT

            std::unique_ptr<node> owning = node::make(std::string(name), std::move(script));
            node* ret = owning.get();

            if(father) {
                father->add_child(std::move(owning));
            } else {
                EXPECTS(name.length() == 0);
                owning.release();
            }

            // slogga::stdout_log.warn("visiting {} with script {}", name, script);
            return ret;
        });

        std::unique_ptr<node> scene_root(root_raw_ptr);

        return scene_root;
    }
}