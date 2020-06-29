#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <random>
#include <SFML/Graphics.hpp>

#include "ray.hpp"
#include "camera.hpp"
#include "primitive.hpp"
#include "scene.hpp"
#include "renderer.hpp"

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
	std::uint8_t pixels[width * height * 4];
	std::mutex pixels_mutex;

	// This thread displays contents of 'pixels' in real time
	std::packaged_task<void()> preview_task{[&pixels, &pixels_mutex, width, height](){
		sfml_thread_main(&pixels[0], width, height, &pixels_mutex);
	}};
	auto preview_task_fut = preview_task.get_future();
	std::thread preview_thread(std::move(preview_task));
	
	// The scene and camera
	rt::scene scene;
	rt::camera cam({0, 4, 8}, {0, 0, 1}, {0, 1, 0}, 0.01, glm::radians(60.f), 1.f);
	rt::renderer ren{scene, cam, {width, height}};

	// Test material
	rt::test_material red_mat{{0.5, 0.1, 0.1}};
	rt::test_material green_mat{{0.1, 0.4, 0.1}};
	rt::test_material white_mat{{0.9, 0.9, 0.9}};

	// Test sphere and floor
	rt::sphere s{{0, 2, 0}, 2};
	rt::sphere s2{{3, 1, 1}, 1};
	rt::scene_object sphere_obj{s, red_mat};
	rt::scene_object sphere2_obj{s2, green_mat};
	scene.add_object(&sphere_obj);
	scene.add_object(&sphere2_obj);
	rt::plane p{{0, 0, 0}, {0, 1, 0}};
	rt::scene_object plane_obj{p, white_mat};
	scene.add_object(&plane_obj);

	// Camera setup
	cam.look_at(s.origin);

	// Main random device
	std::random_device rnd;

	// While the preview is open
	for (int i = 0; preview_task_fut.wait_for(0ms) != std::future_status::ready && i < 3; i++)
	{
		ren.sample(rnd());
		
		{
			std::lock_guard lock{pixels_mutex};
			ren.pixels_to_rgba(pixels);
		}
	}

	preview_thread.join();
	return EXIT_SUCCESS;	
}