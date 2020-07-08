#pragma once
#include <vector>
#include <random>

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
	renderer(const scene &sc, const camera &cam, const ray_accelerator &accel);

private:
	const scene *m_scene;
	const camera *m_camera;
	const ray_accelerator *m_accelerator;

};

}