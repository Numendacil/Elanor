add_plugin(CommonPlugin)

target_sources(
	CommonPlugin PRIVATE

	PluginMain.cpp

	MorningTrigger.hpp
	MorningTrigger.cpp
)