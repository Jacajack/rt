#include "renderer.hpp"
#include <random>
#include "linear_stack.hpp"

using rt::renderer;

renderer::renderer(const scene &sc, const camera &cam, const ray_accelerator &accel) :
	m_scene(&sc),
	m_camera(&cam),
	m_accelerator(&accel)
{
}
