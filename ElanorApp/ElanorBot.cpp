#include <algorithm>
#include <cassert>
#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>
#include <string>

#include <libmirai/mirai.hpp>
#include <Core/Utils/Logger.hpp>
#include <Core/Utils/Common.hpp>
#include <Core/States/States.hpp>

#include <Core/Interface/PluginEntry.hpp>
#include <Core/Interface/IGroupCommand.hpp>
#include <Core/Interface/ITrigger.hpp>
#include "ElanorBot.hpp"
#include "libmirai/Types/BasicTypes.hpp"
#include "libmirai/Utils/Logger.hpp"
#include <Utils/PluginManager.hpp>

using std::string;
using std::vector;
using std::pair;
using std::unique_ptr;


namespace Bot
{

using namespace std::literals;

ElanorBot::ElanorBot() = default;

void ElanorBot::_run()
{
	if (this->_running) return;
	assert(!this->_client.isRunning());
	this->_client.Resume();
	this->_running = true;
}

void ElanorBot::_stop()
{
	if (!this->_running) return;
	assert(this->_client.isRunning());
	this->_client.Pause();
	this->_running = false;
}

void ElanorBot::_LoadPlugins(const std::filesystem::path& folder)
{
	for (const auto& entry : std::filesystem::directory_iterator(folder))
	{
		if (entry.is_regular_file())
		{
			try
			{
				PluginLibrary lib;
				lib.Open(entry.path());
				this->_plugins.emplace_back(std::move(lib));
			}
			catch(const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(), e.what());
			}
		}
	}

	for (int i = 0; i < this->_plugins.size(); i++)
	{
		const auto& plugin = this->_plugins.at(i);
		API* api_table = static_cast<API*>(plugin.GetSym("api_table"));

		api_table->InitPlugin();
		LOG_INFO(Utils::GetLogger(), 
			"Loaded plugin "s
			+ api_table->GetPluginName()
			+ ": "
			+ api_table->GetPluginInfo());

		int command_count = api_table->GetGroupCommandCount();
		int trigger_count = api_table->GetTriggerCount();
		LOG_INFO(Utils::GetLogger(), 
				"Found "s 
				+ std::to_string(command_count)
				+ " Group Commands, "
				+ std::to_string(trigger_count)
				+ " Triggers");

		auto command_deleter = api_table->DeleteGroupCommand;
		auto trigger_deleter = api_table->DeleteTrigger;

		for (int idx = 0; idx < command_count; idx++)
		{
			this->_GroupCommands.emplace_back(
				api_table->GetGroupCommandName(idx),
				std::move(
					std::unique_ptr<GroupCommand::IGroupCommand, void(*)(GroupCommand::IGroupCommand*)>
					{
						api_table->GetGroupCommand(idx), 
						command_deleter
					}
				),
				i
			);
		}

		for (int idx = 0; idx < trigger_count; idx++)
		{
			this->_triggers.emplace_back(
				api_table->GetTriggerName(idx),
				std::move(
					std::unique_ptr<Trigger::ITrigger, void(*)(Trigger::ITrigger*)>
					{
						api_table->GetTrigger(idx), 
						trigger_deleter
					}
				),
				i
			);
		}
	}

	std::sort(this->_GroupCommands.begin(), this->_GroupCommands.end(), 
	[](const Tag<GroupCommand::IGroupCommand>& a, const Tag<GroupCommand::IGroupCommand>& b)
	{ return a.data->Priority() > b.data->Priority(); });
}

void ElanorBot::_OffloadPlugins()
{
	this->_GroupCommands.clear();
	this->_triggers.clear();
	for (auto&& p : this->_plugins)
	{
		p.Close();
	}
	this->_plugins.clear();
}

