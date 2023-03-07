#include "pch.h"
#include "PresetShuffle.h"

void PresetShuffle::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string PresetShuffle::GetPluginName(){
	return "Preset Shuffle";
}

void PresetShuffle::RenderSettings() {
	ImGui::TextUnformatted("A plugin that shuffles between selected presets in your garage after a set amount of games.  Game counter resets to 0 upon exiting the game.");

	CVarWrapper enableCvar = cvarManager->getCvar("ps_enabled");
	if (!enableCvar) { return; }
	bool enabled = enableCvar.getBoolValue();
	if (ImGui::Checkbox("Enable plugin", &enabled)) {					// Checkbox determines plugin enabled state.
		enableCvar.setValue(enabled);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Enables the plugin.");
	}

	CVarWrapper lockCvar = cvarManager->getCvar("LockPreset");
	if (!lockCvar) { return; }
	bool locked = lockCvar.getBoolValue();
	if (ImGui::Checkbox("Lock Preset", &locked)) {						// Checkbox determines preset lock state.
		lockCvar.setValue(locked);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Locks your current preset, pauses game-shuffle cycle (Shuffle Preset button still available)");
	}

	if (ImGui::Button("Shuffle Preset")) {								// Button shuffles preset instantly.
		gameWrapper->Execute([this](GameWrapper* gw) { cvarManager->executeCommand("ShufflePreset"); });
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Shuffles your preset immediately.");
	}

	CVarWrapper rangeCvar = cvarManager->getCvar("game_range");
	CVarWrapper countCvar = cvarManager->getCvar("game_count");
	if (!rangeCvar) { return; }
	int rangeMax = rangeCvar.getIntValue();
	if (ImGui::SliderInt("Number of Games", &rangeMax, 1, 25)) {		// Slider determines game range.
		rangeCvar.setValue(rangeMax);
	}

	/* DEBUG
	if (ImGui::Button("Check State")) {									// Button logs game state and relevant info.
		gameWrapper->Execute([this](GameWrapper* gw) { cvarManager->executeCommand("CheckState"); });
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Check Game State");
	}
	*/
}