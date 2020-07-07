#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <random>
#include <chrono>
#include <SFML/Graphics.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ray.hpp"
#include "camera.hpp"
#include "primitive.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "mesh_data.hpp"
#include "aabb.hpp"
#include "bvh_tree.hpp"

#include "materials/pbr_material.hpp"

static void sfml_thread_main(std::uint8_t *pixel_data, int width, int height, std::mutex *pixel_data_mutex)
{
	// Open a SFML window
	sf::RenderWindow window(sf::VideoMode(width, height), "rt");

	// The displayed texture
	sf::Texture tex;
	tex.create(width, height);

	// Sprite needed for displaying the texture
	sf::Sprite spr;
	spr.setTexture(tex);
	spr.setPosition({0, 0});

	// Main loop
	while (window.isOpen())
	{
		// Process events
		for (sf::Event ev{}; window.pollEvent(ev);)
		{
			if (ev.type == sf::Event::Closed)
				window.close();
		}

		// Draw current buffer
		{
			std::lock_guard lock{*pixel_data_mutex};
			tex.update(pixel_data);
			window.draw(spr);
		}

		window.display();
	}
}

int main(int argc, char **argv)
{
	using namespace std::chrono_literals;

	// The image data
	int width = 1024;
	int height = 1024;
	std::vector<uint8_t> pixels(width * height * 4);
	std::mutex pixels_mutex;

	// This thread displays contents of 'pixels' in real time
	std::packaged_task<void()> preview_task{[&pixels, &pixels_mutex, width, height](){
		sfml_thread_main(&pixels[0], width, height, &pixels_mutex);
	}};
	auto preview_task_fut = preview_task.get_future();
	std::thread preview_thread(std::move(preview_task));
	
	// The scene and camera
	rt::scene scene;

	// Test material
	rt::pbr_material red_mat{{0.7, 0.1, 0.1}, 0.1};
	rt::pbr_material better_red_mat{{0.37, 0.0, 0.0}, 0.05};
	rt::pbr_material gold_mat{{0.8, 0.4, 0.1}, 0.2, 1.0};
	rt::pbr_material green_mat{{0.1, 0.6, 0.1}, 0.5};
	rt::pbr_material white_mat{{0.9, 0.9, 0.9}, 0.5};
	rt::pbr_material glow_mat{{0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, glm::vec3{100.f}};
	rt::pbr_material mirror_mat{{0.8f, 0.8f, 0.8f}, 0.05f, 1.0f};

	// Red sphere
	rt::primitive_collection s{rt::sphere{{0, 2, 0}, 2}};
	rt::scene_object sphere_obj{s, red_mat};
	// scene.add_object(&sphere_obj);

	// Green sphere
	rt::primitive_collection s2{rt::sphere{{3, 1, 1}, 1}};
	rt::scene_object sphere2_obj{s2, green_mat};
	scene.add_object(&sphere2_obj);

	// Plane
	rt::primitive_collection p{rt::plane{{0, 0, 0}, {0, 1, 0}}};
	rt::scene_object plane_obj{p, white_mat};
	scene.add_object(&plane_obj);

	// Glowing sphere
	rt::primitive_collection s3{rt::sphere{{1.5, 0.5, 3}, 0.5}};
	rt::scene_object sphere3_obj{s3, glow_mat};
	scene.add_object(&sphere3_obj);

	// Mirror sphere
	rt::primitive_collection s4{rt::sphere{{-1.5, 0.5, 3}, 0.5}};
	rt::scene_object sphere4_obj{s4, mirror_mat};
	scene.add_object(&sphere4_obj);

	// Golden sphere
	rt::primitive_collection s5{rt::sphere{{5, 3, -4}, 3}};
	rt::scene_object sphere5_obj{s5, gold_mat};
	scene.add_object(&sphere5_obj);

	// Back wall
	rt::plane wall_b{{0, 0, -8}, {0, 0, 1}};
	rt::scene_object wall_b_obj{rt::primitive_collection{wall_b}, white_mat};
	// scene.add_object(&wall_b_obj);

	// Left wall
	rt::plane wall_l{{-4, 0, 0}, {1, 0, 0}};
	rt::scene_object wall_l_obj{rt::primitive_collection{wall_l}, green_mat};
	// scene.add_object(&wall_l_obj);

	// Right wall
	rt::plane wall_r{{5, 0, 0}, {-1, 0, 0}};
	rt::scene_object wall_r_obj{rt::primitive_collection{wall_r}, red_mat};
	// scene.add_object(&wall_r_obj);

	// Test tri
	rt::triangle tri;
	tri.vertices[0] = {-10, 2, -10};
	tri.vertices[1] = {10, 2, -10};
	tri.vertices[2] = {0, 10, -8};
	tri.normals[0] = glm::normalize(glm::cross(tri.vertices[0] - tri.vertices[1], tri.vertices[0] - tri.vertices[2]));
	tri.normals[1] = glm::normalize(glm::cross(tri.vertices[0] - tri.vertices[1], tri.vertices[0] - tri.vertices[2]));
	tri.normals[2] = glm::normalize(glm::cross(tri.vertices[0] - tri.vertices[1], tri.vertices[0] - tri.vertices[2]));
	// rt::scene_object tri_obj{tri, mirror_mat};
	// scene.add_object(&tri_obj);

	// Test mesh
	rt::mesh_data monkey("monkey.obj");
	rt::primitive_collection monkey_pc{monkey};
	rt::scene_object monkey_obj{monkey_pc, red_mat};
	// monkey_obj.set_transform(glm::gtx::translate(0, 1, 0));
	monkey_obj.set_transform(glm::translate(glm::vec3(1.f, 1.f, 0.f)));
	// std::cout << monkey_pc.triangles.size() << std::endl;
	// rt::scene_object monkey_obj{rt::primitive_collection{monkey}, red_mat};
	scene.add_object(&monkey_obj);

	rt::mesh_data bunny("bunny.obj");
	rt::primitive_collection bunny_pc{bunny};
	rt::scene_object bunny_obj{bunny_pc, better_red_mat};
	bunny_obj.set_transform(glm::translate(glm::vec3(-1.f, 0.f, 1.5f)) * glm::scale(glm::vec3(2.f)));
	scene.add_object(&bunny_obj);




	// rt::aabb a({-1, 1, -1}, {1, 10, 1});
	// rt::scene_object a_obj{a, red_mat};
	// scene.add_object(&a_obj);

	// BVH accelerator
	std::cerr << "building BVH..." << std::endl;
	rt::bvh_tree bvh{scene};
	std::cerr << "done" << std::endl;


	// Camera setup
	rt::camera cam({0, 2, 6}, {0, 0, 1}, {0, 1, 0}, 0.01, glm::radians(60.f), 1.f);
	cam.look_at({0, 0.5, 0});

	// Renderer
	rt::renderer ren{scene, cam, {width, height}, bvh};

	// Main random device
	std::random_device rnd;

	// Start time
	auto t_start = std::chrono::high_resolution_clock::now();

	// While the preview is open
	for (int i = 1; preview_task_fut.wait_for(0ms) != std::future_status::ready; i++)
	{
		ren.sample(rnd());
		
		{
			std::lock_guard lock{pixels_mutex};
			ren.pixels_to_rgba(pixels.data());
		}

		auto t_now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> t_total = t_now - t_start;

		std::cerr << std::setw(4) << i << " samples - time = " << std::setw(8) << t_total.count() << "s, per sample = " << std::setw(8) << t_total.count() / i << "s" << std::endl;
	}

	preview_thread.join();
	return EXIT_SUCCESS;	
}