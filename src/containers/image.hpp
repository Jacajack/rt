#pragma once 

#include <vector>
#include <cinttypes>
#include <functional>
#include <stdexcept>
#include <glm/glm.hpp>

namespace rt {

/**
	2D image built of pixels of type T
*/
template <typename T>
class image
{
public:
	/**
		"Tonemapping" constructor
	*/
	template <typename U>
	image(const image<U> &src, std::function<T(const U&)> conv) :
		m_width(src.m_width),
		m_height(src.m_height)
	{
		m_data.reserve(m_width * m_height);
		std::transform(
			src.m_data.begin(), src.m_data.end(),
			std::back_inserter(m_data),
			conv
			);
	}

	/**
		Type-converting constructor (if T is constructible from U)
	*/
	template <typename U>
	explicit image(const image<U> &src) :
		m_width(src.m_width),
		m_height(src.m_height)
	{
		m_data.reserve(m_width * m_height);
		for (const auto &p : src.m_data)
			m_data.emplace_back(p);
	}

	/**
		Constructs empty image
	*/
	image(int w, int h) :
		m_width(w),
		m_height(h)	
	{
		m_data.reserve(w * h);
	}

	/**
		Direct access to the data
	*/
	const std::vector<T> &get_data() const
	{
		return m_data;
	}

	/**
		Directly accesses image data
	*/
	const T &operator[](int index) const
	{
		return m_data[index];
	}

	/**
		Directly accesses image data (with bound check)
	*/
	const T &at(int index) const
	{
		return m_data.at(index);
	}

	/**
		Pixel access by coordinates
	*/
	T &pixel(int x, int y)
	{
		return m_data[y * m_width + x];
	}

	/**
		Pixel access by coordinates
	*/
	const T &pixel(int x, int y) const
	{
		return m_data[y * m_width + x];
	}

	/**
		Pixel access by coordinates (with bound check)
	*/
	T &at_pixel(int x, int y)
	{
		return m_data.at(y * m_width + x);
	}

	/**
		Pixel access by coordinates (with bound check)
	*/
	const T &at_pixel(int x, int y) const
	{
		return m_data.at(y * m_width + x);
	}

	/**
		Returns width
	*/
	int get_width() const
	{
		return m_width;
	}
	
	/**
		Returns height
	*/
	int get_height() const
	{
		return m_height;
	}

	/**
		Returns dimensions as a vector of two ints
	*/
	glm::ivec2 get_dimensions() const
	{
		return {m_width, m_height};
	}

	/**
		Returns size in pixels
	*/
	size_t size() const
	{
		return m_width * m_height;
	}

	/**
		Adds another image to this one
	*/
	image<T> &operator+=(const image<T> &rhs)
	{
		if (get_dimensions() != rhs.get_dimensions())
			throw std::runtime_error("Cannot add rt::images with different dimensions");
	
		for (int i = 0; i < size(); i++)
			m_data[i] = rhs.m_data[i];
	}

private:
	int m_width;
	int m_height;
	std::vector<T> m_data;
};

/**
	Adds two images of different types
*/
template <typename T, typename U>
image<T> operator+(const image<T> &lhs, const image<U> &rhs)
{
	if (lhs.get_dimensions() != rhs.get_dimensions())
		throw std::runtime_error("Cannot add rt::images with different dimensions");
	
	// LHS copy
	image<T> res(lhs);
	
	// Convert U image to T image
	image<T> rhs_t(rhs);

	for (int i = 0; i < res.size(); i++)
		res[i] = rhs_t[i];

	return res;
}

/**
	Adds two images of the same type
*/
template <typename T>
image<T> operator+(const image<T> &lhs, const image<T> &rhs)
{
	if (lhs.get_dimensions() != rhs.get_dimensions())
		throw std::runtime_error("Cannot add rt::images with different dimensions");
	
	// LHS copy
	image<T> res(lhs);

	for (int i = 0; i < res.size(); i++)
		res[i] = rhs[i];

	return res;
}

/**
	Pixel of a non-uniformly sampled image. HDR data + numer of samples
*/
struct sampled_pixel
{
	glm::vec3 data;
	int sample_count;
};

/**
	8-bit RGB pixel
*/
struct rgb_pixel
{
	/**
		Default tonemapping - Reinhard + Gamma correction
	*/
	rgb_pixel(const sampled_pixel &p)
	{
		auto pix = p.data / static_cast<float>(p.sample_count);
		pix = pix / (pix + 1.f);
		pix = glm::pow(pix, glm::vec3{1.f / 2.2f});
		r = pix.r * 255.99f;
		g = pix.g * 255.99f;
		b = pix.b * 255.99f;
	}

	std::uint8_t r, g, b;
};

/**
	8-bit RGBA pixel
*/
struct rgba_pixel
{
	/**
		Default tonemapping - Reinhard + Gamma correction
	*/
	rgba_pixel(const sampled_pixel &p)
	{
		auto pix = p.data / static_cast<float>(p.sample_count);
		pix = pix / (pix + 1.f);
		pix = glm::pow(pix, glm::vec3{1.f / 2.2f});
		r = pix.r * 255.99f;
		g = pix.g * 255.99f;
		b = pix.b * 255.99f;
		a = 255;
	}

	rgba_pixel(const rgb_pixel &p) :
		r(p.r),
		g(p.g),
		b(p.b),
		a(255)
	{}

	std::uint8_t r, g, b, a;
};


using rgb_image = image<rgb_pixel>;
using rgba_image = image<rgba_pixel>;
using hdr_image = image<glm::vec3>;
using sampled_image = image<sampled_pixel>;

}