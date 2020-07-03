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
	Calulates AAB for BVH node based on childrens' and triangles' AABBs
*/
void bvh_node::calculate_volume()
{
	aabb vol({0, 0, 0}, {0, 0, 0});
	
	if (m_left) vol = aabb{vol, m_left->get_aabb()};
	if (m_right) vol = aabb{vol, m_right->get_aabb()};
	
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
	m_left = std::make_unique<bvh_node>(m_triangles.begin(), split);
	m_right = std::make_unique<bvh_node>(split, m_triangles.end());
	m_triangles.clear();
	m_triangles.shrink_to_fit();

	// Check if the children overlap
	m_children_overlap = m_left->get_aabb().check_aabb_overlap(m_right->get_aabb());
}

void bvh_node::recursively_subdivide(unsigned int primitive_count)
{
	if (m_triangles.size() <= primitive_count)
		return;

	subdivide();
	m_left->recursively_subdivide(primitive_count);
	m_right->recursively_subdivide(primitive_count);
}

/**
	Checks the ray agains everything inside the node.
	\warning This node's AABB is not tested against the ray
*/
bool bvh_node::cast_ray(const rt::ray &r, rt::ray_hit &best_hit) const
{
	rt::ray_hit h;
	best_hit.distance = HUGE_VALF;

	// If node has triangles, it's a leaf node and hence
	// children don't need to be traversed
	if (m_triangles.empty())
	{
		// Distances to child AABBs intersections
		float t1, t2;
		t1 = m_left->get_aabb().ray_intersection_distance(r);
		t2 = m_right->get_aabb().ray_intersection_distance(r);

		if (t1 == HUGE_VALF && t2 == HUGE_VALF) // Both missed
			return false; 
		else if (t1 == HUGE_VALF || t2 == HUGE_VALF) // One missed
		{
			// Check the closer child
			if (t1 != HUGE_VALF) m_left->cast_ray(r, h);
			else if (t2 != HUGE_VALF) m_right->cast_ray(r, h);
			best_hit = std::min(best_hit, h);
		}
		else // Both hit
		{
			// Check both children if they overlap or the closer one otherwise
			if (m_children_overlap)
			{
				// But still check the closer child first
				if (t1 < t2)
					m_left->cast_ray(r, h);
				else
					m_right->cast_ray(r, h);

				best_hit = std::min(best_hit, h);

				// Now, if the hit is closer than the second bounding volume 
				// we can skip it
				if (best_hit.distance < std::max(t1, t2))
					return true;
				
				// Check the farther child
				if (t1 < t2)
					m_right->cast_ray(r, h);
				else
					m_left->cast_ray(r, h);

				best_hit = std::min(best_hit, h);
			}
			else
			{
				// Only check closer child
				if (t1 < t2)
					m_left->cast_ray(r, h);
				else
					m_right->cast_ray(r, h);

				best_hit = std::min(best_hit, h);
			}
		}
	}
	else
	{
		for (const auto &t : m_triangles)
			if (t.cast_ray(r, h))
				best_hit = std::min(best_hit, h);
	}

	return best_hit.distance != HUGE_VALF;
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
	if (m_root_node.get_aabb().check_ray_intersect(r))
		if (m_root_node.cast_ray(r, h))
			best_hit = std::min(best_hit, h);

	return best_hit.distance != HUGE_VALF;
}


void bvh_accelerator::build_tree()
{
	m_root_node.recursively_subdivide(32);
}