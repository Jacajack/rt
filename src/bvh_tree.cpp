#include "bvh_tree.hpp"

#include <stack>
#include <queue>
#include <set>
#include <iostream>

#include "ray.hpp"
#include "linear_stack.hpp"

using rt::bvh_tree;
using rt::bvh_tree_node;

bvh_tree_node::bvh_tree_node(soup_triangle *b, soup_triangle *e) :
	children_overlap(true),
	begin(b),
	end(e)
{
	aabb vol({0, 0, 0}, {0, 0, 0});
	for (auto p = b; p && p != e; p++)
		vol = aabb{vol, p->get_aabb()};
	bounding_volume = vol;
}

bvh_tree::bvh_tree(const rt::primitive_soup &soup) :
	m_tree(14),
	m_triangles(soup.triangles),
	m_spheres(soup.spheres),
	m_planes(soup.planes)
{
	m_tree.get_root_node().emplace(m_triangles.data(), m_triangles.data() + m_triangles.size());
	build_tree();
}

void bvh_tree::build_tree()
{
	std::stack<linear_tree<bvh_tree_node>::iterator> to_process;
	to_process.push(m_tree.get_root_node());	

	while (!to_process.empty())
	{
		auto it = to_process.top();
		to_process.pop();

		// Cost of triangle intersection
		constexpr float ci = 1.0f;

		// Cost of traversal
		constexpr float ct = 2.0f;

		// Best split information - cost, index and axis
		float best_cost = HUGE_VALF;
		float glm::vec3::* best_axis;
		int best_split = 0;

		// Parent bounding box surface area and triangle count
		float Sp = it->bounding_volume.get_surface_area();
		int np = it->end - it->begin;

		// Sorts elements in given axis
		auto sort_in_axis = [&](float glm::vec3::* axis){
			std::sort(it->begin, it->end, [&](auto &t1, auto &t2){
				glm::vec3 c1 = (t1.vertices[0] + t1.vertices[1] + t1.vertices[2]) / 3.f;
				glm::vec3 c2 = (t2.vertices[0] + t2.vertices[1] + t2.vertices[2]) / 3.f;
				return c1.*axis < c2.*axis;
			});
		};

		

		// Calculates all possible split costs for given axis 
		auto find_best_split = [&](float glm::vec3::* axis)
		{
			sort_in_axis(axis);

			/*
			if (it->end - it->begin < 2) return;

			std::multiset<float> lx, ly, lz;
			std::multiset<float> rx, ry, rz;

			for (auto p = it->begin; p != it->end; p++)
			{
				aabb box = p->get_aabb();
				rx.insert(box.get_min().x);
				rx.insert(box.get_max().x);
				ry.insert(box.get_min().y);
				ry.insert(box.get_max().y);
				rz.insert(box.get_min().z);
				rz.insert(box.get_max().z);
			}
			*/

			// Cost calculation for each possible split
			for (auto split = it->begin; split != it->end - 1; split++)
			{
				int nl = split - it->begin;
				int nr = it->end - split;

				/*
				aabb box = split->get_aabb();


				// Add to the left box
				lx.insert(box.get_min().x);
				lx.insert(box.get_max().x);
				ly.insert(box.get_min().y);
				ly.insert(box.get_max().y);
				lz.insert(box.get_min().z);
				lz.insert(box.get_max().z);

				// Remove from the right box
				rx.erase(rx.lower_bound(box.get_min().x));
				rx.erase(rx.lower_bound(box.get_max().x));
				ry.erase(ry.lower_bound(box.get_min().y));
				ry.erase(ry.lower_bound(box.get_max().y));
				rz.erase(rz.lower_bound(box.get_min().z));
				rz.erase(rz.lower_bound(box.get_max().z));

				// Get min and max in the left box and the right box
				glm::vec3 lmin{
					*lx.begin(),
					*ly.begin(),
					*ly.begin()
				};

				glm::vec3 lmax{
					*lx.rbegin(),
					*ly.rbegin(),
					*ly.rbegin()
				};

				glm::vec3 rmin{
					*rx.begin(),
					*ry.begin(),
					*ry.begin()
				};

				glm::vec3 rmax{
					*rx.rbegin(),
					*ry.rbegin(),
					*ry.rbegin()
				};

				aabb boxl{lmin, lmax}, boxr{rmin, rmax};
				*/
				
				
				// aabb boxl{it->begin->get_aabb()}, boxr{split->get_aabb()};
				aabb boxl{{0, 0, 0}, {0, 0, 0}}, boxr{{0, 0, 0}, {0, 0, 0}};

				// Calculate left bounding volume after split
				for (auto p = it->begin; p != split; p++)
					boxl = aabb{boxl, p->get_aabb()};

				// Calculate right bounding volume after split
				for (auto p = split; p != it->end; p++)
					boxr = aabb{boxr, p->get_aabb()};
							

				float Sl = boxl.get_surface_area();
				float Sr = boxr.get_surface_area();

				float c = ct + Sl / Sp * nl * ci + Sr / Sp * nr * ci;
				if (c < best_cost)
				{
					best_cost = c;
					best_split = split - it->begin;
					best_axis = axis;
				}
			}
		};

		// Find best split point in all axes
		find_best_split(&glm::vec3::x);
		find_best_split(&glm::vec3::y);
		find_best_split(&glm::vec3::z);

		// Compare best split cost to 'leaving as is' cost
		if (best_cost > np * ci)
			continue;

		// Sort the elements in the best axis
		sort_in_axis(best_axis);
		auto split = it->begin + best_split;
		
		std::cout << "tree index: " << it.get_tree_index() << std::endl;

		// Create children and remove triangles from this node
		it.left().emplace(it->begin, split);
		it.right().emplace(split, it->end);
		it->begin = nullptr;
		it->end = nullptr;

		// Check if the children overlap
		it->children_overlap = it.left()->bounding_volume.check_aabb_overlap(it.right()->bounding_volume);

		// Add children to the processing stack
		to_process.push(it.left());
		to_process.push(it.right());
	}
}

