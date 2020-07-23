#pragma once

#include "containers/image.hpp"

namespace rt {

extern rt::hdr_image denoise_hdr_image(const rt::hdr_image &src, bool is_hdr = false);
extern rt::hdr_image denoise_hdr_image(const rt::sampled_hdr_image &src, bool is_hdr = false);

}