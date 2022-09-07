#include "Common.hpp"

#include <random>

namespace Utils
{

std::mt19937& GetRngEngine()
{
	static std::mt19937 rng(std::random_device{}());
	return rng;
}

} // namespace Utils