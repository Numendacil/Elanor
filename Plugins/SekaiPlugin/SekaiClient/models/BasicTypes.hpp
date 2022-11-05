#ifndef _SEKAI_BASIC_TYPES_HPP_
#define _SEKAI_BASIC_TYPES_HPP_

#include <deque>
#include <functional>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

namespace Sekai
{

class UID_t
{
protected:
	uint64_t _number = 0;

public:
	UID_t() = default;
	explicit UID_t(uint64_t num) : _number{num} {}
	explicit operator uint64_t() const { return this->_number; }
	std::string to_string() const { return std::to_string(this->_number); }

	bool operator<(const UID_t& uid) const { return this->_number < uid._number; }
	bool operator==(const UID_t& uid) const { return this->_number == uid._number; }

	bool operator!=(const UID_t& uid) const { return !(*this == uid); }
	bool operator>(const UID_t& uid) const { return uid < *this; }
	bool operator>=(const UID_t& uid) const { return !(*this < uid); }
	bool operator<=(const UID_t& uid) const { return !(uid < *this); }

	friend void to_json(nlohmann::json& j, const UID_t& p) { j = p._number; }
	friend void from_json(const nlohmann::json& j, UID_t& p) { p._number = j.get<uint64_t>(); }
};

struct Account
{
	UID_t id;
	std::string credential;

	friend void to_json(nlohmann::json& j, const Account& p)
	{
		j["id"] = p.id;
		j["credential"] = p.credential;
	}
	friend void from_json(const nlohmann::json& j, Account& p)
	{
		p.id = j.at("id").get<UID_t>();
		p.credential = j.at("credential").get<std::string>();
	}
};

class split_json_sax : public nlohmann::json_sax<nlohmann::json>
{
private:
	using json = nlohmann::json;
	std::string key_str;
	std::string split_str;

	bool start = false;
	bool first_key = true;

	enum TYPE
	{
		OBJ,
		ARR,
		KEY
	};
	std::deque<std::pair<TYPE, bool>> stack;

	std::function<bool(std::string key, std::string content)> _callback;

public:
	split_json_sax(std::function<bool(std::string key, std::string content)> callback)
		: _callback(std::move(callback)){};

	bool null() override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += "null";
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += "null";
			}
			else
				split_str += ",null";
		}
		else
			return false;

		return true;
	}

	// called when a boolean is parsed; value is passed
	bool boolean(bool val) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += (val ? "true" : "false");
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += (val ? "true" : "false");
			}
			else
				split_str += (val ? ",true" : ",false");
		}
		else
			return false;

		return true;
	}

	// called when a signed or unsigned integer number is parsed; value is passed
	bool number_integer(number_integer_t val) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += json(val).dump();
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += json(val).dump();
			}
			else
				split_str += "," + json(val).dump();
		}
		else
			return false;

		return true;
	}
	bool number_unsigned(number_unsigned_t val) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += json(val).dump();
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += json(val).dump();
			}
			else
				split_str += "," + json(val).dump();
		}
		else
			return false;

		return true;
	}

	// called when a floating-point number is parsed; value and original string is passed
	bool number_float(number_float_t val, const string_t& s) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += json(val).dump();
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += json(val).dump();
			}
			else
				split_str += "," + json(val).dump();
		}
		else
			return false;

		return true;
	}

	// called when a string is parsed; value is passed and can be safely moved away
	bool string(string_t& val) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += json(val).dump();
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += json(val).dump();
			}
			else
				split_str += "," + json(val).dump();
		}
		else
			return false;

		return true;
	}
	// called when a binary value is parsed; value is passed and can be safely moved away
	bool binary(binary_t& val) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().first == KEY)
		{
			split_str += json(val).dump();
			stack.pop_back();
		}
		else if (stack.back().first == ARR)
		{
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += json(val).dump();
			}
			else
				split_str += "," + json(val).dump();
		}
		else
			return false;

		return true;
	}

	// called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
	bool start_object(std::size_t elements) override
	{
		if (!start)
		{
			start = true;
			first_key = true;
			return true;
		}

		if (stack.empty()) return false;

		if (stack.back().second)
		{
			split_str += "{";
			stack.back().second = false;
		}
		else
			split_str += ",{";
		stack.emplace_back(OBJ, true);

		return true;
	}
	bool end_object() override
	{
		if (!start) return false;

		if (stack.empty()) return this->_callback(std::move(this->key_str), std::move(this->split_str));

		if (stack.size() < 2) return false;

		if (stack.back().first != OBJ) return false;
		stack.pop_back();

		if (stack.back().first == KEY) stack.pop_back();

		split_str += "}";

		return true;
	}
	bool start_array(std::size_t elements) override
	{
		if (!start || stack.empty()) return false;

		if (stack.back().second)
		{
			split_str += "[";
			stack.back().second = false;
		}
		else
			split_str += ",[";
		stack.emplace_back(ARR, true);

		return true;
	}
	bool end_array() override
	{
		if (stack.size() < 2 || stack.back().first != ARR) return false;
		stack.pop_back();
		if (stack.back().first == KEY) stack.pop_back();

		split_str += "]";

		return true;
	}
	// called when an object key is parsed; value is passed and can be safely moved away
	bool key(string_t& val) override
	{
		if (!start) return false;
		if (stack.empty())
		{
			if (first_key) first_key = false;
			else
			{
				if (!this->_callback(std::move(this->key_str), std::move(this->split_str))) return false;
			}

			key_str = std::move(val);
			split_str.clear();
			split_str.reserve(1024 * 50); // NOLINT(*-avoid-magic-numbers)
		}
		else
		{
			if (stack.back().first == ARR) return false;
			if (stack.back().second)
			{
				stack.back().second = false;
				split_str += json(val).dump() + ":";
			}
			else
				split_str += "," + json(val).dump() + ":";
		}
		stack.emplace_back(KEY, true);

		return true;
	}

	// called when a parse error occurs; byte position, the last token, and an exception is passed
	bool parse_error(std::size_t position, const std::string& last_token,
	                 const nlohmann::detail::exception& ex) override
	{
		return false;
	}
};

} // namespace Sekai

#endif