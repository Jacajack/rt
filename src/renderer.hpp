#pragma once
#include <vector>

#include "camera.hpp"
#include "scene.hpp"

namespace rt {

/**
	Takes a camera and a scene and produces image
*/
class renderer
{
public:
	renderer(const scene &sc, const camera &cam, const glm::ivec2 &resolution);

	void sample(int seed);

	void pixels_to_rgba(uint8_t *ptr);

private:
	const scene *m_scene;
	const camera *m_camera;
	glm::ivec2 m_resolution;

	std::vector<glm::vec3> m_pixels;
};

}