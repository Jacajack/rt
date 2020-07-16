#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <random>
#include <chrono>
#include <algorithm>
#include <future>
#include <string>

#include <SFML/Graphics.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ray.hpp"
#include "camera.hpp"
#include "primitive.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "path_tracer.hpp"
#include "mesh_data.hpp"
#include "aabb.hpp"
#include "bvh_tree.hpp"

#include "materials/pbr_material.hpp"
#include "materials/glass.hpp"

int main(int argc, char **argv)
{
	using namespace std::chrono_literals;

	// The window size
	glm::ivec2 window_size{1024, 1024};

	// The image data
	glm::ivec2 render_size{1024, 1024};
	std::vector<uint8_t> pixels(render_size.x * render_size.y * 4);

	// The scene and camera
	rt::scene scene;

	// Test material
	rt::pbr_material red_mat{{0.7, 0.1, 0.1}, 0.1};
	rt::pbr_material better_red_mat{{0.37, 0.0, 0.0}, 0.05};
	rt::pbr_material gold_mat{{0.8, 0.4, 0.1}, 0.2, 1.0};
	rt::pbr_material green_mat{{0.1, 0.6, 0.1}, 0.5};
	rt::pbr_material white_mat{{0.9, 0.9, 0.9}, 0.5};
	rt::pbr_material glow_mat{{0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, glm::vec3{25.f}};
	rt::pbr_material mirror_mat{{0.8f, 0.8f, 0.8f}, 0.05f, 1.0f};
	rt::simple_glass_material glass_mat{glm::vec3{0.8f, 0.042f, 0.161f}, 1.50f};

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
	rt::scene_object sphere5_obj{s5, better_red_mat};
	scene.add_object(&sphere5_obj);

	// Glass sphere
	rt::primitive_collection s6{rt::sphere{{-2.f, 0.5, 0}, 0.5}};
	rt::scene_object sphere6_obj{s6, better_red_mat};
	scene.add_object(&sphere6_obj);

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

	// Test mesh
	rt::mesh_data monkey("resources/monkey.obj");
	rt::primitive_collection monkey_pc{monkey};
	rt::scene_object monkey_obj{monkey_pc, red_mat};
	// monkey_obj.set_transform(glm::gtx::translate(0, 1, 0));
	monkey_obj.set_transform(glm::translate(glm::vec3(1.f, 1.f, 0.f)));
	// std::cout << monkey_pc.triangles.size() << std::endl;
	// rt::scene_object monkey_obj{rt::primitive_collection{monkey}, red_mat};
	scene.add_object(&monkey_obj);

	rt::mesh_data bunny("resources/bunny.obj");
	rt::primitive_collection bunny_pc{bunny};
	rt::scene_object bunny_obj{bunny_pc, glass_mat};
	bunny_obj.set_transform(glm::translate(glm::vec3(0.f, 0.f, 2.f)) * glm::scale(glm::vec3(2.f)));
	scene.add_object(&bunny_obj);

	// BVH accelerator
	std::cerr << "building BVH..." << std::endl;
	auto t_bvh_start = std::chrono::high_resolution_clock::now();
	rt::bvh_tree bvh{scene};
	auto t_bvh_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> t_bvh = t_bvh_end - t_bvh_start;
	std::cerr << "done - took " << t_bvh.count() << "s" << std::endl;


	// Camera setup
	rt::camera cam({0, 2, 6}, {0, 0, 1}, {0, 1, 0}, 0.01, glm::radians(60.f), 1.f);
	cam.look_at({0, 0.5, 0});
	scene.set_camera(cam);

	// Main random device
	std::random_device rnd;

	
	// ----------------------------------------------------------------------------
	

	// Open a SFML window
	sf::RenderWindow window(sf::VideoMode(window_size.x, window_size.y), "rt");

	// The displayed texture
	sf::Texture tex;
	tex.create(window_size.x, window_size.y);

	// Sprite needed for displaying the texture
	sf::Sprite spr;
	spr.setTexture(tex);
	spr.setPosition({0, 0});

	// The renderer
	const int render_threads = 6;
	rt::renderer ren(scene, cam, bvh, render_size.x, render_size.y, rnd(), render_threads);
	ren.start();

	// Start time and sample count
	auto t_start = std::chrono::high_resolution_clock::now();
	int samples = 0, last_samples = 0;

	// Changes with mouse drags
	glm::vec2 drag_pos{rt::pi<> * 0.5f, 0.f}, drag_start{0.f};
	float drag_speed = 2.f;
	bool drag_pending = false;

	float camera_speed = 2.5f;
	glm::vec3 camera_dir{0, 0, -1};
	glm::vec3 camera_velocity{0.f};

	// Main loop
	sf::Clock dt_clock;
	while (window.isOpen())
	{
		float dt = dt_clock.restart().asSeconds();

		// Process events
		for (sf::Event ev{}; window.pollEvent(ev);)
		{
			switch (ev.type)
			{
				case sf::Event::MouseButtonPressed:
				{
					auto start = window.mapPixelToCoords(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));
					drag_start = glm::vec2{start.x, -start.y} / glm::vec2{window_size};
					drag_pending = true;
					break;
				}

				case sf::Event::MouseButtonReleased:
				{
					drag_pending = false;
					break;
				}

				case sf::Event::MouseMoved:
				{
					if (!drag_pending) break;
					auto current = window.mapPixelToCoords(sf::Vector2i(ev.mouseMove.x, ev.mouseMove.y));
					glm::vec2 pos = glm::vec2{current.x, -current.y} / glm::vec2{window_size};
					auto delta = pos - drag_start;
					drag_pos += delta * drag_speed;
					drag_start = pos;
					std::cout << drag_pos.x << " " << drag_pos.y << std::endl;
					
					drag_pos.y = glm::clamp(drag_pos.y, -rt::pi<> * 0.49f, rt::pi<> * 0.49f);

					float theta = -drag_pos.y;
					float phi = -drag_pos.x;
					camera_dir.x = std::cos(phi) * std::cos(theta);
					camera_dir.y = std::sin(theta);
					camera_dir.z = std::sin(phi) * std::cos(theta);
					cam.set_direction(camera_dir);
					ren.clear();
					break;
				}

				case sf::Event::KeyPressed:
				{
					if (ev.key.code == sf::Keyboard::W) camera_velocity.z = camera_speed;
					if (ev.key.code == sf::Keyboard::S) camera_velocity.z = -camera_speed;
					if (ev.key.code == sf::Keyboard::A) camera_velocity.x = -camera_speed;
					if (ev.key.code == sf::Keyboard::D) camera_velocity.x = camera_speed;
					if (ev.key.code == sf::Keyboard::C)
					{
						std::stringstream ss;
						ss << std::time(nullptr);
						ss << "-" << ren.get_sample_count() << "S";
						ss << ".png"; 
						spr.getTexture()->copyToImage().saveToFile(ss.str());
					}
					break;
				}

				case sf::Event::KeyReleased:
				{
					if (ev.key.code == sf::Keyboard::W) camera_velocity.z = 0.f;
					if (ev.key.code == sf::Keyboard::S) camera_velocity.z = 0.f;
					if (ev.key.code == sf::Keyboard::A) camera_velocity.x = 0.f;
					if (ev.key.code == sf::Keyboard::D) camera_velocity.x = 0.f;
					break;
				}

				case sf::Event::Closed:
					window.close();
					break;

				default:
					break;
			}			
		}

		// Move the camera
		if (glm::length(camera_velocity) > 0.0001f) ren.clear();
		cam.set_position(cam.get_position() + cam.get_matrix() * camera_velocity * dt);

		// Compute temp result
		ren.compute_result();
		std::copy(ren.get_ldr_image().begin(), ren.get_ldr_image().end(), pixels.begin());
		last_samples = samples;
		samples = ren.get_sample_count();

		// Print some info
		auto t_now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> t_total = t_now - t_start;
		if (samples != last_samples)
		{
			std::cout << std::setw(4) << samples << " samples - time = " << std::setw(8) << std::fixed << t_total.count() 
				<< "s, per sample = " << std::setw(8) << std::fixed << t_total.count() / samples
				<< "s, per sample/th = " << std::setw(8) << std::fixed << t_total.count() / samples * render_threads << std::endl;
			std::cout << ren << std::endl;
		}
		
		// Draw
		tex.update(pixels.data());
		glm::vec2 preview_scale = glm::vec2{window_size} / glm::vec2{render_size};
		spr.setScale(preview_scale.x, preview_scale.y);
		window.draw(spr);
		window.display();
	}

	ren.stop();
	return EXIT_SUCCESS;	
}