void ElanorBot::Start(const Mirai::SessionConfigs &opts)
{
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		this->_LoadPlugins(this->_config.Get<std::string>("/PluginsFolder", "./Plugins"));

		this->_groups.SetSuid(this->_config.Get<Mirai::QQ_t>("/suid", {}));

		std::vector<std::pair<std::string, int>> command_list;
		for (const auto& p : this->_GroupCommands)
			command_list.emplace_back(p.name, p.data->Permission());
		this->_groups.SetCommands(std::move(command_list));

		std::vector<std::pair<std::string, bool>> trigger_list;
		for (const auto& p : this->_triggers)
			trigger_list.emplace_back(p.name, p.data->isDefaultOn());

		this->_groups.LoadGroups(this->_config.Get<std::string>("/BotFolder", "./Bots"));
	}

	this->_timer.LaunchLoop([this]{ 
		auto groups = this->_groups.GetAllGroups();
		for (const auto& p : groups)
		{
			p->ToFile(this->_config.Get<std::filesystem::path>("/BotFolder", "./Bots") / p->gid.to_string());
			std::this_thread::sleep_for(1s);
		}
	}, 1h);

	this->_client->On<Mirai::NudgeEvent>([this](Mirai::NudgeEvent e)
	{
		this->_NudgeEventHandler(e);
	});

	this->_client->On<Mirai::GroupMessageEvent>([this](Mirai::GroupMessageEvent e)
	{
		this->_GroupMessageEventHandler(e);
	});

	this->_client->On<Mirai::ClientConnectionEstablishedEvent>([this](Mirai::ClientConnectionEstablishedEvent e)
	{
		this->_ConnectionOpenedHandler(e);
	});

	this->_client->On<Mirai::ClientConnectionClosedEvent>([this](Mirai::ClientConnectionClosedEvent e)
	{
		this->_ConnectionClosedHandler(e);
	});

	this->_client->On<Mirai::ClientConnectionErrorEvent>([this](Mirai::ClientConnectionErrorEvent e)
	{
		this->_ConnectionErrorHandler(e);
	});

	this->_client->On<Mirai::ClientParseErrorEvent>([this](Mirai::ClientParseErrorEvent e)
	{
		this->_ParseErrorHandler(e);
	});

	while (true)
	{
		try
		{
			LOG_INFO(Utils::GetLogger(), "尝试与 mirai-api-http 建立连接...");
			this->_client.Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			LOG_WARN(Utils::GetLogger(), ex.what());
		}
		std::this_thread::sleep_for(3s);
	}

	try
	{
		string mah_version = this->_client->GetMiraiApiHttpVersion();
		string mc_version = string(this->_client->GetVersion());
		LOG_INFO(Utils::GetLogger(), "mirai-api-http 的版本: " + mah_version
			+ "; mirai-cpp 的版本: " + mc_version);
		if (mah_version != mc_version)
		{
			LOG_WARN(Utils::GetLogger(), "你的 mirai-api-http 插件的版本与 mirai-cpp 的版本不同，可能存在兼容性问题。");
		}
	}
	catch (const std::exception& ex)
	{
		LOG_WARN(Utils::GetLogger(), ex.what());
	}

	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		this->_running = true;
		for (const auto& p : this->_triggers)
		{
			Trigger::ITrigger* trigger = p.data.get();
			this->_timer.Launch([this, trigger]{ trigger->Action(this->_groups, this->_client, this->_config); }, [trigger]{ return trigger->GetNext(); });
		}
	}
	
	LOG_INFO(Utils::GetLogger(), "Triggers enabled, Elanor working...");
}

void ElanorBot::Stop()
{
	LOG_INFO(Utils::GetLogger(), "Shutting down Elanor...");
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		this->_timer.StopAll();
		LOG_INFO(Utils::GetLogger(), "Timers stopped");
		this->_running = false;
	}

	this->_client.Disconnect();
	LOG_INFO(Utils::GetLogger(), "mirai-api-http disconnected");

	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		this->_OffloadPlugins();
	}
}

