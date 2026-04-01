#include "engine/resources_manager/detail/typedefs.hpp"

#include <slogga/log.hpp>
#include <slogga/asserts.hpp>
#include <engine/scene.hpp>
#include <sstream>
#include <dylib.hpp>

#include <engine/resources_manager.hpp>

namespace engine {
    using namespace detail;

    resources_manager::~resources_manager() {
        set_default_3d_shader(std::nullopt);
        collect_garbage();
    }

    void resources_manager::dbg_add_scene_constructor(std::string name, std::function<scene ()> scene_constructor) {
        using namespace detail;
        m_dbg_scene_ctors.insert({name, std::move(scene_constructor)});
    }

    rc<const shader> resources_manager::get_default_3d_shader() { return m_default_3d_shader.value_or(load<shader>(internal_resource_name_t::simple_3d_shader)); }
    void resources_manager::set_default_3d_shader(std::optional<rc<const shader>> s) { m_default_3d_shader = std::move(s); }

    void resources_manager::collect_garbage() {
        slogga::stdout_log("called resources_manager::collect_garbage");

        bool deleted_any;

        std::ostringstream log_ss;

        auto delete_all_marked = [&]<class T> (id_hashset<T>& marked_for_deletion_set) {

            std::size_t previous_size = m_hashmaps.id_to_resource<T>().size();

            while(!marked_for_deletion_set.empty()) {
                auto mfds_iterator = marked_for_deletion_set.cbegin();
                auto itrh_iterator = m_hashmaps.id_to_resource<T>().find(*mfds_iterator);
                EXPECTS(itrh_iterator != m_hashmaps.id_to_resource<T>().end());
                EXPECTS(mfds_iterator != marked_for_deletion_set.cend());

                //IMPORTANT: have care to call erase(mfds_it) BEFORE ~rc_resource() (itrh.erase, rc_resource.resource().reset()), since doing that may cause a rc_resource to reach refcount 0 and be marked for deletion; this in turn will invalidate the mfds_iterator and that is BAD (program hangs forever when trying to erase)
                marked_for_deletion_set.erase(mfds_iterator);

                rc_resource<T>& rc_ref = *std::get<rc_ptr<T>>(itrh_iterator->second);

                //refcount can be >0 if the obj was flagged and a weak<T> managed to lock() it before it was deleted, or if a rc<T> to it was obtained through its name
                if(rc_ref.refcount() == 0) {
                    if(rc_ref.weak_refcount() == 0) {
                        //there are neither rc<T> nor weak<T> pointing to this resource
                        //if present remove name-id correspondence
                        resource_name_t name = std::get<resource_name_t>(itrh_iterator->second);

                        if(!std::holds_alternative<std::monostate>(name)) {
                            EXPECTS(m_hashmaps.name_to_id<T>().contains(name));
                            m_hashmaps.name_to_id<T>().erase(name);
                        }


                        //delete resource
                        m_hashmaps.id_to_resource<T>().erase(itrh_iterator);
                    } else {
                        //there are still weak refs to this resource; destroy the contained value
                        rc_ref.resource().reset();
                    }

                    deleted_any = true;
                }
            }

            std::size_t cleared = previous_size - m_hashmaps.id_to_resource<T>().size();
            if (cleared != 0 && slogga::stdout_log.would_print(slogga::log_level::TRACE)) {
                log_ss << cleared << '/' << previous_size << " " << get_resource_typename<T>() << "s "; // yes, i know, adding an 's' at the end is not really the grammatically correct way to pluralize a noun...
            }
        };


        //actually only scenes can depend on each other, and no resource can depend on one that would get deleted before it. still leaving this behaviour for testing (and because who knows, maybe in the future this won't be the case)
        do {
            deleted_any = false;

            std::apply([&](auto&... args) {
                ((delete_all_marked(args)), ...);
            }, m_hashmaps.all_marked_for_deletion_sets());

            if(deleted_any) {
                slogga::stdout_log.trace("gc pass cleared {}", log_ss.str());
            }
        } while(deleted_any == true);
    }



    //resources manager singleton
    /* a static alloc in which a rm is in-place constructed was chosen instead of other possibilities because:
     * - a rm instance in static memory is constructed & destructed at unspecified moments in program startup/termination, which
     *   is definitely undesirable if we wish to make correct use of constructor and destructor;
     * - a Meyers singleton was avoided because lazily initing a rm seems like a bad idea, and because the destruction still
     *   happens at an unspecified time;
     * - a pointer to the heap requires a heap alloc/dealloc, which is a little weird, but not terrible;
     * - a optional<rm> destructs the object upon program shutdown, which will cause a crash if no opengl context is available;
     *
     * optional<rm> might be preferable if we wish to make deinit of rm mandatory, but as it currently stands deinit of rm is
     * only useful for debug and useless overhead otherwise, since rm only owns gpu resources and memory but nothing that needs
     * to be actually closed or saved before program termination.
     */

    alignas(resources_manager) static char global_rm_instance[sizeof(resources_manager)];
    static bool global_rm_instance_is_inited = false;

    resources_manager &resources_manager::get_instance() {
        EXPECTS(global_rm_instance_is_inited);
        return *reinterpret_cast<resources_manager*>(&global_rm_instance);
    }

    void resources_manager::init_instance() {
        EXPECTS(!global_rm_instance_is_inited);
        resources_manager* rm_p = reinterpret_cast<resources_manager*>(&global_rm_instance);
        new(rm_p) resources_manager();
        global_rm_instance_is_inited = true;
    }

    void resources_manager::deinit_instance() {
        get_instance().~resources_manager();
        global_rm_instance_is_inited = false;
    }

    resources_manager& get_rm() {
        return resources_manager::get_instance();
    }
}

