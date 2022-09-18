#ifndef _ELANOR_BOT_HPP_
#define _ELANOR_BOT_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <Utils/PluginManager.hpp>
#include <Utils/Timer.hpp>

#include <libmirai/Events/Events.hpp>
#include <libmirai/Types/BasicTypes.hpp>
#include <libmirai/Utils/SessionConfig.hpp>

#include <Core/Bot/GroupList.hpp>
#include <Core/Client/Client.hpp>
#include <Core/Interface/IGroupCommand.hpp>
#include <Core/Interface/ITrigger.hpp>
#include <Core/Utils/Common.hpp>

namespace Bot
{

class ElanorBot
{
protected:
	mutable std::mutex _MemberMtx;
	mutable std::mutex _NudgeMtx;

	template<typename T> struct Tag
	{
		std::string name{};
		std::unique_ptr<T, void (*)(T*)> data;
		int LibIdx = -1;
	};

	std::vector<Tag<GroupCommand::IGroupCommand>> _GroupCommands{};
	std::vector<Tag<Trigger::ITrigger>> _triggers{};
	std::vector<PluginLibrary> _plugins{};

	GroupList _groups;
	Client _client{};
	Utils::Timer _timer{};
	Utils::BotConfig _config{};

	bool _running = false;

	void _run();
	void _stop();

	void _LoadPlugins(const std::filesystem::path& folder);
	void _OffloadPlugins();

	void _NudgeEventHandler(Mirai::NudgeEvent& e);
	void _GroupMessageEventHandler(Mirai::GroupMessageEvent& gm);
	void _ConnectionOpenedHandler(Mirai::ClientConnectionEstablishedEvent& e);
	void _ConnectionClosedHandler(Mirai::ClientConnectionClosedEvent& e);
	void _ConnectionErrorHandler(Mirai::ClientConnectionErrorEvent& e);
	void _ParseErrorHandler(Mirai::ClientParseErrorEvent& e);

public:
	ElanorBot();
	ElanorBot(const ElanorBot&) = delete;
	ElanorBot& operator=(const ElanorBot&) = delete;
	ElanorBot(ElanorBot&&) = delete;
	ElanorBot& operator=(ElanorBot&&) = delete;

	bool SetConfig(const std::string& filepath)
	{
		return this->_config.FromFile(filepath);
	}

	void Start(const Mirai::SessionConfigs& opts);
	void Stop();

	~ElanorBot() { this->_stop(); }
};

} // namespace Bot

#endif