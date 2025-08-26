#include <engine/scene/renderer/mesh.hpp>

namespace engine {
    mesh::mesh(material m, rc<const gal::vertex_array> vao) {
        m_primitives.emplace_back(std::move(m), std::move(vao));
    }

    mesh::mesh(std::vector<primitive> primitives) : m_primitives(std::move(primitives)) {}

    const std::vector<primitive>& mesh::primitives() const { return m_primitives; }
    std::vector<primitive>& mesh::primitives() { return m_primitives; }
}
