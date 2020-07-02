#pragma once

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "primitive.hpp"

namespace rt {

/**
	A collection of triangles 
*/
class triangle_mesh : public ray_intersectable
{
public:
	/**
		Loads mesh from file using Assimp
	*/
	triangle_mesh(const std::string &path);

	/**
		Provides a brute-force method of finding mesh-ray intersection
	*/
	bool ray_intersect(const ray &r, ray_intersection &hit) const override;

	/**
		Provides read-only access to the triangle data
	*/
	const rt::triangle &get_triangle(unsigned int index) const
	{
		return m_triangles.at(index);
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