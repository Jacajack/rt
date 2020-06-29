#include "scene.hpp"

using rt::scene;

void scene::add_object(scene_object *obj)
{
	m_objects.push_back(obj);
}

/**
	Intersect ray with objects in the scene.
	\todo Fix this brute-force solution and replace it with some nice acceleration structure
*/
rt::ray_hit scene::cast_ray(const rt::ray &r) const
{
	// Nearest intersection
	rt::ray_hit hit, best_hit;
	best_hit.distance = HUGE_VALF;
	best_hit.object = nullptr;
	
	for (auto obj_ptr : m_objects)
	{
		const auto &geometry = obj_ptr->get_geometry();
		if (geometry.ray_intersect(r, hit) && hit < best_hit)
		{
			best_hit = hit;
			best_hit.object = obj_ptr;
		}
	}	

	// World hit
	if (best_hit.object == nullptr)
	{
		rt::ray_hit world_hit;
		world_hit.distance = HUGE_VALF;
		world_hit.position = {HUGE_VALF, HUGE_VALF, HUGE_VALF};
		world_hit.direction = r.direction;
		world_hit.normal = -r.direction;
		world_hit.geometry = nullptr;
		world_hit.mat = m_world_material.get();
		world_hit.object = nullptr;
		return world_hit;
	}
	
	// Copy material and geometry pointers from object data
	best_hit.mat = &best_hit.object->get_material();
	best_hit.geometry = &best_hit.object->get_geometry();

	return best_hit;
}