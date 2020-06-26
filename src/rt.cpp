#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <SFML/Graphics.hpp>

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
	
	// While the preview is open
	while (preview_task_fut.wait_for(0ms) != std::future_status::ready)
	{

	}

	preview_thread.join();
	return EXIT_SUCCESS;	
}