#include "blender_jsd_loader.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <nlohmann/json.hpp>

#include "camera.hpp"
#include "scene.hpp"
#include "materials/pbr_material.hpp"
#include "materials/glass.hpp"

using namespace nlohmann;

static void read_json_vector(glm::vec3 &v, const json &j)
{
	for (int i = 0; i < v.length(); i++)
		v[i] = j[i].get<float>();
}

/**
	Creates material based on data from the JSON file

	\todo update to work with upcoming material system
*/
static std::shared_ptr<rt::abstract_material> make_material(const json &m)
{
	glm::vec3 base_color, emission;
	read_json_vector(base_color, m["base_color"]);
	read_json_vector(emission, m["emission"]);

	float metallic = m["metallic"].get<float>();
	float roughness = 0.05f + m["roughness"].get<float>();
	float transmission = m["transmission"].get<float>();
	float ior = m["ior"].get<float>();

	if (transmission)
		return std::make_shared<rt::simple_glass_material>(base_color, ior);
	else
		return std::make_shared<rt::pbr_material>(base_color, roughness, metallic, emission);
}

/**
	Creates camera from camera stored in 
*/
static rt::camera make_camera(const json &j)
{
	if (j["type"].get<std::string>() != "camera")
		throw std::runtime_error("non-camera object passed to make_camera in JSD parser");

	float fov = j["fov"][0].get<float>();
	float near = j["near_plane"].get<float>();

	glm::vec3 pos, up, forward;
	read_json_vector(pos, j["position"]);
	read_json_vector(up, j["up"]);
	read_json_vector(forward, j["forward"]);

	return rt::camera{pos, forward, up, near, fov, 1.f};
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
			read_json_vector(u, v["p"]);
			positions.push_back(u);
			read_json_vector(u, v["n"]);
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
				read_json_vector(N, f["n"]);
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

	if (scene_data["cameras"].size() > 0)
		sc.set_camera(std::make_shared<rt::camera>(make_camera(scene_data["cameras"][0])));
	else
	{
		rt::camera default_cam({12, 1, 0}, {0, 0, 1}, {0, 1, 0}, 0.01, glm::radians(60.f), 1.f);
		sc.set_camera(std::make_shared<rt::camera>(default_cam));
	}

	// Add materials
	for (auto &m : materials)
		sc.add_material(m);

	return sc;
}