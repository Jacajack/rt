#include "blender_jsd_loader.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <nlohmann/json.hpp>

#include "scene.hpp"
#include "materials/pbr_material.hpp"
#include "materials/glass.hpp"

using namespace nlohmann;

/**
	Creates material based on data from the JSON file

	\todo update to work with upcoming material system
*/
static std::shared_ptr<rt::abstract_material> make_material(const json &m)
{
	glm::vec3 base_color;
	base_color.r = m["base_color"][0].get<float>();
	base_color.g = m["base_color"][1].get<float>();
	base_color.b = m["base_color"][2].get<float>();

	glm::vec3 emission;
	emission.r = m["emission"][0].get<float>();
	emission.g = m["emission"][1].get<float>();
	emission.b = m["emission"][2].get<float>();

	float metallic = m["metallic"].get<float>();
	float roughness = 0.05f + m["roughness"].get<float>();
	float transmission = m["transmission"].get<float>();
	float ior = m["ior"].get<float>();

	if (transmission)
		return std::make_shared<rt::simple_glass_material>(base_color, ior);
	else
		return std::make_shared<rt::pbr_material>(base_color, roughness, metallic, emission);
}

rt::scene rt::load_jsd_scene(const std::string &path)
{
	std::ifstream f(path);
	std::stringstream ss;
	ss << f.rdbuf();

	auto scene_data = json::parse(ss.str());

	// Default material
	auto default_material = std::make_shared<rt::pbr_material>(glm::vec3{0.9, 0.9, 0.9}, 0.5);

	// New scene data
	rt::scene sc;
	std::vector<std::shared_ptr<rt::abstract_material>> materials{{default_material}};

	// Iterate over objects
	for (auto &obj : scene_data["objects"])
	{
		std::cerr << "Loading object '" << obj["name"].get<std::string>() << "'..." << std::endl;
		
		std::vector<std::shared_ptr<rt::abstract_material>> material_slots;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<rt::triangle> triangles;

		// Load materials
		for (auto &m : obj["materials"])
		{
			auto mat = make_material(m);
			material_slots.push_back(mat);
			materials.push_back(mat);
		}
		
		std::cout << "\t- " << material_slots.size() << " materials" << std::endl;

		// Load vertex data
		for (auto &v : obj["vertices"])
		{
			glm::vec3 u;
			u.x = v["p"][0].get<float>(); 
			u.y = v["p"][1].get<float>(); 
			u.z = v["p"][2].get<float>(); 
			positions.push_back(u);
			u.x = v["n"][0].get<float>(); 
			u.y = v["n"][1].get<float>(); 
			u.z = v["n"][2].get<float>(); 
			normals.push_back(u);
		}
		
		std::cout << "\t- " << positions.size() << " vertex positions" << std::endl;
		std::cout << "\t- " << normals.size() << " vertex normals" << std::endl;

		// Load faces
		for (auto &f : obj["faces"])
		{
			int i0 = f["vi"][0].get<int>();
			int i1 = f["vi"][1].get<int>();
			int i2 = f["vi"][2].get<int>();

			rt::triangle t;
			t.vertices[0] = positions.at(i0);
			t.vertices[1] = positions.at(i1);
			t.vertices[2] = positions.at(i2);
			
			if (f["sm"].get<bool>())
			{
				t.normals[0] = normals.at(i0);
				t.normals[1] = normals.at(i1);
				t.normals[2] = normals.at(i2);
			}
			else
			{
				glm::vec3 N;
				N.x = f["n"][0].get<float>();
				N.y = f["n"][1].get<float>();
				N.z = f["n"][2].get<float>();
				t.normals[0] = t.normals[1] = t.normals[2] = N;
			}
			
			try
			{
				t.material = material_slots.at(f["mat_id"].get<int>()).get();
				// t.material = material_slots.at(0).get();
			}
			catch (const std::out_of_range &ex)
			{
				t.material = default_material.get();
			}

			triangles.push_back(t);
		}

		std::cout << "\t- " << triangles.size() << " triangles" << std::endl;

		// Create collection and object
		auto col = std::make_shared<rt::primitive_collection>();
		col->triangles = std::move(triangles);
		sc.add_object(std::make_shared<rt::scene_object>(col));
	}

	// Add materials
	for (auto &m : materials)
		sc.add_material(m);

	return sc;
}