bool bvh_tree::cast_ray(const rt::ray &r, ray_hit &best_hit) const
{
	rt::ray_hit h;
	best_hit.distance = HUGE_VALF;

	// Check spheres
	for (const auto &s : m_spheres)
		if (s.cast_ray(r, h))
			best_hit = std::min(best_hit, h);

	// Check planes
	for (const auto &p : m_planes)
		if (p.cast_ray(r, h))
			best_hit = std::min(best_hit, h);


	// Intersections with tree nodes
	rt::linear_stack<node_intersection, 256> intersections;

	// Check intersection with the root node
	auto root = m_tree.get_root_node();
	if (root.has_value())
	{
		float t = root->bounding_volume.ray_intersection_distance(r);
		if (t != HUGE_VALF)
			intersections.emplace(root, t);
	}

	// Process all intersections
	while (!intersections.empty())
	{
		auto isec = intersections.top();
		intersections.pop();
		auto node = isec.node;
		float t = isec.t;

		// If an intersection was found earlier, closer than this volume itself - skip
		if (best_hit.distance < t) continue;

		// If this is a leaf node, intersect all triangles
		// and do not traverse further
		if (node->begin != node->end)
		{
			for (auto p = node->begin; p != node->end; p++)
				if (p->cast_ray(r, h))
					best_hit = std::min(best_hit, h);

			continue;
		}

		// Compute child nodes intersections
		float tl = node.left()->bounding_volume.ray_intersection_distance(r);
		float tr = node.right()->bounding_volume.ray_intersection_distance(r);

		if (tl == HUGE_VALF && tr == HUGE_VALF)
			continue;
		else if (tl == HUGE_VALF || tr == HUGE_VALF)
		{
			// Missed only one child
			if (tr == HUGE_VALF && tl < best_hit.distance) intersections.emplace(node.left(), tl);
			else if (tl == HUGE_VALF && tr < best_hit.distance) intersections.emplace(node.right(), tr);
		}
		else
		{
			// Hit both
			// If the children overlap, put the farther child on the stack first
			// Otherwise only check the closer one
			if (node->children_overlap)
			{
				if (tl > tr)
				{
					if (tl < best_hit.distance)
						intersections.emplace(node.left(), tl);
					
					intersections.emplace(node.right(), tr);
				}
				else
				{
					if (tr < best_hit.distance)
						intersections.emplace(node.right(), tr);

					intersections.emplace(node.left(), tl);
				}
			}
			else // Only check the closer one
			{
				if (tl > tr)
					intersections.emplace(node.left(), tl);
				else
					intersections.emplace(node.right(), tr);
			}
		}
	}

	return best_hit.distance != HUGE_VALF;
}
