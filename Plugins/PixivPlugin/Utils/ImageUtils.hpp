#ifndef _PIXIV_FUNCTIONS_IMAGE_UTILS_HPP_
#define _PIXIV_FUNCTIONS_IMAGE_UTILS_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vips/vips8>

namespace Pixiv::ImageUtils
{

inline auto CensorImage(const std::string& image, double sigma, size_t& len, const std::string& cover = {})
{
	using namespace vips;
	VImage in = VImage::new_from_buffer(image.data(), image.size(), nullptr);
	in = in.gaussblur(sigma);
	if (!cover.empty())
	{
		VImage top = VImage::new_from_buffer(cover.data(), cover.size(), nullptr);
		constexpr double TOP_RATIO = 0.7;
		double scale = TOP_RATIO * std::min(in.width() / (double)top.width(), in.height() / (double)top.height());
		top = top.resize(scale);
		in = in.cast(top.format());
		in = in.composite2(
			top, VIPS_BLEND_MODE_OVER,
			VImage::option()->set("x", (in.width() - top.width()) / 2)->set("y", (in.height() - top.height()) / 2));
	}

	char* buffer = nullptr;
	// NOLINTNEXTLINE(*-reinterpret-cast)
	in.write_to_buffer(".jpg", reinterpret_cast<void**>(&buffer), &len);
	return std::unique_ptr<char, std::function<void(char*)>>(buffer, [](auto* p) { VIPS_FREE(p); });
}

constexpr size_t THUMBNAIL_SIZE = 960;

std::string CropAndConvert(const std::string& image)
{
	using namespace vips;
	VImage in = VImage::thumbnail_buffer(vips_blob_new(nullptr, image.data(), image.size()), THUMBNAIL_SIZE, 
		VImage::option()
		->set("size", VipsSize::VIPS_SIZE_DOWN)
		->set("no_rotate", true)
	);

	char* buffer = nullptr;
	size_t len{};
	// NOLINTNEXTLINE(*-reinterpret-cast)
	in.write_to_buffer(".jpg", reinterpret_cast<void**>(&buffer), &len);
	auto p = std::unique_ptr<char, std::function<void(char*)>>
		(buffer, [](auto* p) { VIPS_FREE(p); });
	return {p.get(), len};
}

} // namespace Pixiv::ImageUtils

#endif