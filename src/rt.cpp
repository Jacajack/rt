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
#include "blender_jsd_loader.hpp"

#include "materials/pbr_material.hpp"
#include "materials/glass.hpp"

int main(int argc, char **argv)
{
	using namespace std::chrono_literals;

	// The window size
	glm::ivec2 window_size{1024, 1024};

	// The image data
	glm::ivec2 render_size{1024, 1024};

	// The scene
	rt::scene scene = rt::load_jsd_scene("resources/test_box.jsd");



	// ----------------------------------------------------------------------------



	// Main random device
	std::random_device rnd;

	// Initial camera setup
	rt::camera &cam = scene.get_camera();

	// Build BVH
	std::cerr << "building BVH..." << std::endl;
	auto t_bvh_start = std::chrono::high_resolution_clock::now();
	scene.init_accelerator<rt::bvh_tree>();
	auto t_bvh_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> t_bvh = t_bvh_end - t_bvh_start;
	std::cerr << "done - took " << t_bvh.count() << "s" << std::endl;

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
	rt::renderer ren(scene, render_size.x, render_size.y, rnd(), render_threads);
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
					scene.get_camera().set_direction(camera_dir);
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
						ss << "-" << ren.get_image().get_sample_count() << "S";
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
		rt::image<rt::rgba_pixel> img(ren.get_image());
		last_samples = samples;
		samples = ren.get_image().get_sample_count();

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
		tex.update(reinterpret_cast<const std::uint8_t*>(img.get_data().data()));
		glm::vec2 preview_scale = glm::vec2{window_size} / glm::vec2{render_size};
		spr.setScale(preview_scale.x, preview_scale.y);
		window.draw(spr);
		window.display();
	}

	ren.stop();
	return EXIT_SUCCESS;	
}