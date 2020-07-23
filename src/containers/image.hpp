#pragma once 

#include <vector>
#include <cinttypes>
#include <functional>
#include <stdexcept>
#include <glm/glm.hpp>

namespace rt {

// A forward declaration
template <typename T> class sampled_image;

/**
	High dynamic range pixel
*/
using hdr_pixel = glm::vec3;

/**
	8-bit RGB pixel
*/
struct rgb_pixel
{
	/**
		No tonemapping by default
	*/
	rgb_pixel(hdr_pixel p)
	{
		r = glm::clamp(p.r * 255.99f, 0.f, 255.f);
		g = glm::clamp(p.g * 255.99f, 0.f, 255.f);
		b = glm::clamp(p.b * 255.99f, 0.f, 255.f);
	}

	std::uint8_t r, g, b;
} __attribute__((packed));

/**
	8-bit RGBA pixel
*/
struct rgba_pixel
{
	/**
		No tonemapping by default
	*/
	rgba_pixel(hdr_pixel p)
	{
		r = glm::clamp(p.r * 255.99f, 0.f, 255.f);
		g = glm::clamp(p.g * 255.99f, 0.f, 255.f);
		b = glm::clamp(p.b * 255.99f, 0.f, 255.f);
		a = 255;
	}

	rgba_pixel(const rgb_pixel &p) :
		r(p.r),
		g(p.g),
		b(p.b),
		a(255)
	{}

	std::uint8_t r, g, b, a;
} __attribute__((packed));


/**
	2D image built of pixels of type T

	\todo Fix tonemapping ctors
*/
template <typename T>
class image
{
	template <typename> friend class image;

public:
	image(const image<T> &) = default;
	image<T> &operator=(const image<T> &) = default;

	image(image<T> &&) = default;
	image<T> &operator=(image<T> &&) = default;

	/**
		Image from sampled image
	*/
	template <typename U>
	image(const sampled_image<U> &src);

	/**
		Image from sampled image assignment
	*/
	template <typename U>
	image<T> &operator=(const sampled_image<U> &src);

	/**
		Tonemapping constructor for sampled images
	*/
	template <typename U>
	image(const sampled_image<U> &src, std::function<T(const U&)> conv);

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
		Type-converting assignment operator (if T is constructible from U)
	*/
	template <typename U>
	image<T> &operator=(const image<U> &src)
	{
		m_width = src.m_width;
		m_height = src.m_height;
		m_data.reserve(m_width * m_height);
		for (const auto &p : src.m_data)
			m_data.emplace_back(p);

		return *this;
	}

	/**
		Constructs empty image
	*/
	image(int w, int h) :
		m_width(w),
		m_height(h),
		m_data(w * h)
	{
	}

	/**
		Clears image
	*/
	void clear()
	{
		std::fill(m_data.begin(), m_data.end(), T{});
	}

	/**
		Direct access to the data
	*/
	const std::vector<T> &get_data() const
	{
		return m_data;
	}

	/**
		Direct access to the data
	*/
	std::vector<T> &get_data()
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
		Pixel access by coordinates
	*/
	T &pixel(const glm::ivec2 &pos)
	{
		return m_data[pos.y * m_width + pos.x];
	}

	/**
		Pixel access by coordinates
	*/
	const T &pixel(const glm::ivec2 &pos) const
	{
		return m_data[pos.y * m_width + pos.x];
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
	
		for (unsigned int i = 0; i < size(); i++)
			m_data[i] += rhs.m_data[i];

		return *this;
	}

	typename std::vector<T>::iterator begin()
	{
		return m_data.begin();
	}

	typename std::vector<T>::iterator end()
	{
		return m_data.end();
	}

	typename std::vector<T>::const_iterator cbegin() const
	{
		return m_data.cbegin();
	}

	typename std::vector<T>::const_iterator cend() const
	{
		return m_data.cend();
	}

private:
	int m_width;
	int m_height;
	std::vector<T> m_data;
};

/**
	Adds two images
*/
template <typename T, typename U>
image<T> operator+(const image<T> &lhs, const image<U> &rhs)
{
	// LHS copy and convert U image to T image
	image<T> res(lhs);
	image<T> rhs_t(rhs);
	res += rhs_t;
	return res;
}

/**
	Like `image` but also stores number of samples (per entire image)
*/
template <typename T>
class sampled_image : public image<T>
{
	template <typename> friend class sampled_image;

public:
	using image<T>::image;

	void add_sample()
	{
		m_sample_count++;
	}

	void set_sample_count(int n)
	{
		m_sample_count = n;
	}

	int get_sample_count() const
	{
		return m_sample_count;
	}

	void clear()
	{
		m_sample_count = 0;
		image<T>::clear();
	}

	/**
		Adds another image to this one
	*/
	sampled_image<T> &operator+=(const sampled_image<T> &rhs)
	{
		image<T>::operator+=(rhs);
		m_sample_count += rhs.get_sample_count();
		return *this;
	}

private:
	int m_sample_count = 0;
};

/**
	Sampled images are converted to normal images by simply
	dividing each pixel by the number of samples
*/
template <typename T>
template <typename U>
image<T>::image(const sampled_image<U> &src)
{
	image<U> tmp(static_cast<const image<U>&>(src));
	for (auto &p : tmp)
		p /= src.get_sample_count();
	image<T>::operator=(std::move(tmp));
}

/**
	Sampled images are converted to normal images by simply
	dividing each pixel by the number of samples and tonemapping
	with provided function
*/
template <typename T>
template <typename U>
image<T>::image(const sampled_image<U> &src, std::function<T(const U&)> conv)
{
	image<U> tmp(static_cast<const image<U>&>(src));
	for (auto &p : tmp)
		p /= src.get_sample_count();

	m_data.reserve(m_width * m_height);
	std::transform(
		src.m_data.begin(), src.m_data.end(),
		std::back_inserter(m_data),
		conv
		);
}

template <typename T>
template <typename U>
image<T> &image<T>::operator=(const sampled_image<U> &src)
{
	image<U> tmp(static_cast<const image<U>&>(src));
	for (auto &p : tmp)
		p /= src.get_sample_count();
	image<T>::operator=(std::move(tmp));
	return *this;
}

template <typename T, typename U>
sampled_image<T> operator+(const sampled_image<T> &lhs, const sampled_image<U> &rhs)
{
	// LHS copy
	sampled_image<T> res(lhs);
	res += rhs;
	return res;
}

using rgb_image = image<rgb_pixel>;
using rgba_image = image<rgba_pixel>;
using hdr_image = image<glm::vec3>;

using sampled_rgb_image = sampled_image<rgb_pixel>;
using sampled_rgba_image = sampled_image<rgba_pixel>;
using sampled_hdr_image = sampled_image<glm::vec3>;

}