#include "SekaiClient.hpp"

#include <SekaiNetworkClient/SekaiNetworkClient.hpp>

#include <PluginUtils/Common.hpp>

#include <models/BasicTypes.hpp>
#include <models/Exceptions.hpp>

#include <Core/Utils/Logger.hpp>
#include <Core/Utils/Common.hpp>
#include "libmirai/Utils/Logger.hpp"
#include <stduuid/include/uuid.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using std::string;
using json = nlohmann::json;
namespace filesystem = std::filesystem;

using namespace std::literals;

namespace Sekai
{

SekaiClient::SekaiClient(
	std::filesystem::path AssetFolder,
	std::filesystem::path TemporaryFolder,
	std::filesystem::path UpdaterPath,
	std::string PythonModule,
	std::vector<std::string> filter,
	std::string AESKey, std::string AESIV, 
	std::string ApiUrl,
	// uint64_t HCAKey,
	size_t PoolSize,
	std::string ProxyHost, int ProxyPort
)
: _AssetFolder(std::move(AssetFolder)), 
  _TmpFolder(std::move(TemporaryFolder)),
  _UpdaterPath(std::move(UpdaterPath)),
  _ApiUrl(std::move(ApiUrl)),
  /*_HCAKey(HCAKey),*/ _PoolSize(PoolSize), 
  _PyModule(std::move(PythonModule)),
  _AssetFilter(std::move(filter))
{
	// Create directories
	filesystem::create_directories(this->_AssetFolder / "database");
	filesystem::create_directories(this->_AssetFolder / "assets");

	// Read version info
	string AppVersion = "2.3.5";
	string AppHash = "cc22bebb-bce8-1744-2543-16a166dd220d";
	string AssetVersion = "2.3.5.10";
	string AssetHash = "59b61c36-1d09-b6d7-1921-233766dce4c1";
	string DataVersion = "2.3.5.11";
	string AssetBundleHostHash = "cf2d2388";
	{
		std::ifstream VersionsFile(this->_AssetFolder / "database" / "versions.json");
		if (VersionsFile)
		{
			try
			{
				json versions = json::parse(VersionsFile);
				for (const auto& version : versions.at("appVersions"))
				{
					if (version.at("appVersionStatus").get<string>() == "available" && 
					version.at("systemProfile").get<string>() == "production")
					{
						version.at("appVersion").get_to(AppVersion);
						version.at("appHash").get_to(AppHash);
						version.at("assetVersion").get_to(AssetVersion);
						version.at("assetHash").get_to(AssetHash);
						version.at("dataVersion").get_to(DataVersion);
						break;
					}
				}

			}
			catch(const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(), "Unexpected error occured when reading from versions.json: "s + e.what());
			}
		}
	}
	{
		std::ifstream GameVersion(this->_AssetFolder / "database" / "gameVersion.json");
		if (GameVersion)
		{
			try
			{
				json::parse(GameVersion).at("assetbundleHostHash").get_to(AssetBundleHostHash);

			}
			catch(const std::exception& e)
			{
				LOG_WARN(Utils::GetLogger(), "Unexpected error occured when reading from gameVersion.json: "s + e.what());
			}
		}
	}

	this->_cli = std::make_unique<SekaiNetworkClient>(
		std::move(AESKey), std::move(AESIV), 
		this->_ApiUrl,
		std::move(ProxyHost), ProxyPort,
		std::move(AppVersion),
		std::move(AppHash),
		std::move(AssetVersion),
		std::move(AssetHash),
		std::move(DataVersion),
		std::move(AssetBundleHostHash)
	);

	// Load accounts
	std::ifstream account_file(this->_AssetFolder / "database" / "accounts.json");
	if (!account_file)
		return;
	try
	{
		json accounts = json::parse(account_file);
		this->_cli->AddCredentials(accounts.get<std::vector<Account>>());
	}
	catch(const std::exception& e)
	{
		return;
	}
}

SekaiClient::~SekaiClient()
{
	auto accounts = this->_cli->GetCredentials();
	std::ofstream ofile(this->_AssetFolder / "database" / "accounts.json");
	ofile << json(accounts).dump(1, '\t');
}