void ElanorBot::_NudgeEventHandler(Mirai::NudgeEvent& e)
{
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		if (!this->_running)	
			return;
	}

	try
	{
		if (e.GetTarget().GetTarget() != this->_client->GetBotQQ())
			return;
		if (e.GetSender() == this->_client->GetBotQQ())
			return;

		if (e.GetTarget().GetTargetKind() == Mirai::NudgeTarget::GROUP)
		{
			string sender = this->_client->GetMemberInfo(e.GetTarget().GetGroup(), e.GetSender()).MemberName;
			string group_name = this->_client->GetGroupConfig(e.GetTarget().GetGroup()).name;
			LOG_INFO(Utils::GetLogger(), "有人戳bot <OnNudgeEvent>\t<- [" + sender + "(" + e.GetSender().to_string() + "), " + group_name + "(" + e.GetTarget().GetGroup().to_string() + ")]");
			
			std::unique_lock<std::mutex> lock(this->_NudgeMtx, std::try_to_lock);
			if (!lock)
			{
				LOG_INFO(Utils::GetLogger(), "冷却中 <OnNudgeEvent>");
				return;
			}
			
			constexpr int SWORE_PROB = 6;
			std::uniform_int_distribution<int> dist(0, SWORE_PROB - 1);
			int i = dist(Utils::GetRngEngine());
			if (i)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				this->_client->SendNudge(Mirai::NudgeTarget{Mirai::NudgeTarget::GROUP, e.GetSender(), e.GetTarget().GetGroup()});
				LOG_INFO(Utils::GetLogger(), "戳回去了 <OnNudgeEvent>\t-> [" + sender + "(" + e.GetSender().to_string() + "), " + group_name + "(" + e.GetTarget().GetGroup().to_string() + ")]");
			}
			else
			{
				this->_client->SendGroupMessage(e.GetTarget().GetGroup(), Mirai::MessageChain().At(e.GetSender()).Plain(" 戳你吗"));
				LOG_INFO(Utils::GetLogger(), "骂回去了 <OnNudgeEvent>\t-> [" + sender + "(" + e.GetSender().to_string() + "), " + group_name + "(" + e.GetTarget().GetGroup().to_string() + ")]");
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	catch (std::exception &e)
	{
		LOG_WARN(Utils::GetLogger(), e.what());
	}
}

void ElanorBot::_GroupMessageEventHandler(Mirai::GroupMessageEvent& gm)
{
	{
		std::lock_guard<std::mutex> lk(this->_MemberMtx);
		if (!this->_running)	
			return;
	}

	if (gm.GetSender().id == this->_client->GetBotQQ())
		return;
	
	Group& group = this->_groups.GetGroup(gm.GetSender().group.id);

	int priority = -1;
	for (const auto& p : this->_GroupCommands)
	{
		if ((p.data)->Priority() < priority)
			break;
		
		bool matched = false;
		try
		{
			matched = (p.data)->Execute(gm, group, this->_client, this->_config);
		}
		catch (const std::exception& e)
		{
			LOG_ERROR(Utils::GetLogger(), e.what());
		}
		if (matched)
			priority = (p.data)->Priority();
	}
}

void ElanorBot::_ConnectionOpenedHandler(Mirai::ClientConnectionEstablishedEvent& e)
{
	LOG_INFO(Utils::GetLogger(), "成功建立连接, session-key: " + e.SessionKey);
	std::lock_guard<std::mutex> lk(this->_MemberMtx);
	this->_run();
}

void ElanorBot::_ConnectionClosedHandler(Mirai::ClientConnectionClosedEvent& e)
{
	LOG_INFO(Utils::GetLogger(), "连接关闭：" + e.reason + " <" + std::to_string(e.code) + ">");
	std::lock_guard<std::mutex> lk(this->_MemberMtx);
	this->_stop();
}

void ElanorBot::_ConnectionErrorHandler(Mirai::ClientConnectionErrorEvent& e)
{
	LOG_WARN(Utils::GetLogger(), "连接时出现错误: " + e.reason + "，重试次数: " + std::to_string(e.RetryCount));
}

void ElanorBot::_ParseErrorHandler(Mirai::ClientParseErrorEvent& e)
{
	LOG_WARN(Utils::GetLogger(), "解析事件时出现错误: " + string(e.error.what()));
}

}