#include	"density.hpp"

#include	"octree.hpp"

namespace eng
{
	int const MATERIAL_AIR = 0;
	int const MATERIAL_SOLID = 1;

	float const QEF_ERROR = 1e-6f;
	int const QEF_SWEEPS = 4;
	
	glm::ivec3 const static CHILD_MIN_OFFSETS[] =
	{
		glm::ivec3(0, 0, 0),
		glm::ivec3(0, 0, 1),
		glm::ivec3(0, 1, 0),
		glm::ivec3(0, 1, 1),
		glm::ivec3(1, 0, 0),
		glm::ivec3(1, 0, 1),
		glm::ivec3(1, 1, 0),
		glm::ivec3(1, 1, 1),
	};

	int const static edge_map[12][2] =
	{
		{ 0, 4 }, { 1, 5 },	{ 2, 6 }, { 3, 7 },	// x-axis 
		{ 0, 2 }, { 1, 3 },	{ 4, 6 }, { 5, 7 },	// y-axis
		{ 0, 1 }, { 2, 3 },	{ 4, 5 }, { 6, 7 }	// z-axis
	};

	int const static cell_proc_face_mask[12][3] = {{ 0,4,0 },{ 1,5,0 },{ 2,6,0 },{ 3,7,0 },{ 0,2,1 },{ 4,6,1 },{ 1,3,1 },{ 5,7,1 },{ 0,1,2 },{ 2,3,2 },{ 4,5,2 },{ 6,7,2 }};
	int const static cell_proc_edge_mask[6][5] = {{ 0,1,2,3,0 },{ 4,5,6,7,0 },{ 0,4,1,5,1 },{ 2,6,3,7,1 },{ 0,2,4,6,2 },{ 1,3,5,7,2 }};

	int const static face_proc_face_mask[3][4][3] =
	{
		{{ 4, 0, 0 }, { 5, 1, 0 }, { 6, 2, 0}, { 7, 3, 0}},
		{{ 2, 0, 1 }, { 6, 4, 1 }, { 3, 1, 1}, { 7, 5, 1}},
		{{ 1, 0, 2 }, { 3, 2, 2 }, { 5, 4, 2}, { 7, 6, 2}}
	};

	int const static face_proc_edge_mask[3][4][6] =
	{
		{{ 1, 4, 0, 5, 1, 1 } , { 1, 6, 2, 7, 3, 1 } , { 0, 4, 6, 0, 2, 2 } , { 0, 5, 7, 1, 3, 2 }}, 
		{{ 0, 2, 3, 0, 1, 0 } , { 0, 6, 7, 4, 5, 0 } , { 1, 2, 0, 6, 4, 2 } , { 1, 3, 1, 7, 5, 2 }}, 
		{{ 1, 1, 0, 3, 2, 0 } , { 1, 5, 4, 7, 6, 0 } , { 0, 1, 5, 0, 4, 1 } , { 0, 3, 7, 2, 6, 1 }} 
	} ;

	int const static edge_proc_edge_mask[3][2][5] =
	{
		{{ 3, 2, 1, 0, 0 }, { 7, 6, 5, 4, 0 }},
		{{ 5, 1, 4, 0, 1 }, { 7, 3, 6, 2, 1 }},
		{{ 6, 4, 2, 0, 2 }, { 7, 5, 3, 1, 2 }},
	};

	int const static process_edge_mask[3][4] = {{ 3, 2, 1, 0 }, { 7, 5, 6, 4 }, { 11, 10, 9, 8 }};

