#include "PixivId.hpp"

#include <charconv>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

#include <PixivClient/Models.hpp>
#include <PixivClient/Singleton.hpp>
#include <PixivClient/PixivClient.hpp>
#include <PluginUtils/Common.hpp>
#include <PluginUtils/StringUtils.hpp>
#include <PluginUtils/NetworkUtils.hpp>
#include <nlohmann/json.hpp>

#include <libmirai/mirai.hpp>
#include <libmirai/Client.hpp>

#include <Core/States/CoolDown.hpp>
#include <Core/Utils/Logger.hpp>

#include <Core/Utils/Common.hpp>
#include <PluginUtils/Base64.hpp>

#include <stduuid/include/uuid.h>
#include <Utils/ImageUtils.hpp>

using std::string;
using json = nlohmann::json;
using namespace std::literals;

namespace Pixiv::PixivId
{

void GetIllustById(const std::vector<string>& tokens, const Mirai::GroupMessageEvent& gm, Bot::Group& group,
                   Bot::Client& client, Utils::BotConfig& config)
{
	//////////////////
	// Parse input
	//////////////////

	if (tokens.size() < 3)
	{
		LOG_INFO(Utils::GetLogger(), "缺少参数[pid] <Pixiv Id>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("不发pid看个锤子图"));
		return;
	}

	PID_t pid;
	{
		int64_t num{};
		if (!Utils::Str2Num(tokens[2], num))
		{
			LOG_INFO(Utils::GetLogger(),
			         "无效参数[pid] <Pixiv Id>: " + tokens[2] + Utils::GetDescription(gm.GetSender(), false));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[2] + "是个锤子pid"));
			return;
		}
		pid = PID_t(num);
	}

