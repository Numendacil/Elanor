#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>
#include <httplib.h>

#include <PluginUtils/StringUtils.hpp>
#include <PluginUtils/NetworkUtils.hpp>

#include "TaskDispatcher.hpp"
#include "utils.hpp"


using std::string;
using json = nlohmann::json;
namespace filesystem = std::filesystem;

namespace
{

struct AssetMeta
{
	std::string hash;
	uint64_t crc;
	size_t FileSize;

	bool friend operator<=>(const AssetMeta&, const AssetMeta&) = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetMeta, hash, crc, FileSize);
};

}

/** Arguments
 *  1. Asset Folder
 *  2. Pymodule
 *  3. Decryption key for hca decoding
 *  4. Pool size
 *  5- Filters for asset keys
 */

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		LOGGING("Not enough arguments");
		return 1;
	}

try
{
	// Read commandline args
	const filesystem::path AssetFolder(argv[1]);
	string pymodule(argv[2]);

	size_t PoolSize{};
	if (!Utils::Str2Num(argv[3], PoolSize))
	{
		LOGGING("argument 3 is not an unsigned number: " + string(argv[4]));
		return 1;
	}

	std::vector<std::regex> filter;
	if (argc > 4)
	{
		filter.reserve(argc - 4);
		for (int i = 4; i < argc; i++)
			filter.emplace_back(argv[i], std::regex_constants::ECMAScript);
	}

	LOGGING("Reading neccessary infomations from file...");

	// Read asset version and hash from file
	const filesystem::path VersionsFile = AssetFolder / "database/versions.json";
	string AssetVersion;
	string AssetHash;
	{
		std::ifstream ifile(VersionsFile);
		if (!ifile)
		{
			LOGGING("Failed to open file " + VersionsFile.string());
			return 1;
		}

		json version = json::parse(ifile);
		for (const auto& obj : version.at("appVersions"))
		{
			if (obj.at("appVersionStatus").get<string>() == "available")
			{
				AssetVersion = obj.at("assetVersion").get<string>();
				AssetHash = obj.at("assetHash").get<string>();
				break;
			}
		}
	}
	if (AssetVersion.empty() || AssetHash.empty())
	{
		LOGGING("Failed to read asset version info from file " + VersionsFile.string());
		return 1;
	}


	// Read asset bundle hash from file (added in game version 2.3.5)
	const filesystem::path GameVersionFile = AssetFolder / "database/gameVersion.json";
	string AssetBundleHash;
	{
		std::ifstream ifile(GameVersionFile);
		if (!ifile)
		{
			LOGGING("Failed to open file " + VersionsFile.string());
			return 1;
		}

		AssetBundleHash = json::parse(ifile).at("assetbundleHostHash").get<string>();
	}


	// Read asset lists from file
	const filesystem::path AssetListFile = AssetFolder / "database/assetList.json";
	json AssetBundles;
	string AppOS;
	{
		std::ifstream ifile(AssetListFile);
		if (!ifile)
		{
			LOGGING("Failed to open file " + AssetListFile.string());
			return 1;
		}

		AssetBundles = json::parse(ifile);
		AppOS = AssetBundles.at("os").get<string>();
		if (AssetVersion != AssetBundles.at("version").get<string>())
		{
			LOGGING("Version dismatch: " + AssetBundles.at("version").get<string>() + " <-> " + AssetVersion);
			return 3;
		}
		AssetBundles = AssetBundles.at("bundles");
	}


	// Read file hash info and prepare files for output
	std::map<std::string, AssetMeta> AssetInfo;
	const filesystem::path meta_file = AssetFolder / "AssetInfo.json";
	{
		std::ifstream ifile(meta_file);
		if (ifile)
			AssetInfo = json::parse(ifile).get<decltype(AssetInfo)>();
	}

	const filesystem::path update_file = AssetFolder / "updates.txt";
	std::ofstream UpdateFile(update_file);


	LOGGING("Requesting cookie...");

	// Obtain cookies for api
	string cookie;
	{
		httplib::Client cli("https://issue.sekai.colorfulpalette.org");

		cli.set_connection_timeout(10);		// NOLINT(*-avoid-magic-numbers)
		cli.set_write_timeout(10);		// NOLINT(*-avoid-magic-numbers)
		cli.set_read_timeout(120);		// NOLINT(*-avoid-magic-numbers)

		cli.set_default_headers({
			// {"Host", "issue.sekai.colorfulpalette.org"},
			{"Accept", "*/*"},
			{"Accept-Encoding", "deflate, gzip"},
			{"Accept-Language", "en-US,en;q=0.9"},
			{"User-Agent", "ProductName/94 CFNetwork/1335.0.3 Darwin/21.6.0"}
		});
	
		auto result = cli.Post("/api/signature");
		if (!Utils::VerifyResponse(result))
		{
			LOGGING("Failed to obtain cookie: " + 
			((result)? "Reason: " + result->reason + ", Body: " + result->body + " <" + std::to_string(result->status) + ">"
			: "Reason: " + httplib::to_string(result.error()) + " <-1>"));
			return 1;
		}

		if (result->has_header("Set-Cookie"))
		{
			cookie = result->get_header_value("Set-Cookie");
		}
		else
		{
			LOGGING("Response header does not contain 'Set-Cookie': \n"
			+ [&result]() -> string 
			{
				string header;
				for (const auto [key, value] : result->headers)
					header += key + ": " + value + "\n";
				return header;
			}());
			return 1;
		}
	}

	LOGGING("Update started");

	AssetUpdater::TaskDispatcher dispatcher(
		PoolSize, 
		std::move(pymodule), 
		"/" + AssetVersion + "/" + AssetHash + "/" + AppOS,
		AssetFolder / "assets",
		std::move(cookie), 
		std::move(AssetBundleHash)
	);


	// Progress thread
	std::mutex mtx;
	std::condition_variable cv;
	bool stop = false;
	std::thread counter_th([&]{
		while(true)
		{
			{
				std::unique_lock<std::mutex> lk(mtx);
				if (cv.wait_for(lk, std::chrono::seconds(10), [&]{ return stop; }))
				{	
					dispatcher.PrintProgress();
					return;
				}
			}
			dispatcher.PrintProgress();
		}
	});


	// Dispatch works
	for (const auto& item : AssetBundles.items())
	{
		string key = item.key();
		bool skip = false;
		for (const auto& reg : filter)
		{
			if (std::regex_match(key, reg))
			{
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			AssetMeta meta{
				item.value().at("hash").get<string>(),
				item.value().at("crc").get<uint64_t>(),
				item.value().at("fileSize").get<size_t>()
			};

			auto it = AssetInfo.find(key);
			if (it != AssetInfo.end())
			{
				if (it->second == meta)
				{
					const auto dl_file = AssetFolder / "assets" / (key + ".pack");
					const auto unpack_folder = AssetFolder / "assets" / key;
					const auto final_folder = AssetFolder / "assets" / (key + "_rip");

					bool has_file = filesystem::exists(dl_file) && filesystem::is_regular_file(dl_file);
					bool has_unpack_folder = filesystem::exists(unpack_folder) && filesystem::is_directory(unpack_folder);
					bool has_final_folder = filesystem::exists(final_folder) && filesystem::is_directory(final_folder);

					if (has_final_folder && !has_unpack_folder && !has_file)
						continue;
				}
			}
			dispatcher.AddTask(std::move(key));
			UpdateFile << item.key() << "\n";
			AssetInfo[item.key()] = std::move(meta);
		}
	}

	// Release big objects
	json{}.swap(AssetBundles);

	// Write new info
	UpdateFile.close();
	{
		std::ofstream ofile(meta_file);
		ofile << json(std::move(AssetInfo)).dump(1, '\t');
	}


	// Wait for finish and clean up
	dispatcher.WaitForFinish();

	{
		std::unique_lock<std::mutex> lk(mtx);
		stop = true;
	}
	cv.notify_one();
	if (counter_th.joinable())
		counter_th.join();
	LOGGING("Finish!");
}
catch(const std::exception& e)
{
	LOGGING("Exception occured: " + std::string(e.what()));
	return 2;
}

	return 0;
}