void SekaiClient::_UpdateMasterDB() const
{
	{
		LOG_DEBUG(Utils::GetLogger(), "Loading Game version");
		json GameVersion = this->_cli->GetGameVersionInfo();

		const filesystem::path version_info = this->_AssetFolder / "database/gameVersion.json";
		std::ofstream ofile(version_info);
		ofile << GameVersion.dump(1, '\t');
	}

	// {
	// 	json version = this->_cli->GetVersionInfo();

	// 	const filesystem::path version_info = this->_AssetFolder / "database/versions.json";
	// 	std::ofstream ofile(version_info);
	// 	ofile << version.dump(1, '\t');
	// }

	// std::this_thread::sleep_for(1s);

	{
		LOG_DEBUG(Utils::GetLogger(), "Loading Asset list");
		json assets = this->_cli->GetAssetList();
		std::ofstream ofile(this->_AssetFolder / "database" / "assetList.json");
		ofile << assets.dump();
	}

	// std::this_thread::sleep_for(3s);

	{
		LOG_DEBUG(Utils::GetLogger(), "Loading Master suite");
		split_json_sax sax([this](string key, string content)
		{
			if (key.empty() || content.empty())
				return false;

			const auto filepath = this->_AssetFolder / "database" / (key + ".json");
			{
				std::ofstream ofile(filepath);
				if (!ofile)
					return false;

				ofile << content;
			}

			string{}.swap(content);
			string{}.swap(key);
			// Check for json validity
			{
				std::ifstream ifile(filepath);
				if (!ifile)
					return false;
				return json::accept(ifile);
			}
			return true;
		});
		string master = this->_cli->GetMasterSuiteInfo();

		if (!json::sax_parse(std::move(master), &sax, json::input_format_t::msgpack))
			throw UpdateError("Failed to parse master suite");
	}
}

void SekaiClient::_UpdateAssets() const
{
	std::vector<string> cmd{
		this->_UpdaterPath, 
		this->_AssetFolder, this->_PyModule,
		this->_ApiUrl,
		/*std::to_string(this->_HCAKey),*/ std::to_string(this->_PoolSize)
	};
	cmd.insert(cmd.end(), this->_AssetFilter.begin(), this->_AssetFilter.end());
	int status = Utils::ExecCmd(std::move(cmd));

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		throw UpdateError("Updater exited abnormally with status " + std::to_string(status));
}



json SekaiClient::GetMetaInfo(const string& category)
{
	std::shared_lock<std::shared_mutex> lk(this->_mtx);
	std::ifstream file(this->_AssetFolder / "database" / (category + ".json"));
	if (!file)
		throw FileError("File " + category + ".json not found");
	return json::parse(file);
}

std::vector<filesystem::path> SekaiClient::GetAssetContents(const string& key)
{
	std::vector<filesystem::path> contents{};
	const auto base = this->_AssetFolder / "assets" / (key + "_rip");

	std::shared_lock<std::shared_mutex> lk(this->_mtx);
	for (const auto& entry : filesystem::directory_iterator(base))
	{
		contents.emplace_back(filesystem::relative(entry.path(), base));
	}
	return contents;
}

filesystem::path SekaiClient::GetFile(const string& key, const filesystem::path& file)
{
	const auto target = this->_AssetFolder / "assets" / (key + "_rip") / file;
	uuids::basic_uuid_random_generator rng(Utils::GetRngEngine());
	const auto tmpfile = this->_TmpFolder / uuids::to_string(rng());

	std::shared_lock<std::shared_mutex> lk(this->_mtx);
	filesystem::copy_file(target, tmpfile);
	return tmpfile;
}

string SekaiClient::GetFileContents(const string& key, const filesystem::path& file)
{
	const auto target = this->_AssetFolder / "assets" / (key + "_rip") / file;

	string result;
	constexpr size_t BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)

	std::shared_lock<std::shared_mutex> lk(this->_mtx);
	std::ifstream ifile(target);
	while(ifile.read(buffer, BUFFER_SIZE))
		result.append(buffer, BUFFER_SIZE);
	result.append(buffer, ifile.gcount());
	return result;
}

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

