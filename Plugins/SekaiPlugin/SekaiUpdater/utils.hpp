#ifndef _ASSET_UPDATER_UTILS_HPP_
#define _ASSET_UPDATER_UTILS_HPP_

#include <string>
#include <memory>
#include <functional>
#include <chrono>
#include <iostream>

namespace AssetUpdater
{

inline std::string gettime()
{
	using namespace std::chrono;
	auto tp = system_clock::now();
	std::time_t t = system_clock::to_time_t(tp);

	constexpr size_t BUFFER_SIZE = 128;
	char buf[BUFFER_SIZE]; // NOLINT(*-avoid-c-arrays)
	std::strftime(buf, BUFFER_SIZE, "\x1b[95m%Y-%m-%d %H:%M:%S\x1b[0m", std::localtime(&t));
	return std::string{buf}.append(" \x1b[96;100m[AssetUpdate]\x1b[0m ");
}


#define LOGGING(_str_) std::cout << (AssetUpdater::gettime() + (_str_) + "\n") << std::flush;


}


#endif