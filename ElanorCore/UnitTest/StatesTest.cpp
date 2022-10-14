#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <Core/States/States.hpp>
#include <libmirai/mirai.hpp>

// NOLINTBEGIN

using namespace Mirai;
using json = nlohmann::json;
using namespace std::chrono;

TEST(StatesTest, AccessCtrlListTest)
{
	std::unique_ptr<State::AccessCtrlList> state = std::make_unique<State::AccessCtrlList>();

	std::set<QQ_t> list = {1_qq, 2_qq, 3_qq, 10_qq};
	for (const auto& qq : list)
	{
		state->BlackListAdd(qq);
		state->WhiteListAdd(qq);
	}
	for (const auto& qq : list)		// Check for duplicated insertion
	{
		state->BlackListAdd(qq);
		state->WhiteListAdd(qq);
	}
	state->SetSuid(1234_qq);

	EXPECT_TRUE(state->IsBlackList(2_qq));
	EXPECT_TRUE(state->IsWhiteList(2_qq));
	EXPECT_FALSE(state->IsBlackList(20_qq));
	EXPECT_FALSE(state->IsWhiteList(20_qq));
	EXPECT_EQ(state->GetSuid(), 1234_qq);

	auto blist = state->GetBlackList();
	EXPECT_EQ(blist.size(), list.size());
	for (const auto& qq : blist)
		EXPECT_TRUE(list.contains(qq));

	auto wlist = state->GetBlackList();
	EXPECT_EQ(wlist.size(), list.size());
	for (const auto& qq : wlist)
		EXPECT_TRUE(list.contains(qq));

	state->BlackListClear();
	state->WhiteListClear();
	
	EXPECT_TRUE(state->GetBlackList().empty());
	EXPECT_TRUE(state->GetWhiteList().empty());
}

TEST(StatesTest, ActivityTest)
{
	auto state = std::make_unique<State::Activity>();

	std::mutex mtx;
	std::condition_variable cv;

	bool sync = false;

	auto token = state->CheckAndStart("TestActivity");
	EXPECT_TRUE(token);
	std::thread th([&](){
		EXPECT_TRUE(state->HasActivity());
		EXPECT_EQ(state->GetActivityName(), "TestActivity");

		auto token2 = state->CheckAndStart("TestActivity2");
		EXPECT_FALSE(token2);

		{
			std::unique_lock<std::mutex> lk(mtx);
			cv.wait(lk, [&]{ return sync; });
			sync = false;
		}

		state->AddAnswer("Answer");
	});

	json info;
	bool f = state->WaitForAnswerUntil(system_clock::now() + milliseconds(10), info);
	EXPECT_FALSE(f);
	EXPECT_TRUE(info.is_null());

	{
		std::unique_lock<std::mutex> lk(mtx);
		sync = true;
		cv.notify_all();
	}

	f = state->WaitForAnswer(info);
	EXPECT_TRUE(f);
	EXPECT_EQ(info.get<std::string>(), "Answer");

	th.join();
	token.reset();
	EXPECT_FALSE(state->HasActivity());
}

// NOLINTEND