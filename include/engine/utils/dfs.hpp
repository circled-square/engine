#ifndef ENGINE_UTILS_DFS_HPP
#define ENGINE_UTILS_DFS_HPP

namespace engine {
    //pre-order dfs, without recursion
    template<Callable<void(node)> callable_t>
    inline void depth_first_traversal(node root, const callable_t& callable) {
        std::vector<node> stack;
        stack.push_back(root);

        while(!stack.empty()) {
            node n = stack.back();
            stack.pop_back();

            //iterate in reverse, so the first child is added last, which means it is visited first
            auto children = n.children();
            for(std::int64_t i = (std::int64_t)children.size()-1; i >= 0; i--) {
                stack.push_back(children[i]); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            }

            callable(n);
        }
    }

    //pre+post-order dfs, without recursion
    template<typename dfs_payload_t, Callable<dfs_payload_t(node, const dfs_payload_t&)> preorder_t, Callable<void(node, const dfs_payload_t&)> postorder_t>
    inline void depth_first_traversal(node root, const dfs_payload_t& root_params, const preorder_t& preorder, const postorder_t& postorder) {
        struct stack_entry_t {
            node n;
            // payload to be passed to children when they are visited, both pre- and post-order
            dfs_payload_t p;
            // index of next child to be visited
            size_t i;
        };
        std::vector<stack_entry_t> stack;

        dfs_payload_t root_payload = preorder(root, root_params); // must be done before root is moved out of

        stack.push_back({ root, std::move(root_payload), 0 });

        while(!stack.empty()) {
            auto& [n, p, i] = stack.back();
            auto n_children = n.children();

            if(i < n_children.size()) {
                node c = n_children[i]; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // i < n_children.size()
                dfs_payload_t c_payload = preorder(c, p); // must be done before c is moved out of
                i++; //do NOT update i after having pushed, as pushing can possibly invalidate the reference
                stack.push_back({ c, std::move(c_payload), 0 });
            } else {
                node m = n; // copy value before destroying the entry
                stack.pop_back();

                if(stack.empty()) {
                    postorder(m, root_params);
                } else {
                    stack_entry_t& father_entry = stack.back();
                    postorder(m, father_entry.p);
                }
            }
        }
    }
}

#endif // ENGINE_UTILS_DFS_HPP
