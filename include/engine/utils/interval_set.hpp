#ifndef ENGINE_UTILS_INTERVAL_SET_HPP
#define ENGINE_UTILS_INTERVAL_SET_HPP

#include "slogga/log.hpp"
#include <set>
#include <concepts>
#include <sstream>

namespace engine {
    namespace detail {
        // for types that define ++ but not +1
        inline auto get_successor(auto v) {
            auto ret = v;
            ret++;
            return ret;
        }

        // for types that define -- but not -1
        inline auto get_predecessor(auto v) {
            auto ret = v;
            ret--;
            return ret;
        }
    }

    template<std::integral T>
    class interval_set {
    public:
        struct interval_t {
            T a, b;
            std::size_t size() const { return b - a + 1; }
        };
        struct comparator_t { bool operator()(const interval_t& x, const interval_t& y) const { return x.a < y.a; } };
    private:
        std::size_t m_size = 0; // number of single elements inserted (i.e. covered by one interval)
        std::set<interval_t, comparator_t> m_intervals;
    public:
        // it is unchecked whether this should actually be inserted at the end (which can make insertion slower) and more importantly whether it overlaps with other intervals
        void insert_at_end(interval_t v) {
            EXPECTS(!m_intervals.contains(v)); // TODO: this check is incomplete
            m_size += v.size();
            m_intervals.emplace_hint(m_intervals.end(), v);
        }

        void insert(T v) {
            using detail::get_successor;
            using detail::get_predecessor;

            // first interval [a,_] s.t. a >= v (or end())
            const auto lower_bound_it = m_intervals.lower_bound({v, 0});
            // v can only be contained in it if v=a
            if(lower_bound_it != m_intervals.end() && lower_bound_it->a == v) {
                return;
            }

            // last interval [a,_] s.t. a < v (or end())
            auto last_smaller_it = [&](){
                if(lower_bound_it == m_intervals.begin()) {
                    return m_intervals.end();
                }
                return get_predecessor(lower_bound_it);
            }();

            // check if the last interval w/ a < b contains v
            if(last_smaller_it != m_intervals.end() && last_smaller_it->b >= v) {
                EXPECTS(last_smaller_it->a <= v);
                return;
            }


            interval_t new_interval{v, v}; // default value for if we just need to add an interval containing only v
            auto insertion_position_hint = lower_bound_it;

            // check if v should be added to the last smaller interval, to the lower bound or to both (in which case merge them)
            if (
                last_smaller_it != m_intervals.end() && lower_bound_it != m_intervals.end()
                && last_smaller_it->b + 1 == v && lower_bound_it->a - 1 == v
            ) {
                // v is adjacent to both last_smaller and lower_bound
                new_interval = {last_smaller_it->a, lower_bound_it->b};
                insertion_position_hint = lower_bound_it;
                insertion_position_hint++;
                m_intervals.erase(last_smaller_it, get_successor(lower_bound_it)); // erase both intervals from the set
            } else if (last_smaller_it != m_intervals.end() && last_smaller_it->b + 1 == v) {
                // v is adjacent to last_smaller
                new_interval = {last_smaller_it->a, v};
                m_intervals.erase(last_smaller_it);
            } else if (lower_bound_it != m_intervals.end() && lower_bound_it->a - 1 == v) {
                new_interval = {v, lower_bound_it->b};
                insertion_position_hint = get_successor(lower_bound_it);
                m_intervals.erase(lower_bound_it);
            }

            m_intervals.emplace_hint(insertion_position_hint, new_interval);
            m_size++;
        }

        bool empty() const {
            // slogga::stdout_log("m_size = {}; m_intervals.empty() = {}; m_intervals.size() = {}", m_size, m_intervals.empty(), m_intervals.size());
            ASSERTS((m_size == 0) == m_intervals.empty());
            return m_intervals.empty();
        }

        T extract_first_element() {
            EXPECTS(!empty());
            m_size--;
            interval_t first_interval = *m_intervals.begin();
            m_intervals.erase(m_intervals.begin());
            if(first_interval.a + 1 <= first_interval.b) {
                m_intervals.emplace_hint(m_intervals.begin(), first_interval.a + 1, first_interval.b);
            }

            return first_interval.a;
        }
        interval_t peek_last_interval() const { EXPECTS(!empty()); return *detail::get_predecessor(m_intervals.end()); }
        void erase_last_interval() {
            EXPECTS(!empty());
            auto last_interval_iter = detail::get_predecessor(m_intervals.end());
            m_size -= last_interval_iter->size();
            m_intervals.erase(last_interval_iter);
        }

        std::size_t intervals_count() const { return m_intervals.size(); }
        std::size_t size() const { return m_size; }
    };
}

#endif // ENGINE_UTILS_INTERVAL_SET_HPP
