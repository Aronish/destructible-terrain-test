#include <glad/glad.h>

#include "graphics/vertex_array.hpp"
#include "graphics/vertex_data_layout.hpp"

#include "mesh.hpp"

namespace eng
{
	MeshVertex::MeshVertex(glm::vec3 const & position, glm::vec3 const & normal) : position(position), normal(normal)
	{
	}

	Mesh::Mesh(GameSystem & game_system) :
		va_(game_system.getAssetManager().createVertexArray()),
		vb_(game_system.getAssetManager().createBuffer()),
		ib_(game_system.getAssetManager().createBuffer())
	{
		VertexArray::associateVertexBuffer(va_, vb_, VertexDataLayout{{{3, GL_FLOAT}, {3, GL_FLOAT}}});
		VertexArray::associateIndexBuffer(va_, ib_);
	}

	void Mesh::refreshMesh(VertexBuffer const & vertices, std::vector<int> const & indices)
	{
		if (vertices.empty() || indices.empty()) return;

		glBindVertexArray(va_);

		glBindBuffer(GL_ARRAY_BUFFER, vb_);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
		index_count_ = static_cast<int>(indices.size());

		ENG_LOG_F("Mesh: %lld vertices %lld triangles\n", vertices.size(), indices.size() / 3);

		glBindVertexArray(0);
	}
}
