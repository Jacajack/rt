#pragma once
#include <vector>
#include <thread>
#include <memory>

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
public:
	renderer(
		const scene &sc,
		const camera &cam,
		const ray_accelerator &accel,
		int width,
		int height,
		unsigned long seed,
		int num_threads);

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

	const int get_sample_count() const;
	const std::vector<glm::vec3> &get_hdr_image() const;
	const std::vector<std::uint8_t> &get_ldr_image() const;

private:
	const scene *m_scene;
	const camera *m_camera;
	const ray_accelerator *m_accelerator;

	//! Active flag
	std::unique_ptr<bool> m_active_flag;

	//! Path tracers
	std::vector<rt::path_tracer> m_tracers;

	//! Threads running path tracing
	std::vector<std::thread> m_threads;
	int m_thread_count;

	//! HDR image
	int m_width;
	int m_height;
	mutable std::vector<glm::vec3> m_hdr;
	mutable int m_sample_count;

	//! Raw 8-bit RGBA image (tonemapped from m_hdr)
	mutable std::vector<std::uint8_t> m_raw;

	//! Ran by each rendering thread
	static void render_thread(path_tracer *ctx, const bool *active);
};

}