	static OctreeNode * simplifyOctree(OctreeNode * node, float threshold)
	{
		if (!node) return nullptr;
		if (node->type != OctreeNodeType::INTERNAL) return node; // can't simplify

		svd::QefSolver qef;
		int signs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
		int mid_sign = -1;
		int edge_count = 0;
		bool is_collapsible = true;

		for (int i = 0; i < 8; i++)
		{
			node->children[i] = simplifyOctree(node->children[i], threshold);
			if (node->children[i])
			{
				OctreeNode * child = node->children[i];
				if (child->type == OctreeNodeType::INTERNAL) is_collapsible = false;
				else
				{
					qef.add(child->draw_info->qef);
					mid_sign = (child->draw_info->corners >> (7 - i)) & 1;
					signs[i] = (child->draw_info->corners >> i) & 1;
					edge_count++;
				}
			}
		}
		if (!is_collapsible) return node; // at least one child is an internal node, can't collapse

		svd::Vec3 qef_position;
		qef.solve(qef_position, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);
		float error = qef.getError();

		glm::vec3 position(qef_position.x, qef_position.y, qef_position.z);

		// at this point the masspoint will actually be a sum, so divide to make it the average
		if (error > threshold) return node;// this collapse breaches the threshold

		if (position.x < node->min.x || position.x >(node->min.x + node->size) ||
			position.y < node->min.y || position.y >(node->min.y + node->size) ||
			position.z < node->min.z || position.z >(node->min.z + node->size)
		)
		{
			const auto & mp = qef.getMassPoint();
			position = glm::vec3(mp.x, mp.y, mp.z);
		}

		// change the node from an internal node to a 'pseudo leaf' node
		OctreeDrawInfo * drawInfo = new OctreeDrawInfo;

		for (int i = 0; i < 8; i++)
		{
			if (signs[i] == -1)
			{
				// undetermined, use centre sign instead
				drawInfo->corners |= (mid_sign << i);
			}
			else
			{
				drawInfo->corners |= (signs[i] << i);
			}
		}
		drawInfo->average_normal = glm::vec3(0.f);
		for (int i = 0; i < 8; i++)
		{
			if (node->children[i])
			{
				OctreeNode * child = node->children[i];
				if (child->type == OctreeNodeType::PSEUDO || child->type == OctreeNodeType::LEAF)
				{
					drawInfo->average_normal += child->draw_info->average_normal;
				}
			}
		}
		drawInfo->average_normal = glm::normalize(drawInfo->average_normal);
		drawInfo->position = position;
		drawInfo->qef = qef.getData();
		for (int i = 0; i < 8; i++)
		{
			OctreeNode::destroyOctree(node->children[i]);
			node->children[i] = nullptr;
		}
		node->type = OctreeNodeType::PSEUDO;
		node->draw_info = drawInfo;
		return node;
	}

	static void generateVertexIndices(OctreeNode * node, Mesh::VertexBuffer & vertex_buffer)
	{
		if (!node) return;
		if (node->type != OctreeNodeType::LEAF)
		{
			for (int i = 0; i < 8; i++)
			{
				generateVertexIndices(node->children[i], vertex_buffer);
			}
		}
		if (node->type != OctreeNodeType::INTERNAL)
		{
			OctreeDrawInfo * d = node->draw_info;
			if (!d)
			{
				ENG_LOG("Couldn't add vertex!");
				return;
			}
			d->index = static_cast<int>(vertex_buffer.size());
			vertex_buffer.push_back(MeshVertex(d->position, d->average_normal));
		}
	}

