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
#include "materials/glass.hpp"

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
	rt::scene_object sphere5_obj{s5, gold_mat};
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
	rt::scene_object bunny_obj{bunny_pc, glass_mat};
	bunny_obj.set_transform(glm::translate(glm::vec3(0.f, 0.f, 2.f)) * glm::scale(glm::vec3(2.f)));
	scene.add_object(&bunny_obj);

	// BVH accelerator
	std::cerr << "building BVH..." << std::endl;
	rt::bvh_tree bvh{scene};
	std::cerr << "done" << std::endl;


	// Camera setup
	rt::camera cam({0, 2, 6}, {0, 0, 1}, {0, 1, 0}, 0.01, glm::radians(60.f), 1.f);
	cam.look_at({0, 0.5, 0});

	// Main random device
	std::random_device rnd;

	// Renderer
	rt::renderer ren{scene, cam, bvh};

	// Path tracing context
	rt::renderer::path_tracing_context ctx(width, height, rnd());

	// Start time
	auto t_start = std::chrono::high_resolution_clock::now();

	int buffer_pool_size = 20;
	int render_threads = 6;

	std::vector<std::unique_ptr<std::vector<glm::vec3>>> buffer_pool;
	std::condition_variable buffer_pool_cv;
	std::mutex buffer_pool_mutex;


	std::vector<std::unique_ptr<std::vector<glm::vec3>>> dirty_buffer_pool;
	std::mutex dirty_buffer_pool_mutex;

	// Create pools
	buffer_pool.reserve(buffer_pool_size);
	dirty_buffer_pool.reserve(buffer_pool_size);
	for (int i = 0; i < buffer_pool_size; i++)
		buffer_pool.emplace_back(std::make_unique<std::vector<glm::vec3>>(width * height));


	// Create a new context for each thread
	std::vector<rt::renderer::path_tracing_context> contexts;
	contexts.reserve(render_threads);
	for (int i = 0; i < render_threads; i++)
		contexts.emplace_back(width, height, rnd());


	bool active = true;

	auto render_task = [
		&active,
		&ren,
		&buffer_pool,
		&buffer_pool_cv,
		&buffer_pool_mutex,
		&dirty_buffer_pool,
		&dirty_buffer_pool_mutex](rt::renderer::path_tracing_context &ctx)
	{
		using namespace std::chrono_literals;

		while (active)
		{
			// Clear context buffer
			std::fill(ctx.pixels.begin(), ctx.pixels.end(), glm::vec3{0.f});

			ren.sample_image(ctx);
			std::unique_ptr<std::vector<glm::vec3>> buf_ptr;

			{
				// Wait for any clean buffer to be available
				std::unique_lock lock{buffer_pool_mutex};

				while (buffer_pool.empty())
				{
					buffer_pool_cv.wait_for(lock, 100ms);
					if (!active) return;
				}

				// Get a clean buffer
				buf_ptr = std::move(buffer_pool.back());
				buffer_pool.pop_back();
			}

			// Copy rendered data to the buffer
			*buf_ptr = ctx.pixels;

			// Push dirty buffer
			{
				std::lock_guard dirty_lock{dirty_buffer_pool_mutex};
				dirty_buffer_pool.emplace_back(std::move(buf_ptr));
			}
		}
	};

	// Spawn rendering threads
	std::vector<std::thread> threads;
	threads.reserve(render_threads);
	for (int i = 0; i < render_threads; i++)
	{
		threads.emplace_back(render_task, std::ref(contexts[i]));
	}

	// While the preview is open
	for (int i = 1; preview_task_fut.wait_for(0ms) != std::future_status::ready; i++)
	{
		// Consume one dirty buffer
		if (!dirty_buffer_pool.empty())
		{
			// std::cout << "clean/dirty: " << buffer_pool.size() << " " << dirty_buffer_pool.size() << std::endl;

			// Acquire dirty pool
			std::unique_ptr<std::vector<glm::vec3>> buf_ptr;
			{
				std::lock_guard lock{dirty_buffer_pool_mutex};
				buf_ptr = std::move(dirty_buffer_pool.back());
				dirty_buffer_pool.pop_back();
			}

			// Add the pixels to the main context (temporary solution)
			std::transform(
					buf_ptr->begin(), buf_ptr->end(),
					ctx.pixels.begin(),
					ctx.pixels.begin(),
					std::plus<glm::vec3>()
			);

			// Return the buffer
			{
				std::unique_lock lock{buffer_pool_mutex};

				if (buffer_pool.empty())
					std::cout << "no buffers avaiable..." << std::endl;

				buffer_pool.emplace_back(std::move(buf_ptr));
			}

			buffer_pool_cv.notify_all();
			ctx.sample_count++;
		

			{
				std::lock_guard lock{pixels_mutex};

				std::uint8_t *ptr = pixels.data();
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						// Reinhard tonemapping and sRGB correction
						glm::vec3 pix =	ctx.pixels[y * width + x] / float(ctx.sample_count);
						pix = pix / (pix + 1.f);
						pix = glm::pow(pix, glm::vec3{1.f / 2.2f});

						*ptr++ = pix.r * 255.99f;
						*ptr++ = pix.g * 255.99f;
						*ptr++ = pix.b * 255.99f;
						*ptr++ = 255;
					}
				}
			}

			auto t_now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> t_total = t_now - t_start;

			std::cerr << std::setw(4) << ctx.sample_count << " samples - time = " << std::setw(8) << t_total.count() 
				<< "s, per sample = " << std::setw(8) << t_total.count() / ctx.sample_count 
				<< "s, per sample/th = " << std::setw(8) << t_total.count() / ctx.sample_count * render_threads << std::endl;
		}
	}

	active = false;
	for (auto &t : threads)
		t.join();

	preview_thread.join();
	return EXIT_SUCCESS;	
}