#include "denoise.hpp"
#include <stdexcept>
#include <string>

#ifdef WITH_OIDN

#include <OpenImageDenoise/oidn.hpp>

rt::hdr_image rt::denoise_hdr_image(const rt::hdr_image &src, bool is_hdr)
{
	rt::hdr_image img(src);
	void *ptr = &img.get_data()[0];
	int width = img.get_width();
	int height = img.get_height();

	oidn::DeviceRef device = oidn::newDevice();
	device.commit();

	oidn::FilterRef filter = device.newFilter("RT");
	filter.setImage("color", ptr, oidn::Format::Float3, width, height);
	filter.setImage("output", ptr, oidn::Format::Float3, width, height);
	filter.set("hdr", is_hdr);
	filter.commit();
	filter.execute();

	const char *err;
	if (device.getError(err) != oidn::Error::None)
		throw std::runtime_error(std::string{"OIDN error: "} + err);
	
	return img;
}

#else

rt::hdr_image rt::denoise_hdr_image(const rt::hdr_image &src, bool is_hdr)
{
	throw std::runtime_error("Build with WITH_OIDN to use denoiser!");
}

#endif 

rt::hdr_image rt::denoise_hdr_image(const rt::sampled_hdr_image &src, bool is_hdr)
{
	return rt::denoise_hdr_image(rt::hdr_image(src), is_hdr);
}