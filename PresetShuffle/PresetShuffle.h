#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class PresetShuffle: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow {
	void onLoad() override;
	void onUnload() override;
	void loadHooks();
	void shufflePreset();
//	void checkState();

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};





//TODO: select activated presets, recently used presets? favorite presets?
//MAYBE: better loading screen button protection? shuffle preset protection? (if in game, add to total game count and flag, if in training, allow, if in menu, allow)