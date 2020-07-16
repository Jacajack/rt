#include "renderer.hpp"
#include <random>
#include <iostream>

using rt::renderer;

renderer::renderer(
		const scene &sc,
		int width,
		int	height,
		unsigned long seed,
		int num_threads) :
	m_scene(&sc),
	m_active_flag(std::make_unique<std::atomic<bool>>(false)),
	m_thread_count(num_threads),
	m_image(width, height)
{
	// Random generator used only for initialization
	// of path tracers' generators
	std::mt19937 init_rnd(seed);

	// Initialize output images
	m_images.reserve(m_thread_count);
	for (int i = 0; i < m_thread_count; i++)
	{
		m_images.emplace_back(width, height);
	}

	// Initialize all path tracers
	m_tracers.reserve(m_thread_count);
	for (int i = 0; i < m_thread_count; i++)
	{
		m_tracers.emplace_back(
			*m_scene,
			m_images[i],
			init_rnd());
	}
}

void renderer::start()
{
	if (!m_threads.empty())
		throw std::runtime_error("rt::renderer already running...");

	*m_active_flag = true;

	// Spawn threads
	for (int i = 0; i < m_thread_count; i++)
		m_threads.emplace_back(renderer::render_thread, std::ref(m_tracers[i]), std::ref(*m_active_flag));
}

void renderer::stop()
{
	*m_active_flag = false;

	// Wait for every thread to join
	for (auto &t : m_threads)
		t.join();

	// Remove all threads
	m_threads.clear();
}

/**
	\todo Implement thread killing
*/
void renderer::terminate()
{
	*m_active_flag = false;
	throw std::runtime_error("not implemented!");
}

/**
	Clears buffers in all path tracers and HDR and raw data buffers
*/
void renderer::clear()
{
	for (auto &t : m_tracers)
		t.clear_image();
	m_image.clear();
}

void renderer::render_thread(path_tracer &ctx, const std::atomic<bool> &active)
{
	while (active)
		ctx.sample_image(40, 0.f, &active);
}

/**
	\todo Move tonemapping into image class
*/
void renderer::compute_result()
{
	m_image.clear();

	// Compute resulting image
	for (int i = 0; i < m_thread_count; i++)
		m_image += m_images[i];
}

const rt::sampled_hdr_image &renderer::get_image() const
{
	return m_image;
}

std::ostream &rt::operator<<(std::ostream &s, const renderer &r)
{
	// Print last times per sample
	s << "s/S :\t";
	for (auto &t : r.m_tracers)
	{
		s << t.get_last_sample_time().count() << "\t";
	}

	return s;
}
