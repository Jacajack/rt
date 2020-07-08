#include "renderer.hpp"
#include <random>

using rt::renderer;

renderer::renderer(
		const scene &sc,
		const camera &cam,
		const ray_accelerator &accel,
		int width,
		int	height,
		unsigned long seed,
		int num_threads) :
	m_scene(&sc),
	m_camera(&cam),
	m_accelerator(&accel),
	m_active_flag(std::make_unique<bool>(false)),
	m_thread_count(num_threads),
	m_width(width),
	m_height(height),
	m_hdr(width * height, glm::vec3{0.f}),
	m_sample_count(0),
	m_raw(width * height * 4, 0)
{
	// Random generator used only for initialization
	// of path tracers' generators
	std::mt19937 init_rnd(seed);

	// Initialize all path tracers
	m_tracers.reserve(m_thread_count);
	for (int i = 0; i < m_thread_count; i++)
	{
		m_tracers.emplace_back(
			*m_camera,
			*m_scene,
			*m_accelerator,
			m_width,
			m_height,
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
		m_threads.emplace_back(renderer::render_thread, &m_tracers[i], m_active_flag.get());
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

	std::fill(m_hdr.begin(), m_hdr.end(), glm::vec3{0.f});
	std::fill(m_raw.begin(), m_raw.end(), 0);
	m_sample_count = 0;
}

void renderer::render_thread(path_tracer *ctx, const bool *active)
{
	while (*active)
		ctx->sample_image();
}

/**
	\todo Move tonemapping into image class
*/
void renderer::compute_result()
{
	m_sample_count = 0;
	std::fill(m_hdr.begin(), m_hdr.end(), glm::vec3{0.f});

	// Compute HDR
	for (auto &t : m_tracers)
	{
		std::transform(
					t.get_image().cbegin(), t.get_image().cend(),
					m_hdr.begin(),
					m_hdr.begin(),
					std::plus<glm::vec3>()
			);
		m_sample_count += t.get_sample_count();
	}

	for (auto &p : m_hdr)
		p /= static_cast<float>(m_sample_count);
	
	// Tonemap
	std::uint8_t *ptr = m_raw.data();
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			// Reinhard tonemapping and sRGB correction
			glm::vec3 pix =	m_hdr[y * m_width + x];
			pix = pix / (pix + 1.f);
			pix = glm::pow(pix, glm::vec3{1.f / 2.2f});

			*ptr++ = pix.r * 255.99f;
			*ptr++ = pix.g * 255.99f;
			*ptr++ = pix.b * 255.99f;
			*ptr++ = 255;
		}
	}
}

const int renderer::get_sample_count() const
{
	return m_sample_count;
}

const std::vector<glm::vec3> &renderer::get_hdr_image() const
{
	return m_hdr;
}

const std::vector<std::uint8_t> &renderer::get_ldr_image() const
{
	return m_raw;
}
