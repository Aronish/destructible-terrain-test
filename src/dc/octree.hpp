#pragma once

#include <glm/glm.hpp>

#include "qef.hpp"
#include "mesh.hpp"

namespace eng
{
	enum class OctreeNodeType
	{
		NONE,
		INTERNAL,
		PSEUDO,
		LEAF
	};

	struct OctreeDrawInfo
	{
		int index{ -1 };
		int	corners{ 0 };
		glm::vec3 position{};
		glm::vec3 average_normal{};
		svd::QefData qef{};
	};

	class OctreeNode
	{
	public:
		OctreeNodeType type{ OctreeNodeType::NONE };
		glm::ivec3 min{};
		int size{};
		OctreeNode * children[8]{};
		OctreeDrawInfo * draw_info{};

		OctreeNode()
		{
			for (auto & i : children) i = nullptr;
		}

		explicit OctreeNode(const OctreeNodeType type) : type(type)
		{
			for (auto & i : children) i = nullptr;
		}

		static OctreeNode * buildOctree(const glm::ivec3 & min, const int size, const float threshold);
		static void destroyOctree(OctreeNode * node);
		static void generateMeshFromOctree(OctreeNode * node, Mesh::VertexBuffer & vertex_buffer, std::vector<int> & index_buffer);
	};

}