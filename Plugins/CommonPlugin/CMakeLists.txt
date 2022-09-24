add_plugin(CommonPlugin)

target_sources(
	CommonPlugin PRIVATE

	PluginMain.cpp

	MorningTrigger.hpp
	MorningTrigger.cpp
	RollDice.hpp
	RollDice.cpp
)