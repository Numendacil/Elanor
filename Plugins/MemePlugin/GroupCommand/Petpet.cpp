#include "Petpet.hpp"

#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>
#include <charconv>

#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>

#include <libmirai/mirai.hpp>

#include <Core/Bot/Group.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/Utils/Logger.hpp>
#include <PluginUtils/NetworkUtils.hpp>

#include <PluginUtils/Base64.hpp>
#include <httplib.h>

#include <vips/vips8>

using json = nlohmann::json;
using std::string;
using std::vector;

namespace GroupCommand
{

namespace
{

auto GeneratePetpet(const std::string& avatar, std::filesystem::path folder, size_t& len)
{
	using namespace vips;

	constexpr int FRAME = 5;
	constexpr int SIZE = 112;
	constexpr int DELAY = 60;

	std::vector<VImage> image(FRAME);
	VImage source = VImage::new_from_buffer(avatar.data(), avatar.size(), nullptr);

	for (int i = 0 ; i < FRAME; i++)
	{
		VImage hand = VImage::new_from_file((folder / ("pet" + std::to_string(i) + ".gif")).c_str()).premultiply();
		hand = hand.resize(SIZE / hand.height());
		hand.unpremultiply();

		constexpr double SQUEEZE_CO = 1.5;
		double squeeze  = (i < std::floor(FRAME / 2)) ? std::pow(i, SQUEEZE_CO) : std::pow((FRAME - i), SQUEEZE_CO);
		double width = 0.9 + squeeze * 0.05;	// NOLINT(*-avoid-magic-numbers)
		double height = 0.98 - squeeze * 0.1;	// NOLINT(*-avoid-magic-numbers)
		double offsetX = (1 - width) * 0.5 + 0.01;	// NOLINT(*-avoid-magic-numbers)
		double offsetY = (1 - height) + 0.05;	// NOLINT(*-avoid-magic-numbers)
		
		VImage squeezed = source.resize(
			width * SIZE / source.width(),
			VImage::option()
			->set("vscale", height * SIZE / source.height())
		);

		image.at(i) = hand.new_from_image({0, 0, 0, 0})
		.composite2(squeezed, VipsBlendMode::VIPS_BLEND_MODE_OVER,
			VImage::option()
			->set("x", offsetX * SIZE)
			->set("y", offsetY * SIZE)
		)
		.composite2(hand, VipsBlendMode::VIPS_BLEND_MODE_OVER);
	}

	VImage result = VImage::arrayjoin(image, VImage::option()->set("across", 1)).cast(VipsBandFormat::VIPS_FORMAT_UCHAR).copy();
	result.set("delay", std::vector<int>(FRAME, DELAY));
	result.set("page-height", SIZE);

	char* buffer = nullptr;
	// NOLINTNEXTLINE(*-reinterpret-cast)
	result.write_to_buffer(".gif", reinterpret_cast<void**>(&buffer), &len,
		VImage::option()
		->set("format", "gif")
		->set("optimize_gif_transparency", true)
		->set("optimize_gif_frames", true)
	);
	return std::unique_ptr<char, std::function<void(char*)>>(buffer, [](auto* p) { VIPS_FREE(p); });
}

}

bool Petpet::Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, Bot::Client& client,
                     Utils::BotConfig& config)
{
	string str = Utils::ReplaceMark(Utils::GetText(gm.GetMessage()));
	if (!Utils::trim(str).empty() && Utils::trim(str)[0] != '#') return false;

	vector<string> tokens;
	if (Utils::Tokenize(str, tokens) < 1) return false;

	string command = Utils::toLower(tokens[0]);
	if (command != "#pet" && command !=  "#petpet" && command != "#摸摸") return false;


	LOG_INFO(Utils::GetLogger(), "Calling Petpet <Petpet>" + Utils::GetDescription(gm.GetSender()));

	if (!Utils::CheckAuth(gm.GetSender(), group, this->Permission()))
	{
		LOG_INFO(Utils::GetLogger(), "权限不足 <Petpet>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("权限不足捏～"));
		return true;
	}

	Mirai::QQ_t target;

	if (tokens.size() > 1)
	{
		string arg = Utils::toLower(tokens[1]);
		if (arg == "help" || arg == "h" || arg == "帮助")
		{
			LOG_INFO(Utils::GetLogger(), "帮助文档 <Petpet>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("usage:\n#petpet [QQ]"));
			return true;
		}
		else if (arg.empty())
		{
			LOG_INFO(Utils::GetLogger(), "参数为空 <Petpet>" + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("QQ号看不见捏，怎么会事捏"));
			return true;
		}

		int64_t id{};
		if (!Utils::Str2Num(tokens[1], id))
		{
			LOG_INFO(Utils::GetLogger(),
					"无效参数[QQ] <Petpet>: " + tokens[1] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[1] + "是个锤子QQ号"));
			return true;
		}
		target = (Mirai::QQ_t)id;
	}
	else
	{
		auto AtMsg = gm.GetMessage().GetAll<Mirai::AtMessage>();
		if (!AtMsg.empty())
			target = AtMsg[0].GetTarget();
	}

	if (target == Mirai::QQ_t{})
	{
		LOG_INFO(Utils::GetLogger(),"缺少target <Petpet>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("你搁这摸空气呢"));
		return true;
	}

	httplib::Client cli("http://q1.qlogo.cn");
	cli.set_compress(true);
	cli.set_decompress(true);
	cli.set_connection_timeout(10); // NOLINT(*-avoid-magic-numbers)
	cli.set_read_timeout(300);       // NOLINT(*-avoid-magic-numbers)
	cli.set_write_timeout(10);      // NOLINT(*-avoid-magic-numbers)

	auto result = cli.Get("/g?b=qq&nk=" + target.to_string() + "&s=640");
	if (!Utils::VerifyResponse(result))
	{
		LOG_WARN(Utils::GetLogger(),"Error occured while downloading image <Petpet>: " + 
			((result)? 
			"Reason: " + result->reason + ", Body: " + result->body + " <" + std::to_string(result->status) + ">"
			: "Reason: " + httplib::to_string(result.error()) + " <-1>")
		);
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return true;
	}

	size_t len{};
	auto out = GeneratePetpet(
		result->body, 
		config.Get("/path/MediaFiles", "MediaFiles") / std::filesystem::path("images/petpet"), 
		len
	);
	
	LOG_INFO(Utils::GetLogger(), "上传图片 <Petpet>" + Utils::GetDescription(gm.GetSender(), false));
	client.SendGroupMessage(group.gid, Mirai::MessageChain().Image(
		client->UploadGroupImage({out.get(), len})
	));
	return true;
}

}