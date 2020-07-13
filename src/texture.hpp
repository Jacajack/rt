#pragma once

#include "containers/image.hpp"

namespace rt {

static struct tex_nearest_tag_t {} tex_nearest_tag;
static struct tex_linear_tag_t {} tex_linear_tag;

/**
	Texture wrapping modes
*/
enum class tex_wrap
{
	CLAMP,
	REPEAT
};

/**
	texture = image + sampling
*/
template <typename T>
class texture
{
public:
	texture(int width, int height) :
		m_image(width, height),
		m_texel_size(1.f / width, 1.f / height)
	{}

	texture(const rt::image<T> &im) :
		m_image(im),
		m_texel_size(1.f / im.get_dimensions())
	{}

	const rt::image<T> &get_image() const
	{
		return m_image;
	}

	rt::image<T> &get_image()
	{
		return m_image;
	}

	T &uv(glm::vec2 pos, tex_nearest_tag_t)
	{
		pos = wrap_uv(pos);
		return m_image.pixel(pos / m_texel_size);
	}

	const T &uv(glm::vec2 pos, tex_nearest_tag_t) const
	{
		pos = wrap_uv(pos);
		return m_image.pixel(pos / m_texel_size);
	}

	T uv(const glm::vec2 &pos) const
	{
		return uv(pos, tex_nearest_tag);
	}	

private:
	glm::vec2 wrap_uv(const glm::vec2 &pos)
	{
		if (m_wrap == tex_wrap::CLAMP)
			return glm::clamp(pos, 0.f, 1.f);
		else if (m_wrap == tex_wrap::REPEAT)
			return glm::mod(pos, 1.f);
	}

	rt::image<T> m_image;
	tex_wrap m_wrap = tex_wrap::CLAMP;
	glm::vec2 m_texel_size;
};

}