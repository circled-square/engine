#include <engine/utils/interval_set.hpp>
#include <engine/entity_component_system.hpp> // just to import ecs_id_t so we can test on it specifically
#include <set>
#include <random>

using engine::ecs_id_t;
using engine::interval_set;

bool check_consistency(const std::set<ecs_id_t>& correct, interval_set<ecs_id_t> maybe_incorrect) {
    if(correct.size() != maybe_incorrect.size()) {
        return false;
    }

    while(!maybe_incorrect.empty()) {
        ecs_id_t el = maybe_incorrect.extract_first_element();

        if(!correct.contains(el)) {
            return false;
        }
    }

    return true;
}

template<class T, size_t N = T::state_size>
T make_seeded() {
    // typename T::result_type random_data[N];
    // std::random_device source;
    // std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    // std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    std::seed_seq seeds({ 0, 1, 2, 3 }); // using some predefined numbers instead of std::random_device because we want the testing to be deterministic
    T seededEngine(seeds);
    return seededEngine;
}

constexpr ecs_id_t max_id = 0xff'ff'ff;
constexpr std::uint64_t repetitions = max_id/2; // make sure the data is somewhat sparse

int main() {
    interval_set<ecs_id_t> maybe_incorrect;
    std::set<ecs_id_t> correct;

    if(!check_consistency(correct, maybe_incorrect)) {
        return -1;
    }

    auto rng = make_seeded<std::mt19937_64>();
    std::uniform_int_distribution<ecs_id_t> id_distr(0, max_id);
    std::uniform_int_distribution<char> bool_distr(0, 2); // biased to true

    for(int i = 0; i < repetitions; i++) {
        if(bool_distr(rng) != 0) {
            //insert
            ecs_id_t id = id_distr(rng);
            maybe_incorrect.insert(id);
            correct.insert(id);
        } else {
            if(!correct.empty()) {
                ecs_id_t maybe_incorrect_id = maybe_incorrect.extract_first_element();
                ecs_id_t correct_id = *correct.begin();
                correct.erase(correct.begin());

                if(maybe_incorrect_id != correct_id) {
                    return -1;
                }
            }
        }
    }

    if(!check_consistency(correct, std::move(maybe_incorrect))) {
        return -1;
    }


    return 0;
}
