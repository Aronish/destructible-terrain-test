#include "octree.hpp"

#include "logger.hpp"

namespace eng
{
	Octree::Octree(glm::vec3 const & position, int unsigned size, int unsigned depth) : m_position(position), m_size(size), m_depth(depth)
	{
		ENG_LOG_F("Created node of size %d at depth %d", m_size, m_depth);
	}

	Octree::~Octree()
	{
		ENG_LOG_F("Deleted node of size %d at depth %d", m_size, m_depth);
		if (m_child_nodes) delete[] m_child_nodes;
	}

	void Octree::addContent(glm::vec3 const & position)
	{
		if (m_depth == MAX_DEPTH)
		{
			m_contents.push_back(position);
			return;
		};
		float half_size = 0.5f * m_size;
		uint8_t child_index = (position.x >= m_position.x + half_size) << 2 | (position.y >= m_position.y + half_size) << 1 | (position.z >= m_position.z + half_size);
		if (!m_child_nodes)
		{
			m_child_nodes = new Octree[8]
			{
				{ glm::vec3{ m_position.x,				m_position.y,				m_position.z,				}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x,				m_position.y,				m_position.z + half_size,	}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x,				m_position.y + half_size,	m_position.z,				}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x,				m_position.y + half_size,	m_position.z + half_size,	}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x + half_size,	m_position.y,				m_position.z,				}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x + half_size,	m_position.y,				m_position.z + half_size,	}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x + half_size,	m_position.y + half_size,	m_position.z,				}, m_size >> 1, m_depth + 1 },
				{ glm::vec3{ m_position.x + half_size,	m_position.y + half_size,	m_position.z + half_size,	}, m_size >> 1, m_depth + 1 }
			};
		}
		m_child_nodes[child_index].addContent(position);
	}

	void Octree::render(std::shared_ptr<Shader> const & shader, CubeFrame const & frame)
	{
		shader->setUniformMatrix4f("u_model", glm::translate(glm::mat4(1.0f), m_position) * glm::scale(glm::mat4(1.0f), glm::vec3{ static_cast<float>(m_size) }));
		glLineWidth(m_size / 2.0f);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
		if (m_child_nodes)
		{
			for (int unsigned i = 0; i < 8; ++i) m_child_nodes[i].render(shader, frame);
		}
		else
		{
			for (auto const & content : m_contents)
			{
				shader->setUniformMatrix4f("u_model", glm::translate(glm::mat4(1.0f), content) * glm::scale(glm::mat4(1.0f), glm::vec3{0.1f}));
				glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
}