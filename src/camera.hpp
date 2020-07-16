#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"

namespace rt {

class camera
{
public:
	camera(const glm::vec3 &pos, const glm::vec3 &forward, const glm::vec3 &up, float near, float fov, float aspect);
	void set_position(const glm::vec3 &pos);
	void set_direction(const glm::vec3 &forward, const glm::vec3 &up = {0, 1, 0});
	void look_at(const glm::vec3 &target, const glm::vec3 &up = {0, 1, 0});
	void set_near_plane(float near);
	void set_fov(float fov);
	void set_aspect_ratio(float aspect);

	const glm::vec3 &get_position() const;
	glm::mat3 get_matrix() const;
	inline ray get_ray(const glm::vec2 &pixel_pos) const;

private:
	void update_near_plane();

	// Position
	glm::vec3 m_position;

	// Camera coordinate system
	glm::vec3 m_forward;
	glm::vec3 m_left;
	glm::vec3 m_up;

	// Near plane
	float m_near;
	glm::vec2 m_near_plane_half_size;

	// Vectors used for calculating rays
	glm::vec3 m_near_forward;
	glm::vec3 m_near_right;
	glm::vec3 m_near_up;

	// Aspect ratio (x/y) and FOV
	float m_aspect;
	float m_fov;
};

/**
	Takes pixel position on screen (-1;1) and returns a ray passing through it
*/
rt::ray camera::get_ray(const glm::vec2 &pixel_pos) const
{
	return ray{m_position, m_near_forward + m_near_right * pixel_pos.x + m_near_up * pixel_pos.y};
}


}