	uint64_t page = 1;
	if (tokens.size() > 3)
	{
		if (Utils::toLower(tokens[3]) == "all") 
			page = 0;
		else
		{
			if (!Utils::Str2Num(tokens[3], page))
			{
				LOG_INFO(Utils::GetLogger(),
				         "无效参数(page) <Pixiv Id>: " + tokens[3] + Utils::GetDescription(gm.GetSender(), false));
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(tokens[3] + "是个锤子页码"));
				return;
			}
		}
	}

	//////////////////
	// CD check
	//////////////////

	auto cooldown = group.GetState<State::CoolDown>();
	std::chrono::seconds remaining;
	auto tk = cooldown->GetRemaining("Pixiv", 20s, remaining);
	if (!tk)
	{
		LOG_INFO(Utils::GetLogger(),
		         "冷却剩余<Pixiv>: " + std::to_string(remaining.count())
		             + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(
			gm.GetSender().group.id,
			Mirai::MessageChain().Plain("冷却中捏（剩余: " + std::to_string(remaining.count()) + "s）"));
		return;
	}

	//////////////////
	// Pixiv Api
	//////////////////

	std::shared_ptr<PixivClient> pclient;
	if (config.IsNull("/pixiv/proxy"))
		pclient = GetClient(config.Get("/pixiv/token", ""));
	else
		pclient = GetClient(
			config.Get("/pixiv/token", ""), 
			config.Get("/pixiv/proxy/host", config.Get("/proxy/host", "")), 
			config.Get("/pixiv/proxy/port", config.Get("/proxy/port", -1))
		);
	Illust illust;
	try
	{
		json msg = pclient->GetIllustDetails(pid);
		if (!msg.contains("illust"))
			throw Utils::ParseError("Response does not contain 'illust' field", msg);
		illust = pclient->GetIllustDetails(pid).at("illust").get<Illust>();

	}
	catch(const Utils::NetworkException& e)
	{
		if (e._code != -1)	// NOLINT(*-avoid-magic-numbers)
		{
			json msg = json::parse(e._body, nullptr, false);
			if (!msg.is_discarded() && msg.contains("/error/user_message"_json_pointer))
			{
				std::string str = msg.at("/error/user_message"_json_pointer).get<std::string>();
				if (e._code == 404)	// NOLINT(*-avoid-magic-numbers)
					LOG_INFO(Utils::GetLogger(),"未找到图片<Pixiv Id>" + Utils::GetDescription(gm.GetSender(), false));
				else
					LOG_WARN(Utils::GetLogger(),"Error occured <Pixiv Id>: " + str + " <" + std::to_string(e._code) + ">");
				client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain(str));
				return;
			}
		}

		LOG_WARN(Utils::GetLogger(),"Error occured in pixiv api <Pixiv Id>: " + string(e.what()));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return;
	}
	catch(const std::exception& e)
	{
		LOG_WARN(Utils::GetLogger(),"Error occured in pixiv api <Pixiv Id>: " + string(e.what()));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return;
	}

	auto urls = illust.GetImageUrls();
	string message = "标题: " + illust.title + " (id: " + illust.id.to_string() + ")\n作者: " 
		+ illust.user.name + " (id: " + illust.user.id.to_string() + ")\n标签:";
	constexpr size_t TAGS_MAX = 7;
	size_t count = 0;
	for (const auto& tag : illust.tags)
	{
		if (tag.name.find("users入り") == string::npos)
		{
			message += " #" + tag.name + (tag.translate.empty() ? "" : " (" + tag.translate + ")") + "  ";
			count++;
		}
		if (count >= TAGS_MAX)
		{
			message += " ...";
			break;
		}
	}
	message += "\n👀 " + std::to_string(illust.TotalViews)  + "   🧡 " + std::to_string(illust.TotalBookmarks);

	//////////////////
	// Download illust
	//////////////////

	if (page != 0)
	{
		if (page > urls.size())
			page = urls.size() - 1;
		
		std::string image;
		try
		{
			image = pclient->DownloadIllust(urls.at(page - 1));
		}
		catch(const std::exception& e)
		{
			LOG_WARN(Utils::GetLogger(),"Error occured while downloading image <Pixiv Id>: " + string(e.what()));
			client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return;
		}

		if (illust.PageCount > 1)
		{
			message += "\n页码: " + std::to_string(page) + "/" + std::to_string(illust.PageCount) + "\n";
		}

		auto msg = Mirai::MessageChain().Plain(std::move(message));
		if (illust.x_restrict == X_RESTRICT::SAFE)
		{
			msg += Mirai::ImageMessage({}, {}, {}, Utils::b64encode(image));
		}
		else
		{
			std::filesystem::path cover_path = config.Get("/path/MediaFiles", "MediaFiles")
							/ std::filesystem::path("images/forbidden.png");
			std::string cover;
			{
				std::ifstream ifile(cover_path);

				constexpr size_t BUFFER_SIZE = 4096;
				char buffer[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)
				while (ifile.read(buffer, sizeof(buffer)))
					cover.append(buffer, sizeof(buffer));
				cover.append(buffer, ifile.gcount());
			}
			
			size_t len{};

			constexpr double R18_RATIO = 0.05, R18G_RATIO = 0.15;
			double sigma = (illust.x_restrict == X_RESTRICT::R18 ? R18_RATIO : R18G_RATIO) 
					* std::max(illust.width, illust.height);

			auto out = ImageUtils::CensorImage(image, sigma, len, cover);
			msg += Mirai::ImageMessage({}, {}, {}, Utils::b64encode(out.get(), len));
		}
		
		LOG_INFO(Utils::GetLogger(), "上传结果 <Pixiv Id>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, msg);
	}
	else
	{
		// Send all

		message += "\n总页数: " + std::to_string(illust.PageCount);

		Mirai::ForwardMessage msg;

		Mirai::ForwardMessage::Node node;
		node.SetSenderId(client->GetBotQQ());
		node.SetTimestamp(std::time(nullptr));
		node.SetSenderName("pixiv");
		node.SetMessageChain(Mirai::MessageChain().Plain(std::move(message)));
		msg.emplace_back(node);

		constexpr size_t MAX_BYTES_MEMORY = 1024 * 1024 * 10;
		size_t bytes_count = 0;
		uuids::basic_uuid_random_generator rng(Utils::GetRngEngine());
		
		if (illust.x_restrict == X_RESTRICT::SAFE)
		{
			for (size_t i = 0; i < urls.size(); i++)
			{
				const std::string& url = urls[i];
				LOG_INFO(Utils::GetLogger(), "Downloading " + std::to_string(i + 1) + "/" + std::to_string(urls.size()));

				node.SetTimestamp(std::time(nullptr));
				if (bytes_count > MAX_BYTES_MEMORY)
				{
					std::string tmpfile = uuids::to_string(rng());
					std::filesystem::path tmp_path = config.Get("/path/MediaFiles", "MediaFiles")
								/ std::filesystem::path("tmp") / tmpfile;
					std::ofstream ofile(tmp_path);
					try
					{
						pclient->DownloadIllust(url, 
						[&ofile](const char* data, size_t len)
						{
							ofile.write(data, len);		// NOLINT(*-narrowing-conversions)
							return true;
						}
						);
					}
					catch(const std::exception& e)
					{
						LOG_WARN(Utils::GetLogger(),"Error occured while downloading image <Pixiv Id>: " + string(e.what()));
						client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
						return;
					}
					node.SetMessageChain(Mirai::MessageChain().Image("", "", tmp_path, ""));
				}
				else
				{	
					std::string image;
					try
					{
						image = pclient->DownloadIllust(url);
					}
					catch(const std::exception& e)
					{
						LOG_WARN(Utils::GetLogger(),"Error occured while downloading image <Pixiv Id>: " + string(e.what()));
						client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
						return;
					}
					bytes_count += image.size();
					node.SetMessageChain(Mirai::MessageChain().Image("", "", "", Utils::b64encode(image)));
				}
				msg.emplace_back(node);
			}
		}
		else
		{
			
			std::filesystem::path cover_path = config.Get("/path/MediaFiles", "MediaFiles")
							/ std::filesystem::path("images/forbidden.png");
			std::string cover;
			{
				std::ifstream ifile(cover_path);

				constexpr size_t BUFFER_SIZE = 4096;
				char buffer[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)
				while (ifile.read(buffer, sizeof(buffer)))
					cover.append(buffer, sizeof(buffer));
				cover.append(buffer, ifile.gcount());
			}

			for (size_t i = 0; i < urls.size(); i++)
			{
				const std::string& url = urls[i];
				LOG_INFO(Utils::GetLogger(), "Downloading " + std::to_string(i + 1) + "/" + std::to_string(urls.size()));

				std::string image;
				node.SetTimestamp(std::time(nullptr));
				try
				{
					image = pclient->DownloadIllust(url);
				}
				catch(const std::exception& e)
				{
					LOG_WARN(Utils::GetLogger(),"Error occured while downloading image <Pixiv Id>: " + string(e.what()));
					client.SendGroupMessage(group.gid, Mirai::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
					return;
				}

				constexpr double R18_RATIO = 0.05, R18G_RATIO = 0.15;
				double sigma = (illust.x_restrict == X_RESTRICT::R18 ? R18_RATIO : R18G_RATIO) 
						* std::max(illust.width, illust.height);

				size_t len{};
				auto out = ImageUtils::CensorImage(image, sigma, len, cover);

				if (bytes_count > MAX_BYTES_MEMORY)
				{
					std::string tmpfile = uuids::to_string(rng());
					std::filesystem::path tmp_path = config.Get("/path/MediaFiles", "MediaFiles")
								/ std::filesystem::path("tmp") / tmpfile;
					std::ofstream ofile(tmp_path);

					ofile.write(reinterpret_cast<char *>(out.get()), len);	// NOLINT
					node.SetMessageChain(Mirai::MessageChain().Image("", "", tmp_path, ""));
				}
				else
				{
					bytes_count += len;
					node.SetMessageChain(Mirai::MessageChain().Image("", "", "", Utils::b64encode(out.get(), len)));
				}
				msg.emplace_back(node);
			}
		}
	
		LOG_INFO(Utils::GetLogger(), "上传结果 <Pixiv Id>" + Utils::GetDescription(gm.GetSender(), false));
		client.SendGroupMessage(group.gid, Mirai::MessageChain().Forward(std::move(msg)));
	}
}

} // namespace Pixiv::PixivId