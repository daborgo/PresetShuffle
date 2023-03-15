#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class PresetShuffle : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow {
	std::map<std::string, bool> presetMap;
	std::vector<std::string> nameVec;
	void onLoad() override;
	void onUnload() override;
	void loadHooks();
	void shufflePreset();
	void updateMap();
//	void checkState();

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};





//TODO: persistent storage maps, favorite presets?
//MAYBE: better loading screen shuffle button protection?