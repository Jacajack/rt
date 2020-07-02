#pragma once
#include <vector>

#include "camera.hpp"
#include "scene.hpp"
#include "ray_accelerator.hpp"

namespace rt {

/**
	Takes a camera and a scene and produces image
*/
class renderer
{
public:
	renderer(const scene &sc, const camera &cam, const glm::ivec2 &resolution, const ray_accelerator &accel);

	void sample(int seed);

	void pixels_to_rgba(uint8_t *ptr);

private:
	const scene *m_scene;
	const camera *m_camera;
	const ray_accelerator *m_accelerator;
	glm::ivec2 m_resolution;

	std::vector<glm::vec3> m_pixels;
	int m_samples = 0;
};

}