	static void contourProcessEdge(OctreeNode * node[4], int dir, std::vector<int> & index_buffer)
	{
		int min_size = 1000000; // arbitrary big number
		int min_index = 0;
		int indices[4] = { -1, -1, -1, -1 };
		bool flip = false;
		bool sign_change[4] = { false, false, false, false };

		for (int i = 0; i < 4; i++)
		{
			int const edge = process_edge_mask[dir][i];
			int const c1 = edge_map[edge][0];
			int const c2 = edge_map[edge][1];

			int const m1 = (node[i]->draw_info->corners >> c1) & 1;
			int const m2 = (node[i]->draw_info->corners >> c2) & 1;

			if (node[i]->size < min_size)
			{
				min_size = node[i]->size;
				min_index = i;
				flip = m1 != MATERIAL_AIR;
			}

			indices[i] = node[i]->draw_info->index;

			sign_change[i] = (m1 == MATERIAL_AIR && m2 != MATERIAL_AIR) || (m1 != MATERIAL_AIR && m2 == MATERIAL_AIR);
		}
		if (sign_change[min_index])
		{
			if (!flip)
			{
				index_buffer.push_back(indices[0]);
				index_buffer.push_back(indices[1]);
				index_buffer.push_back(indices[3]);

				index_buffer.push_back(indices[0]);
				index_buffer.push_back(indices[3]);
				index_buffer.push_back(indices[2]);
			}
			else
			{
				index_buffer.push_back(indices[0]);
				index_buffer.push_back(indices[3]);
				index_buffer.push_back(indices[1]);

				index_buffer.push_back(indices[0]);
				index_buffer.push_back(indices[2]);
				index_buffer.push_back(indices[3]);
			}
		}
	}

	static void contourEdgeProc(OctreeNode * node[4], int const dir, std::vector<int> & index_buffer)
	{
		if (!node[0] || !node[1] || !node[2] || !node[3]) return;

		if (node[0]->type != OctreeNodeType::INTERNAL &&
			node[1]->type != OctreeNodeType::INTERNAL &&
			node[2]->type != OctreeNodeType::INTERNAL &&
			node[3]->type != OctreeNodeType::INTERNAL
		)
		{
			contourProcessEdge(node, dir, index_buffer);
		}
		else
		{
			for (int i = 0; i < 2; i++)
			{
				OctreeNode * edge_nodes[4] = {};
				int const c[4] =
				{
					edge_proc_edge_mask[dir][i][0],
					edge_proc_edge_mask[dir][i][1],
					edge_proc_edge_mask[dir][i][2],
					edge_proc_edge_mask[dir][i][3],
				};

				for (int j = 0; j < 4; j++)
				{
					if (node[j]->type == OctreeNodeType::LEAF || node[j]->type == OctreeNodeType::PSEUDO)
					{
						edge_nodes[j] = node[j];
					}
					else
					{
						edge_nodes[j] = node[j]->children[c[j]];
					}
				}
				contourEdgeProc(edge_nodes, edge_proc_edge_mask[dir][i][4], index_buffer);
			}
		}
	}
	
	static void contourFaceProc(OctreeNode * node[2], int const dir, std::vector<int> & index_buffer)
	{
		if (!node[0] || !node[1]) return;

		if (node[0]->type == OctreeNodeType::INTERNAL || node[1]->type == OctreeNodeType::INTERNAL)
		{
			for (int i = 0; i < 4; i++)
			{
				OctreeNode * face_nodes[2] = {};
				int const c[2] =
				{
					face_proc_face_mask[dir][i][0],
					face_proc_face_mask[dir][i][1],
				};

				for (int j = 0; j < 2; j++)
				{
					if (node[j]->type != OctreeNodeType::INTERNAL)
					{
						face_nodes[j] = node[j];
					}
					else
					{
						face_nodes[j] = node[j]->children[c[j]];
					}
				}
				contourFaceProc(face_nodes, face_proc_face_mask[dir][i][2], index_buffer);
			}

			int const orders[2][4] =
			{
				{ 0, 0, 1, 1 },
				{ 0, 1, 0, 1 },
			};
			for (int i = 0; i < 4; i++)
			{
				OctreeNode * edge_nodes[4] = {};
				int const c[4] =
				{
					face_proc_edge_mask[dir][i][1],
					face_proc_edge_mask[dir][i][2],
					face_proc_edge_mask[dir][i][3],
					face_proc_edge_mask[dir][i][4],
				};

				int const * order = orders[face_proc_edge_mask[dir][i][0]];
				for (int j = 0; j < 4; j++)
				{
					if (node[order[j]]->type == OctreeNodeType::LEAF || node[order[j]]->type == OctreeNodeType::PSEUDO)
					{
						edge_nodes[j] = node[order[j]];
					}
					else
					{
						edge_nodes[j] = node[order[j]]->children[c[j]];
					}
				}
				contourEdgeProc(edge_nodes, face_proc_edge_mask[dir][i][5], index_buffer);
			}
		}
	}