std::vector<string> SekaiClient::UpdateContents() const
{
	std::unique_lock<std::shared_mutex> lk(this->_mtx);

	string AppVersion;
	string AssetVersion;
	string DataVersion;
	{
		const filesystem::path version_info = this->_AssetFolder / "database/versions.json";
		std::ifstream ifile(version_info);
		if (ifile)
		{
			json version = json::parse(ifile);
			for (const auto& obj : version.at("appVersions"))
			{
				if (obj.at("appVersionStatus").get<string>() == "available")
				{
					AppVersion = obj.at("appVersion").get<string>();
					AssetVersion = obj.at("assetVersion").get<string>();
					DataVersion = obj.at("dataVersion").get<string>();
					break;
				}
			}
		}
	}

	string AppVersionNew;
	string AssetVersionNew;
	string DataVersionNew;
	json version;
	{
		version = this->_cli->GetVersionInfo();
		for (const auto& obj : version.at("appVersions"))
		{
			if (obj.at("appVersionStatus").get<string>() == "available")
			{
				AppVersionNew = obj.at("appVersion").get<string>();
				AssetVersionNew = obj.at("assetVersion").get<string>();
				DataVersionNew = obj.at("dataVersion").get<string>();
				break;
			}
		}
	}

	std::vector<string> updated_assets;

	if (DataVersion != DataVersionNew)
	{
		LOG_INFO(Utils::GetLogger(), "Found new data version: " + DataVersion + " -> " + DataVersionNew);
		this->_UpdateMasterDB();

		{
			const filesystem::path version_info = this->_AssetFolder / "database/versions.json";
			std::ofstream ofile(version_info);
			ofile << version.dump(1, '\t');
		}
		
		LOG_INFO(Utils::GetLogger(), "Data files update finished");
	}

	if (AssetVersion != AssetVersionNew)
	{
		LOG_INFO(Utils::GetLogger(), "Found new asset version: " + AssetVersion + " -> " + AssetVersionNew);
		this->_UpdateAssets();
		std::ifstream ifile(this->_AssetFolder / "updates.txt");
		if (!ifile)
			throw UpdateError("updates.txt not found");
		
		string line;
		while(std::getline(ifile, line))
		{
			if (!line.empty())
				updated_assets.emplace_back(std::move(line));
		}
		LOG_INFO(Utils::GetLogger(), "Asset files update finished");
	}
	else
	{
		// Check for missing files
		bool missing = false;
		{
			std::map<std::string, AssetMeta> AssetInfo;
			{
				std::ifstream ifile(this->_AssetFolder / "AssetInfo.json");
				if (ifile)
					AssetInfo = json::parse(ifile).get<decltype(AssetInfo)>();
			}
			
			json AssetBundles;
			{
				std::ifstream ifile(this->_AssetFolder / "database/assetList.json");
				if (!ifile)
				{
					LOG_WARN(Utils::GetLogger(), "Failed to open asset file");
					return {};
				}

				AssetBundles = json::parse(ifile).at("bundles");
			}
			for (const auto& item : AssetBundles.items())
			{
				string key = item.key();
				bool skip = false;
				for (const auto& reg : this->_AssetFilter)
				{
					if (std::regex_match(key, std::regex(reg, std::regex::ECMAScript)))
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
							const auto dl_file = this->_AssetFolder / "assets" / (key + ".pack");
							const auto unpack_folder = this->_AssetFolder / "assets" / key;
							const auto final_folder = this->_AssetFolder / "assets" / (key + "_rip");

							bool has_file = filesystem::exists(dl_file) && filesystem::is_regular_file(dl_file);
							bool has_unpack_folder = filesystem::exists(unpack_folder) && filesystem::is_directory(unpack_folder);
							bool has_final_folder = filesystem::exists(final_folder) && filesystem::is_directory(final_folder);

							if (has_final_folder && !has_unpack_folder && !has_file)
								continue;
						}
					}
					
					missing = true;
					break;
				}
			}
		}

		if (missing)
		{
			LOG_INFO(Utils::GetLogger(), "Found missing assets");
			this->_UpdateAssets();
			std::ifstream ifile(this->_AssetFolder / "updates.txt");
			if (!ifile)
				throw UpdateError("updates.txt not found");
			
			string line;
			while(std::getline(ifile, line))
			{
				if (!line.empty())
					updated_assets.emplace_back(std::move(line));
			}
			LOG_INFO(Utils::GetLogger(), "Asset files update finished");
		}
	}

	if (AppVersionNew != AppVersionNew)
	{
		LOG_INFO(Utils::GetLogger(), "Found new app version: " + AppVersionNew + " -> " + AppVersionNew);
	}

	return updated_assets;
}

}