#include "primitive_soup.hpp"

#include <stdexcept>

#include "triangle_mesh.hpp"

using rt::primitive_soup;

primitive_soup::primitive_soup(const rt::scene &sc)
{
	// Traverse scene objects and try to add them to the soup
	const std::vector<scene_object*> &objects = sc.m_objects;
	for (auto obj_ptr : objects)
	{
		// Get geometry pointer
		const rt::ray_intersectable *geom_ptr = &obj_ptr->get_geometry();

		// Is the object a plane?
		auto plane_ptr = dynamic_cast<const rt::plane*>(geom_ptr);
		if (plane_ptr)
		{
			planes.emplace_back(*plane_ptr, obj_ptr);
			continue;
		}

		// Is the object a sphere?
		auto sphere_ptr = dynamic_cast<const rt::sphere*>(geom_ptr);
		if (sphere_ptr)
		{
			spheres.emplace_back(*sphere_ptr, obj_ptr);
			continue;
		}

		// Is the object a mesh?
		auto mesh_ptr = dynamic_cast<const rt::triangle_mesh*>(geom_ptr);
		if (mesh_ptr)
		{
			for (const auto &t : mesh_ptr->get_triangles())
				triangles.emplace_back(t, obj_ptr);
			continue;
		}

		// Is the object a triangle?
		auto triangle_ptr = dynamic_cast<const rt::triangle*>(geom_ptr);
		if (triangle_ptr)
		{
			triangles.emplace_back(*triangle_ptr, obj_ptr);
			continue;
		}

		// Cannot add the object to our soup
		throw std::runtime_error("primitive_soup cannot handle provided primitive type!");
	}
}