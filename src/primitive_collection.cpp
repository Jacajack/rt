#include "primitive_collection.hpp"

using rt::primitive_collection;

primitive_collection::primitive_collection(const mesh_data &mesh) :
	triangles(mesh.get_triangles())
{
}

primitive_collection::primitive_collection(mesh_data &&mesh) :
	triangles(std::move(mesh.m_triangles))
{
}

primitive_collection::primitive_collection(const triangle &t) :
	triangles(1, t)
{
}

primitive_collection::primitive_collection(const sphere &s) :
	spheres(1, s)
{
}

primitive_collection::primitive_collection(const plane &p) :
	planes(1, p)
{
}

void primitive_collection::apply_transform(const glm::mat4 &mat)
{
	for (auto &p : triangles)
		p = p.transform(mat);

	for (auto &p : spheres)
		p = p.transform(mat);

	for (auto &p : planes)
		p = p.transform(mat);
}

void primitive_collection::assign_material(const abstract_material *material)
{
	for (auto &p : triangles)
		if (!p.material) p.material = material;

	for (auto &p : spheres)
		if (!p.material) p.material = material;
		
	for (auto &p : planes)
		if (!p.material) p.material = material;
}

void primitive_collection::set_material(const abstract_material *material)
{
	for (auto &p : triangles)
		p.material = material;

	for (auto &p : spheres)
		p.material = material;
		
	for (auto &p : planes)
		p.material = material;
}