#pragma once
#include <vector>
#include <thread>
#include <memory>
#include <atomic>
#include <iosfwd>

#include "path_tracer.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "ray_accelerator.hpp"

namespace rt {

/**
	Renders tonemapped
*/
class renderer
{
	friend std::ostream &operator<<(std::ostream &, const renderer &);

public:
	renderer(
		const scene &sc,
		int width,
		int height,
		unsigned long seed,
		int num_threads);

	~renderer()
	{
		stop();
	}

	/**
		Starts path tracing
	*/
	void start();

	/**
		Stops path tracing peacefully - waits for
		all tracers to finish, so the resulting image
		is evenly sampled.
	*/
	void stop();

	/**
		Kills path tracing threads
	*/
	void terminate();

	/**
		Clears data accumulated in path tracers
	*/
	void clear();

	/**
		Computes resulting image from data currently stored
		in the path tracers
	*/
	void compute_result();

	const rt::sampled_hdr_image &get_image() const;

private:
	const scene *m_scene;

	//! Active flag
	std::unique_ptr<std::atomic<bool>> m_active_flag;

	//! Path tracers
	std::vector<rt::path_tracer> m_tracers;

	//! Images from path tracers
	std::vector<rt::sampled_hdr_image> m_images;

	//! Threads running path tracing
	std::vector<std::thread> m_threads;
	int m_thread_count;

	//! Resulting image
	rt::sampled_hdr_image m_image;

	//! Ran by each rendering thread
	static void render_thread(path_tracer &ctx, const std::atomic<bool> &active);
};

extern std::ostream &operator<<(std::ostream &, const renderer &);

}