	static void contourCellProc(OctreeNode * node, std::vector<int> & index_buffer
	)
	{
		if (node == NULL) return;
		if (node->type == OctreeNodeType::INTERNAL)
		{
			for (int i = 0; i < 8; i++)
			{
				contourCellProc(node->children[i], index_buffer);
			}

			for (int i = 0; i < 12; i++)
			{
				OctreeNode * face_nodes[2] = {};
				int const c[2] = { cell_proc_face_mask[i][0], cell_proc_face_mask[i][1] };

				face_nodes[0] = node->children[c[0]];
				face_nodes[1] = node->children[c[1]];

				contourFaceProc(face_nodes, cell_proc_face_mask[i][2], index_buffer);
			}

			for (int i = 0; i < 6; i++)
			{
				OctreeNode * edgeNodes[4] = {};
				int const c[4] =
				{
					cell_proc_edge_mask[i][0],
					cell_proc_edge_mask[i][1],
					cell_proc_edge_mask[i][2],
					cell_proc_edge_mask[i][3],
				};

				for (int j = 0; j < 4; j++)
				{
					edgeNodes[j] = node->children[c[j]];
				}

				contourEdgeProc(edgeNodes, cell_proc_edge_mask[i][4], index_buffer);
			}
		}
	}

	glm::vec3 approximateZeroCrossingPosition(glm::vec3 const & p0, glm::vec3 const & p1)
	{
		// approximate the zero crossing by finding the min value along the edge
		float minValue = 100000.f;
		float t = 0.f;
		float currentT = 0.f;
		int const steps = 8;
		float const increment = 1.f / (float)steps;
		while (currentT <= 1.f)
		{
			glm::vec3 const p = p0 + ((p1 - p0) * currentT);
			float const density = glm::abs(getDensity(p));
			if (density < minValue)
			{
				minValue = density;
				t = currentT;
			}

			currentT += increment;
		}

		return p0 + ((p1 - p0) * t);
	}

	glm::vec3 calculateSurfaceNormal(glm::vec3 const & p)
	{
		float const H = 0.001f;
		float const dx = getDensity(p + glm::vec3(H, 0.f, 0.f)) - getDensity(p - glm::vec3(H, 0.f, 0.f));
		float const dy = getDensity(p + glm::vec3(0.f, H, 0.f)) - getDensity(p - glm::vec3(0.f, H, 0.f));
		float const dz = getDensity(p + glm::vec3(0.f, 0.f, H)) - getDensity(p - glm::vec3(0.f, 0.f, H));

		return glm::normalize(glm::vec3(dx, dy, dz));
	}

