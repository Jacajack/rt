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
	rt::ray_hit hit, best_hit;
	best_hit.distance = HUGE_VALF;

	for (auto obj_ptr : m_objects)
	{
		// Return closest hit
		if (obj_ptr->cast_ray(r, hit) && hit.distance < best_hit.distance)
			best_hit = hit;
	}	

	// World hit?
	if (best_hit.distance == HUGE_VALF)
	{
		static diffuse_brdf world_brdf{glm::vec3(0.2, 0.2, 0.7)};

		best_hit.position = {HUGE_VALF, HUGE_VALF, HUGE_VALF};
		best_hit.direction = r.direction;
		best_hit.normal = -r.direction;
		best_hit.brdf = &world_brdf;
	}

	return best_hit;
}