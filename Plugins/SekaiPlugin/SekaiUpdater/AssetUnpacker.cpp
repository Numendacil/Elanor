#include "AssetUnpacker.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/wait.h>

#include "TaskDispatcher.hpp"

namespace filesystem = std::filesystem;
using std::string;

namespace AssetUpdater
{

AssetUnpacker::AssetUnpacker(
	std::size_t pool_size, 
	string pymodule,
	std::filesystem::path path_prefix,
	TaskDispatcher* dispatcher
)
: _pymodule(std::move(pymodule)), _path_prefix(std::move(path_prefix)), _dispatcher(dispatcher)
{
	this->_workers.reserve(pool_size);
	for (std::size_t i = 0; i < pool_size; i++)
	{
		_workers.emplace_back([this]() { this->_loop(); });
	}
}

AssetUnpacker::~AssetUnpacker()
{
	{
		std::unique_lock<std::mutex> lk(this->_mtx);
		this->_stop = true;
	}
	this->_cv.notify_all();
	for (std::thread& th : this->_workers)
	{
		if (th.joinable()) 
			th.join();
	}
}

void AssetUnpacker::unpack(std::string key)
{
	std::unique_lock<std::mutex> lk(this->_mtx);
	this->_workloads.emplace(std::move(key));
	this->_cv.notify_one();
}

namespace
{

std::tuple<pid_t, int, int> ForkAndRedirect(std::vector<std::string> cmd)
{
	if (cmd.empty())
		return {};
	std::vector<char *> param(cmd.size() + 1);
	std::transform(cmd.begin(), cmd.end(), param.begin(),
		[](std::string& str) { return str.data(); });
	param.back() = nullptr;

	pid_t pid{};
	int p1[2];	// NOLINT(*-avoid-c-arrays)
	int p2[2];	// NOLINT(*-avoid-c-arrays)
	if (pipe(p1) == -1)
	{
		throw std::runtime_error("Failed to open pipe");
	}
	if (pipe(p2) == -1)
	{
		throw std::runtime_error("Failed to open pipe");
	}
	if ((pid = fork()) == -1)
	{
		throw std::runtime_error("Failed to fork program");
	}
	if (pid == 0)
	{
		dup2(p1[1], STDOUT_FILENO);
		dup2(p2[0], STDIN_FILENO);
		close(p1[0]);
		close(p1[1]);
		close(p2[0]);
		close(p2[1]);
		execvp(param[0], param.data());
		perror(("Failed to execute " + cmd[0]).c_str());
		exit(1);
	}
	close(p1[1]);
	close(p2[0]);

	return std::tie(pid, p1[0], p2[1]);
}

class Pipe
{
private:
	FILE* _file_in{};
	FILE* _file_out{};
	pid_t _pid{};

public:
	Pipe() = default;
	Pipe(const Pipe&) = delete;
	Pipe& operator=(const Pipe&) = delete;
	Pipe(Pipe&& rhs) : _pid(rhs._pid), _file_in(rhs._file_in), _file_out(rhs._file_out)
	{
		rhs._pid = 0;
		rhs._file_in = nullptr;
		rhs._file_out = nullptr;
	}
	Pipe& operator=(Pipe&& rhs)
	{
		if (this == &rhs)
			return *this;

		this->_pid = rhs._pid;
		this->_file_in = rhs._file_in;
		this->_file_out = rhs._file_out;
		rhs._pid = 0;
		rhs._file_in = nullptr;
		rhs._file_out = nullptr;
		return *this;
	}

	~Pipe()
	{
		if (_file_in)
			fclose(_file_in);	// NOLINT(*-owning-memory)
		if (_file_out)
			fclose(_file_out);	// NOLINT(*-owning-memory)
	}

	void open(std::string pymodule)
	{
		auto [pid, p_read, p_write] = ForkAndRedirect({
			"python", "-m", std::move(pymodule)
		});

		this->_pid = pid;
		this->_file_in = fdopen(p_read, "r");
		this->_file_out = fdopen(p_write, "w");
		if (!this->_file_in || !this->_file_out)
			throw std::runtime_error("Failed to open file descriptor");
	}

	Pipe(std::string pymodule) { this->open(std::move(pymodule)); }

	void write(const std::string& str)
	{
		fputs(str.c_str(), this->_file_out);
		fputc('\n', this->_file_out);
		fflush(this->_file_out);
	}

	std::string read()
	{
		std::string result;
		constexpr size_t BUFFER_SIZE = 1024;
		char buffer[BUFFER_SIZE];	// NOLINT(*-avoid-c-arrays)
		do
		{
			if (!fgets(buffer, BUFFER_SIZE, this->_file_in))
				return result;
			result.append(buffer);
		}
		while (result.back() != '\n');
		return result.substr(0, result.size() - 1);
	}

	int close()
	{
		int status{};
		fclose(this->_file_out);	// NOLINT(*-owning-memory)
		fclose(this->_file_in);		// NOLINT(*-owning-memory)
		this->_file_in = nullptr;
		this->_file_out = nullptr;
		if (waitpid(this->_pid, &status, 0) == -1)
			return -1;
		return status;
	}

	bool isOpen() const
	{
		return this->_file_in && this->_file_out && this->_pid;
	}

};

}

void AssetUnpacker::_loop()
{
	Pipe pipe;

	while(true)
	{
		string key;
		{
			std::unique_lock<std::mutex> lk(this->_mtx);
			if (!this->_cv.wait_for(lk, std::chrono::seconds(10), 
				[this] { 
					return this->_stop || !this->_workloads.empty(); 
				}	
			))
			{
				if (pipe.isOpen())
				{
					pipe.write("exit");
					string str;
					do
					{
						str = pipe.read();
					}
					while(!str.empty() && str != "exit");
					(void)pipe.close();
				}
				continue;
			}
			
			if (this->_stop) 
				break;
			
			if (!pipe.isOpen())
				pipe.open(this->_pymodule);
			key = std::move(this->_workloads.front());
			this->_workloads.pop();
		}

		filesystem::remove_all(this->_path_prefix / key);
		filesystem::remove_all(this->_path_prefix / (key + "_rip"));
		filesystem::create_directories(this->_path_prefix / key);
		pipe.write("start");
		pipe.write(key);
		pipe.write(this->_path_prefix);
		pipe.write(this->_path_prefix / (key + ".pack"));
		pipe.write("finish");

		string result = pipe.read();
		if (result.empty())
		{
			int status = pipe.close();
			if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
			{
				this->_dispatcher->ReturnTask({
					std::move(key), TaskDispatcher::FAILED, 
					"Python script crashed with status: " + std::to_string(status)
				});
			}
			pipe = Pipe(this->_pymodule);	// restart module
			continue;
		}

		if (result == "ok")
			this->_dispatcher->ReturnTask({std::move(key), TaskDispatcher::DECODE, result});
		else
			this->_dispatcher->ReturnTask({std::move(key), TaskDispatcher::FAILED, result});
	}

	if (pipe.isOpen())
	{
		pipe.write("exit");
		string str;
		do
		{
			str = pipe.read();
		}
		while(!str.empty() && str != "exit");
		(void)pipe.close();
	}

}

} // namespace AssetUpdater