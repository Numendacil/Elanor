#include "Common.hpp"

#include <fstream>
#include <random>
#include <string>

#include <Core/Utils/Logger.hpp>

using json = nlohmann::json;

namespace Utils
{

std::mt19937& GetRngEngine()
{
	static std::mt19937 rng(std::random_device{}());
	return rng;
}

bool BotConfig::FromFile(const std::string& filepath)
{
	std::ifstream ifile(filepath);
	if (ifile.fail())
	{
		LOG_WARN(GetLogger(), "Failed to open file <BotConfig>: " + filepath);
		return false;
	}
	try
	{
		std::lock_guard<std::mutex> lk(this->_mtx);
		this->_config = json::parse(ifile);
	}
	catch (const std::exception& e)
	{
		LOG_WARN(GetLogger(), "Failed to parse file <BotConfig>: " + std::string(e.what()));
		return false;
	}
	return true;
}

} // namespace Utils