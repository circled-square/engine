#include <engine/resources_manager.hpp>
#include <engine/application/window.hpp>
#include <GAL/init.hpp>

//fool resources_manager into thinking we are their friend (class)
namespace engine {
    struct application {
        application() { resources_manager::init_instance(); }
        ~application() { resources_manager::deinit_instance(); }
        std::size_t get_number_of_textures() {
            return std::get<detail::id_to_resource_hashmap<gal::texture>>(get_rm().m_active_resources).size();
        }
    };
}

int main() {
    using namespace engine;

    //initialization
    window::window window({400,400}, "test_window"); // unfortunately opengl sucks and it can't seize resources without a valid context, that is to say a window
    gal::initialize_opengl();
    application fakeapp;

    gal::texture::specification spec { .res = {1,1} };
    rc<gal::texture> tex_ref = get_rm().new_mut_from(gal::texture(spec));
    // check pointer correctness
    if(tex_ref->resolution() != spec.res)
        throw std::runtime_error("incorrect pointer returned by resources_manager::get_mut_from");
    //check that the texture is managed by rm
    if(fakeapp.get_number_of_textures() != 1)
        throw std::runtime_error("the texture doesn't seem to be managed by rm");

    weak<gal::texture> tex_weakref = tex_ref;
    if(!tex_weakref.lock())
        throw std::runtime_error("could not lock valid weak<T>");

    // force object deletion but no deallocation
    tex_ref = nullptr;
    get_rm().collect_garbage();

    if(tex_weakref.lock())
        throw std::runtime_error("succeeded in locking invalid weak<T>");

    // force deallocation
    get_rm().collect_garbage();

    // todo: make a local test class (not application) be the friend that does all this dirty stuff
    if(fakeapp.get_number_of_textures() != 0)
        throw std::runtime_error("resource was not deallocated");


    return 0;
}
