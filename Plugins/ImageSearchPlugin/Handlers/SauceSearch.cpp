#include "SauceSearch.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <string>
#include <vips/vips8>

#include <PluginUtils/Base64.hpp>
#include <PluginUtils/Common.hpp>
#include <PluginUtils/NetworkUtils.hpp>
#include <PluginUtils/UrlComponents.hpp>
#include <SauceNAO/Models.hpp>
#include <SauceNAO/SauceClient.hpp>
#include <httplib.h>

#include <Core/States/CoolDown.hpp>
#include "libmirai/Types/MediaTypes.hpp"

using std::string;

namespace SearchHandlers
{

namespace
{

auto CensorImage(const std::string& image, double sigma, size_t& len, const std::string& cover = {})
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

// string CropAndConvert(const std::string& image)
// {
// 	constexpr int THUMBNAIL_SIZE = 1500;
// 	using namespace vips;
// 	VImage in = VImage::thumbnail_buffer(vips_blob_new(nullptr, image.data(), image.size()), THUMBNAIL_SIZE, 
// 		VImage::option()
// 		->set("size", VipsSize::VIPS_SIZE_DOWN)
// 		->set("no_rotate", true)
// 	);

// 	char* buffer = nullptr;
// 	size_t len{};
// 	// NOLINTNEXTLINE(*-reinterpret-cast)
// 	in.write_to_buffer(".png", reinterpret_cast<void**>(&buffer), &len);
// 	auto p = std::unique_ptr<char, std::function<void(char*)>>
// 		(buffer, [](auto* p) { VIPS_FREE(p); });
// 	return {p.get(), len};
// }

} // namespace

void SearchSauce(std::string url, const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                 Utils::BotConfig& config)
{
	//////////////////
	// CD check
	//////////////////

	using namespace std::literals;

	auto cooldown = group.GetState<State::CoolDown>();
	std::chrono::seconds remaining;
	auto tk = cooldown->GetRemaining("ImageSearch", 20s, remaining);
	if (!tk)
	{
		LOG_INFO(Utils::GetLogger(),
		         "冷却剩余<SearchSauce>: " + std::to_string(remaining.count())
		             + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(
			gm.GetSender().group.id,
			Mirai::MessageChain().Plain("冷却中捏（剩余: " + std::to_string(remaining.count()) + "s）"));
		return;
	}

	/////////////////////
	// Download Target Img
	/////////////////////

	string target;
	try
	{
		Utils::UrlComponent comp = Utils::UrlComponent::ParseUrl(url);
		httplib::Client cli(comp.GetOrigin());
		cli.set_decompress(true);
		cli.set_connection_timeout(300); // NOLINT(*-avoid-magic-numbers)
		cli.set_read_timeout(300);       // NOLINT(*-avoid-magic-numbers)
		cli.set_write_timeout(120);      // NOLINT(*-avoid-magic-numbers)

		auto result = cli.Get(
			comp.GetRelativeRef(),
			{	
				{"Accept", "*/*"},
				{"Accept-Encoding", "gzip, deflate"},
				{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}
			}
		);
		if (!Utils::VerifyResponse(result)) throw Utils::NetworkException(result);

		LOG_INFO(Utils::GetLogger(), "Finish downloading target <SearchSauce>");

		target = std::move(result->body);
		// target = CropAndConvert(target);
	}
	catch (const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(), "Error occured when downloading target image <SearchSauce>: " + string(e.what()));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return;
	}

	//////////////////
	// SauceNAO Api
	//////////////////

	using namespace SauceNAO;

	SauceClient cli = [&]()
	{
		if (config.IsNull("/saucenao/proxy")) return SauceClient(config.Get("/saucenao/token", ""), {});
		else
			return SauceClient(config.Get("/saucenao/token", ""), {},
			                   config.Get("/saucenao/proxy/host", config.Get("/proxy/host", "")),
			                   config.Get("/saucenao/proxy/port", config.Get("/proxy/port", -1)));
	}();

	SauceNAOResult result;
	try
	{
		constexpr std::array<uint64_t, 3> PRIORITY_LIST = {PIXIV, PIXIV_HISTORY, TWITTER};
		constexpr double PRIORITY_COMPENSATE = 5;
		
		result = cli.SearchFile(std::move(target), std::move(url));
		std::sort(result.results.begin(), result.results.end(),
		          [&](const SauceNAOResult::Result& a, const SauceNAOResult::Result& b) -> bool
		          {
					  auto sim_a = std::stod(a.header.similarity);
					  auto sim_b = std::stod(b.header.similarity);

					  for (const auto& mask : PRIORITY_LIST)
					  {
						  if (a.header.index == Mask2Index(mask)) sim_a += PRIORITY_COMPENSATE;
						  if (b.header.index == Mask2Index(mask)) sim_b += PRIORITY_COMPENSATE;
					  }
					  return sim_a > sim_b;
				  });

		LOG_INFO(Utils::GetLogger(),
		         "Remaining searches <SearchSauce>: " + std::to_string(result.header.ShortRemaining) + " | "
		             + std::to_string(result.header.LongRemaining));

		LOG_DEBUG(
			Utils::GetLogger(),
			"Results <SearchSauce>: " + [&]() -> string
			{
				string str;
				bool first = true;
				for (const auto& p : result.results)
				{
					if (!first) str += ", ";
					first = false;
					str += "[index: " + p.header.IndexName;
					str += ", similarity: " + p.header.similarity;
					str += ", hidden: ";
					str += (p.header.hidden ? "true" : "false");
					str += ", <title='" + p.data.title;
					str += "', author='" + p.data.author;
					str += "'>]";
				}
				return str;
			}());

		constexpr double MIN_SIMILARITY = SauceClient::SearchOptions{}.MinSimilarity - PRIORITY_COMPENSATE;
		if (result.results.empty() || std::stod(result.results[0].header.similarity) < MIN_SIMILARITY)
		{
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("找不到相关图片捏"));
			return;
		}
	}
	catch (const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(), "Error occured in SauceNAO API <SearchSauce>: " + string(e.what()));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return;
	}

	/////////////////////
	// Download Thumbnail
	/////////////////////

	auto best_sauce = result.results[0];
	Mirai::GroupImage thumbnail;
	try
	{
		Utils::UrlComponent comp = Utils::UrlComponent::ParseUrl(best_sauce.header.thumbnail);
		httplib::Client cli(comp.GetOrigin());
		cli.set_decompress(true);
		cli.set_connection_timeout(300); // NOLINT(*-avoid-magic-numbers)
		cli.set_read_timeout(300);       // NOLINT(*-avoid-magic-numbers)
		cli.set_write_timeout(120);      // NOLINT(*-avoid-magic-numbers)

		auto result = cli.Get(
			comp.GetRelativeRef(),
			{{"Accept", "*/*"},
		     {"Accept-Encoding", "gzip, deflate"},
		     {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		if (!Utils::VerifyResponse(result)) throw Utils::NetworkException(result);

		LOG_INFO(Utils::GetLogger(), "Finish downloading thumbnail <SearchSauce>");

		if (best_sauce.header.hidden)
		{
			size_t len{};
			auto out = CensorImage(result->body, 0.04 * 150, len); // NOLINT(*-avoid-magic-numbers)
			string{}.swap(result->body);
			thumbnail = client->UploadGroupImage({out.get(), len});
		}
		else
			thumbnail = client->UploadGroupImage(std::move(result->body));
	}
	catch (const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(), "Error occured when downloading thumbnails <SearchSauce>: " + string(e.what()));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return;
	}

	string msg;
	msg += "标题: " + best_sauce.data.title + "\n";
	msg += "作者: " + best_sauce.data.author + "\n";
	msg += "网址: " + best_sauce.data.url + "\n";
	for (const auto& [key, value] : best_sauce.data.extras)
		msg += key + ": " + value + "\n";
	msg += "相似度: " + best_sauce.header.similarity + "\n";
	LOG_INFO(Utils::GetLogger(), "上传结果 <SearchSauce>" + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(group.gid, Mirai::MessageChain()
		.Plain(std::move(msg))
		.Image(std::move(thumbnail))
	);
}

} // namespace SearchHandlers