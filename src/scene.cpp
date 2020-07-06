#include "scene.hpp"

using rt::scene;

void scene::add_object(scene_object *obj)
{
	m_objects.push_back(obj);
}


rt::ray_hit scene::cast_ray(const rt::ray &r, const rt::ray_accelerator &accel) const
{
	// Nearest intersection
	rt::ray_hit hit;

	if (accel.cast_ray(r, hit))
	{
		return hit;
	}
	else
	{
		rt::ray_hit world_hit;
		world_hit.distance = HUGE_VALF;
		world_hit.position = {HUGE_VALF, HUGE_VALF, HUGE_VALF};
		world_hit.direction = r.direction;
		world_hit.normal = -r.direction;
		// world_hit.geometry = nullptr;
		world_hit.material = m_world_material.get();
		return world_hit;
	}
}