#pragma once

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "primitive.hpp"

namespace rt {

/**
	`mesh_data` class stores a collection of triangles read from a model file.
	The triangles do not have any material assigned
*/
class mesh_data
{
	friend struct primitive_collection;

public:
	/**
		Loads mesh from file using Assimp
	*/
	mesh_data(const std::string &path);

	/**
		Provides read-only access to the triangle data
	*/
	const std::vector<rt::triangle> &get_triangles() const
	{
		return m_triangles;
	}

	/**
		Returns triangle count in the mesh
	*/
	unsigned int get_triangle_count() const
	{
		return m_triangles.size();
	} 

private:
	void append_assimp_mesh(aiMesh *mesh);
	void process_assimp_node(const aiScene *scene, aiNode *node);

	std::vector<rt::triangle> m_triangles;
};

}