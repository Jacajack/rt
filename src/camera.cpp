#include "camera.hpp"

using rt::camera;

camera::camera(const glm::vec3 &pos, const glm::vec3 &forward, const glm::vec3 &up, float near, float fov, float aspect) :
	m_position(pos),
	m_near(near),
	m_aspect(aspect),
	m_fov(fov)
{
	set_direction(forward, up);
}

void camera::set_position(const glm::vec3 &pos)
{
	m_position = pos;
	update_near_plane();
}

void camera::set_direction(const glm::vec3 &forward, const glm::vec3 &up)
{
	m_forward = glm::normalize(forward);
	m_left = glm::normalize(glm::cross(up, m_forward));
	m_up = glm::cross(m_forward, m_left);
	update_near_plane();
}

void camera::set_near_plane(float near)
{
	m_near = near;
	update_near_plane();
}

void camera::set_fov(float fov)
{
	m_fov = fov;
	update_near_plane();
}

void camera::set_aspect_ratio(float aspect)
{
	m_aspect = aspect;
	update_near_plane();
}



void camera::update_near_plane()
{
	// Update sizes
	float half_width = std::tan(m_fov / 2) * m_near;
	float half_height = half_width / m_aspect;
	m_near_plane_half_size = {half_width, half_height};

	// Update near plane vectors
	m_near_forward = m_forward * m_near;
	m_near_right = -m_left * m_near_plane_half_size.x;
	m_near_up = m_up * m_near_plane_half_size.y;
}