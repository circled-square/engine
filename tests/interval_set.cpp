#include <engine/utils/interval_set.hpp>
#include <engine/entity_component_system.hpp> // just to import ecs_id_t so we can test on it specifically
#include <set>
#include <random>
#include <iostream>
#include <chrono>
#include <ranges>

using engine::ecs_id_t;
using engine::interval_set;

using ref_impl_t = std::set<ecs_id_t>;

bool check_consistency(ref_impl_t correct, interval_set<ecs_id_t> maybe_incorrect) {
    if(correct.size() != maybe_incorrect.size()) {
        return false;
    }
    for(ecs_id_t correct_el : correct) {
        ecs_id_t maybe_incorrect_el = maybe_incorrect.extract_first_element();

        if(maybe_incorrect_el != correct_el) {
            return false;
        }
    }

    return maybe_incorrect.empty();
}

template<class T, size_t N = T::state_size>
T make_seeded() {
    // typename T::result_type random_data[N];
    // std::random_device source;
    // std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    // std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    std::seed_seq seeds({ 0, 1, 2, 3 }); // using some predefined numbers instead of std::random_device because we want the testing to be deterministic
    T engine(seeds);
    return engine;
}

constexpr ecs_id_t max_id = 0xff'ff'ff; // fairly high max id but not high enough to make this too slow
constexpr std::uint64_t repetitions = max_id;
constexpr char remove_probability = 50; // expressed as a percentage

inline void erase_first(std::set<ecs_id_t>& s) { s.erase(s.begin()); }
inline void erase_first(interval_set<ecs_id_t>& s) { auto _ = s.extract_first_element(); }

inline void insert(std::set<ecs_id_t>& s, ecs_id_t e) { s.insert(e); }
inline void insert(interval_set<ecs_id_t>& s, ecs_id_t e) { s.insert(e); }

std::vector<ecs_id_t> build_insertions_vector(auto rng) {
    auto all_acceptable_values = std::ranges::iota_view{0, (int32_t)max_id};
    std::vector<ecs_id_t> insertions;

    std::sample(std::begin(all_acceptable_values), std::end(all_acceptable_values), std::back_inserter(insertions), repetitions, rng);

    return insertions;
}

template<typename T>
std::chrono::milliseconds random_operations(T& set) {
    static std::vector<ecs_id_t> insertions = build_insertions_vector(make_seeded<std::mt19937_64>()); // use a separate instantiation of the rng to make sure it stays untouched, this way the vector can be static

    auto rng = make_seeded<std::mt19937_64>();
    std::uniform_int_distribution<char> distr(1, 100); //NOLINT(cppcoreguidelines-avoid-magic-numbers) // 1-100 allows expressing probability as percentage

    auto t1 = std::chrono::high_resolution_clock::now();

    for(std::size_t i = 0; i < insertions.size(); i++) {
        // insert
        insert(set, insertions[i]);

        if(distr(rng) <= remove_probability) { // erase
            if(!set.empty()) {
                erase_first(set);
            }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
}

int main() {
    interval_set<ecs_id_t> maybe_incorrect;
    ref_impl_t correct;

    auto d_ref = random_operations(correct);
    auto d_impl = random_operations(maybe_incorrect);

    std::cout << "std::set     took " << d_ref << " with size " << correct.size() << " (0x" << std::hex << correct.size() << ")" << std::dec << std::endl;
    std::cout << "interval_set took " << d_impl << " and created " << maybe_incorrect.intervals_count() << " intervals (average " << (float(maybe_incorrect.size()) / maybe_incorrect.intervals_count()) << " elements per interval)" << std::endl;

    if(!check_consistency(std::move(correct), std::move(maybe_incorrect))) {
        return -1;
    }


    return 0;
}