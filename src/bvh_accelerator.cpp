#include "bvh_accelerator.hpp"

#include <algorithm>
#include <stdexcept>

using rt::bvh_node;
using rt::bvh_accelerator;

bvh_node::bvh_node(rt::primitive_soup &&soup) :
	m_triangles(std::move(soup.triangles))
{
	calculate_volume();
}

bvh_node::bvh_node(const rt::primitive_soup &soup) :
	m_triangles(soup.triangles)
{
	calculate_volume();
}

/**
	Calulates AAB for BVH node based on childrens' and triangles AABBs
*/
void bvh_node::calculate_volume()
{
	aabb vol({0, 0, 0}, {0, 0, 0});
	
	for (const auto &c : m_children)
		vol = aabb{vol, c.m_volume};
	
	for (const auto &t : m_triangles)
		vol = aabb{vol, t.get_aabb()};

	m_volume = vol;
}

void bvh_node::subdivide()
{
	if (m_triangles.size() == 0)
		throw std::runtime_error("cannot subdivide BVH node with 0 primitives");

	glm::vec3 volsize{m_volume.get_size()};
	glm::vec3 volcenter{m_volume.get_center()};

	// Decide split direction
	float glm::vec3::* axis;
	if (volsize.x > volsize.y && volsize.x > volsize.z)
		axis = &glm::vec3::x;
	else if (volsize.y > volsize.x && volsize.y > volsize.z)
		axis = &glm::vec3::y;
	else
		axis = &glm::vec3::z;

	// Sort the triangles based on the selected component
	// \todo Do not calculate AABBs over and over!!! Store triangles along with their centroids!
	std::sort(m_triangles.begin(), m_triangles.end(), [&](auto &t1, auto &t2){
		glm::vec3 c1 = (t1.vertices[0] + t1.vertices[1] + t1.vertices[2]) / 3.f;
		glm::vec3 c2 = (t2.vertices[0] + t2.vertices[1] + t2.vertices[2]) / 3.f;
		return c1.*axis < c2.*axis;
	});

	// Attempt midpoint split
	auto split = std::lower_bound(m_triangles.begin(), m_triangles.end(), volcenter.*axis, [&](auto &t1, auto c){
		return ((t1.vertices[0] + t1.vertices[1] + t1.vertices[2]) / 3.f).*axis < c;
	});

	// If any child is empty, attempt median split
	if (split == m_triangles.end() || split == m_triangles.begin())
		split = m_triangles.begin() + m_triangles.size() / 2;
	
	// Create children and remove triangles from this node
	m_children.emplace_back(m_triangles.begin(), split);
	m_children.emplace_back(split, m_triangles.end());
	m_triangles.clear();
	m_triangles.shrink_to_fit();
}

void bvh_node::recursively_subdivide(unsigned int primitive_count)
{
	if (m_triangles.size() <= primitive_count)
		return;

	subdivide();
	for (auto &c : m_children)
		c.recursively_subdivide(primitive_count);
}

bool bvh_node::cast_ray(const rt::ray &r, rt::ray_hit &best_hit) const
{
	if (m_volume.check_ray_intersect(r))
	{
		rt::ray_hit h;
		best_hit.distance = HUGE_VALF;

		// Check children 
		for (const auto &c : m_children)
			if (c.cast_ray(r, h))
				best_hit = std::min(best_hit, h);

		// Check primitives
		for (const auto &t : m_triangles)
			if (t.cast_ray(r, h))
				best_hit = std::min(best_hit, h);

		return best_hit.distance != HUGE_VALF;
	}
	else
	{
		return false;
	}
}

bvh_accelerator::bvh_accelerator(rt::primitive_soup &&soup) :
	m_root_node(std::move(soup)),
	m_spheres(std::move(soup.spheres)),
	m_planes(std::move(soup.planes))
{
	build_tree();
}

bvh_accelerator::bvh_accelerator(const rt::primitive_soup &soup) :
	m_root_node(soup),
	m_spheres(soup.spheres),
	m_planes(soup.planes)
{
	build_tree();
}

bool bvh_accelerator::cast_ray(const rt::ray &r, rt::ray_hit &best_hit) const
{
	rt::ray_hit h;
	best_hit.distance = HUGE_VALF;

	// Check spheres and planes
	for (const auto &s : m_spheres)
		if (s.cast_ray(r, h))
			best_hit = std::min(best_hit, h);

	for (const auto &p : m_planes)
		if (p.cast_ray(r, h))
			best_hit = std::min(best_hit, h);

	// Traverse BVH
	if (m_root_node.cast_ray(r, h))
		best_hit = std::min(best_hit, h);

	return best_hit.distance != HUGE_VALF;
}


void bvh_accelerator::build_tree()
{
	m_root_node.recursively_subdivide(4);
}