	OctreeNode * constructLeaf(OctreeNode * leaf)
	{
		if (!leaf || leaf->size != 1)
		{
			return nullptr;
		}

		int corners = 0;
		for (int i = 0; i < 8; i++)
		{
			glm::vec3 const cornerPos = leaf->min + CHILD_MIN_OFFSETS[i];
			float const density = getDensity(glm::vec3(cornerPos));
			int const material = density < 0.f ? MATERIAL_SOLID : MATERIAL_AIR;
			corners |= (material << i);
		}

		if (corners == 0 || corners == 255)
		{
			// voxel is full inside or outside the volume
			delete leaf;
			return nullptr;
		}

		// otherwise the voxel contains the surface, so find the edge intersections
		int const MAX_CROSSINGS = 6;
		int edgeCount = 0;
		glm::vec3 averageNormal(0.f);
		svd::QefSolver qef;

		for (int i = 0; i < 12 && edgeCount < MAX_CROSSINGS; i++)
		{
			int const c1 = edge_map[i][0];
			int const c2 = edge_map[i][1];

			int const m1 = (corners >> c1) & 1;
			int const m2 = (corners >> c2) & 1;

			if ((m1 == MATERIAL_AIR && m2 == MATERIAL_AIR) || (m1 == MATERIAL_SOLID && m2 == MATERIAL_SOLID))
			{
				// no zero crossing on this edge
				continue;
			}

			glm::vec3 const p1 = glm::vec3(leaf->min + CHILD_MIN_OFFSETS[c1]);
			glm::vec3 const p2 = glm::vec3(leaf->min + CHILD_MIN_OFFSETS[c2]);
			glm::vec3 const p = approximateZeroCrossingPosition(p1, p2);
			glm::vec3 const n = calculateSurfaceNormal(p);
			qef.add(p.x, p.y, p.z, n.x, n.y, n.z);

			averageNormal += n;

			edgeCount++;
		}

		svd::Vec3 qefPosition;
		qef.solve(qefPosition, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);

		OctreeDrawInfo * drawInfo = new OctreeDrawInfo;
		drawInfo->position = glm::vec3(qefPosition.x, qefPosition.y, qefPosition.z);
		drawInfo->qef = qef.getData();

		glm::vec3 const min = glm::vec3(leaf->min);
		glm::vec3 const max = glm::vec3(leaf->min + glm::ivec3(leaf->size));
		if (drawInfo->position.x < min.x || drawInfo->position.x > max.x ||
			drawInfo->position.y < min.y || drawInfo->position.y > max.y ||
			drawInfo->position.z < min.z || drawInfo->position.z > max.z)
		{
			const auto & mp = qef.getMassPoint();
			drawInfo->position = glm::vec3(mp.x, mp.y, mp.z);
		}

		drawInfo->average_normal = glm::normalize(averageNormal / (float)edgeCount);
		drawInfo->corners = corners;

		leaf->type = OctreeNodeType::LEAF;
		leaf->draw_info = drawInfo;

		return leaf;
	}

	OctreeNode * constructOctreeNodes(OctreeNode * node)
	{
		if (!node)
		{
			return nullptr;
		}

		if (node->size == 1)
		{
			return constructLeaf(node);
		}

		int const childSize = node->size / 2;
		bool hasChildren = false;

		for (int i = 0; i < 8; i++)
		{
			OctreeNode * child = new OctreeNode;
			child->size = childSize;
			child->min = node->min + (CHILD_MIN_OFFSETS[i] * childSize);
			child->type = OctreeNodeType::INTERNAL;

			node->children[i] = constructOctreeNodes(child);
			hasChildren |= (node->children[i] != nullptr);
		}

		if (!hasChildren)
		{
			delete node;
			return nullptr;
		}

		return node;
	}

	OctreeNode * OctreeNode::buildOctree(glm::ivec3 const & min, int const size, float const threshold)
	{
		OctreeNode * root = new OctreeNode;
		root->min = min;
		root->size = size;
		root->type = OctreeNodeType::INTERNAL;

		constructOctreeNodes(root);
		root = simplifyOctree(root, threshold);

		return root;
	}

	void OctreeNode::generateMeshFromOctree(OctreeNode * node, Mesh::Mesh::VertexBuffer & vertex_buffer, std::vector<int> & index_buffer)
	{
		if (!node)
		{
			return;
		}

		vertex_buffer.clear();
		index_buffer.clear();

		generateVertexIndices(node, vertex_buffer);
		contourCellProc(node, index_buffer);
	}

	void OctreeNode::destroyOctree(OctreeNode * node)
	{
		if (!node)
		{
			return;
		}

		for (int i = 0; i < 8; i++)
		{
			destroyOctree(node->children[i]);
		}

		if (node->draw_info)
		{
			delete node->draw_info;
		}

		delete node;
	}
}