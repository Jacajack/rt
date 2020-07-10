#include "bvh_tree.hpp"

#include <stack>
#include <memory>
#include <future>

#include "ray.hpp"
#include "containers/linear_stack.hpp"

using rt::bvh_tree;
using rt::bvh_tree_node;

bvh_tree_node::bvh_tree_node(rt::triangle *b, rt::triangle *e) :
	begin(b),
	end(e)
{
	aabb vol{b->get_aabb()};
	for (auto p = b + 1; p < e; p++)
		vol = aabb{vol, p->get_aabb()};
	bounding_volume = vol;
}

bvh_tree::bvh_tree(const rt::scene &scene) :
	m_tree(4)
{
	//! \todo Improve BVH build process by partitioning entire objects first

	// Unpack and decompose scene
	for (const auto &obj_ptr : scene.get_objects())
	{
		primitive_collection col{obj_ptr->get_transformed_primitive_collection()};
		std::copy(col.triangles.begin(), col.triangles.end(), std::back_inserter(m_triangles));
		std::copy(col.spheres.begin(), col.spheres.end(), std::back_inserter(m_spheres));
		std::copy(col.planes.begin(), col.planes.end(), std::back_inserter(m_planes));
	}

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
		constexpr float ct = 4.0f;

		// Parent bounding box surface area and triangle count
		float Sp = it->bounding_volume.get_surface_area();
		int np = it->end - it->begin;

		// Calculates all possible split costs for given axis 
		auto find_best_split = [&](int &best_split, float &best_cost, float glm::vec3::* axis)
		{
			auto data = std::make_unique<std::vector<rt::triangle>>(it->begin, it->end);

			// Sort the data
			std::sort(data->begin(), data->end(), [&](auto &t1, auto &t2){
				glm::vec3 c1 = (t1.vertices[0] + t1.vertices[1] + t1.vertices[2]) / 3.f;
				glm::vec3 c2 = (t2.vertices[0] + t2.vertices[1] + t2.vertices[2]) / 3.f;
				return c1.*axis < c2.*axis;
			});

			// Reset cost
			best_cost = HUGE_VALF;

			// AAB collection
			aabb_collection lcol, rcol(data->begin(), data->end());

			// Cost calculation for each possible split
			for (auto split = data->begin(); split != data->end() - 1; split++)
			{
				lcol.push(split->get_aabb());
				rcol.pop(split->get_aabb());
				int nl = split - data->begin() + 1;
				int nr = data->end() - split - 1;

				aabb boxl{lcol.get_aabb()}, boxr{rcol.get_aabb()};

				// Box surfaces
				float Sl = boxl.get_surface_area();
				float Sr = boxr.get_surface_area();

				// Calculate split cost
				float c = ct + Sl / Sp * nl * ci + Sr / Sp * nr * ci;
				if (c < best_cost)
				{
					best_cost = c;
					best_split = split - data->begin() + 1; // Mind this +1!
				}
			}

			return std::move(data);
		};

		// Best split index and cost
		int best_split = 0;
		float best_cost;

		// Costs and split points
		float cost_x, cost_y, cost_z;
		int split_x, split_y, split_z;

		// Async flags depending on triangle count
		auto async_policy = (it->end - it->begin) > 10 ? std::launch::async : std::launch::deferred;

		// Find best split point in all axes
		auto fut_x = std::async(async_policy, find_best_split, std::ref(split_x), std::ref(cost_x), &glm::vec3::x);
		auto fut_y = std::async(async_policy, find_best_split, std::ref(split_y), std::ref(cost_y), &glm::vec3::y);
		auto fut_z = std::async(async_policy, find_best_split, std::ref(split_z), std::ref(cost_z), &glm::vec3::z);
		
		// Best data set
		std::unique_ptr<std::vector<triangle>> best_data;

		// Get data from the futures
		auto data_x = fut_x.get();
		auto data_y = fut_y.get();
		auto data_z = fut_z.get();

		if (cost_x < cost_y && cost_x < cost_z) // Best in X axis
		{
			best_data = std::move(data_x);
			best_split = split_x;
			best_cost = cost_x;
		}
		else if (cost_y < cost_x && cost_y < cost_z) // Best in Y
		{
			best_data = std::move(data_y);
			best_split = split_y;
			best_cost = cost_y;
		}
		else // Best in Z
		{
			best_data = std::move(data_z);
			best_split = split_z;
			best_cost = cost_z;
		}

		// Compare best split cost to 'leaving as is' cost
		if (best_cost > np * ci)
			continue;

		// Copy the sorted data back and calculate splitting iterator
		std::copy(best_data->begin(), best_data->end(), it->begin);
		auto split = it->begin + best_split;

		/*
			Create children and remove triangles from this node

			Passing pointers by value here is very important. Otherwise they will
			be passed by reference to the part of tree that might get invalidated
			if it grows.

			I fucking wasted like 3 hours here...
		*/
		rt::triangle *b = it->begin;
		rt::triangle *e = it->end;
		it.left().emplace(b, split);
		it.right().emplace(split, e);
		it->begin = nullptr;
		it->end = nullptr;

		// Add children to the processing stack
		to_process.push(it.left());
		to_process.push(it.right());
	}
}

bool bvh_tree::cast_ray(const rt::ray &r, ray_hit &best_hit) const
{
	// Assume miss
	best_hit.distance = rt::ray_miss;

	// Nearset intersection
	rt::ray_intersection isec;
	isec.distance = rt::ray_miss;

	// Check spheres
	const rt::sphere *best_sphere = rt::sphere::ray_intersect(m_spheres.data(), m_spheres.data() + m_spheres.size(), r, isec);
	if (best_sphere) best_hit = best_sphere->get_ray_hit(isec, r);

	// Check planes
	const rt::plane *best_plane = rt::plane::ray_intersect(m_planes.data(), m_planes.data() + m_planes.size(), r, isec);
	if (best_plane) best_hit = best_plane->get_ray_hit(isec, r);

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
		auto node_isec = intersections.top();
		intersections.pop();
		auto node = node_isec.node;
		float t = node_isec.t;

		// If an intersection was found earlier, closer than this volume itself - skip
		if (best_hit.distance < t) continue;

		// If this is a leaf node, intersect all triangles
		// and do not traverse further
		if (node->begin != node->end)
		{
			const rt::triangle *tri = rt::triangle::ray_intersect(node->begin, node->end, r, isec);
			if (tri != nullptr)
				best_hit = tri->get_ray_hit(isec, r);
			continue;
		}

		// Compute child nodes intersections
		float tl = node.left()->bounding_volume.ray_intersection_distance(r);
		float tr = node.right()->bounding_volume.ray_intersection_distance(r);

		// If at least one child is hit
		// Check the farther child later, so put it on the stack first
		// Also, only put nodes on stack if the closest intersection with them
		// is closer than current best hit
		if (tl != rt::ray_miss || tr != rt::ray_miss)
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
	}

	return best_hit.distance != HUGE_VALF;
}
