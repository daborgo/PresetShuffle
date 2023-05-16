#include "pch.h"
#include "PresetShuffle.h"

void PresetShuffle::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string PresetShuffle::GetPluginName(){
	return "Preset Shuffle";
}

void PresetShuffle::RenderSettings() {
	ImGui::TextUnformatted("A plugin that shuffles/cycles between selected presets in your garage after a set amount of games.");

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
		ImGui::SetTooltip("Locks your current preset, pauses game-shuffle cycle (Change Preset button still available)");
	}

	CVarWrapper changeCvar = cvarManager->getCvar("ChangeMode");		// Buttons determine change mode.
	if (!changeCvar) { return; }
	int mode = changeCvar.getBoolValue();
	ImGui::RadioButton("Shuffle Presets", &mode, 0); ImGui::SameLine();
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Shuffles presets randomly.");
	}
	ImGui::RadioButton("Cycle Presets", &mode, 1);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Cycles through presets in garage order.");
	}
	if (mode == 0) { changeCvar.setValue(false); } else { changeCvar.setValue(true); }

	if (ImGui::Button("Change Preset")) {								// Button changes preset instantly.
		gameWrapper->Execute([this](GameWrapper* gw) { cvarManager->executeCommand("ChangePreset"); });
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Based on above switch, shuffles randomly OR changes to next enabled preset.");
	}

	CVarWrapper rangeCvar = cvarManager->getCvar("game_range");
	CVarWrapper countCvar = cvarManager->getCvar("game_count");
	if (!rangeCvar) { return; }
	int rangeMax = rangeCvar.getIntValue();
	if (ImGui::SliderInt("Number of Games", &rangeMax, 1, 25)) {		// Slider determines game range.
		rangeCvar.setValue(rangeMax);
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Select the checkboxes next to the presets you want to be enabled! All unselected presets will not be considered.");
	
	ImGui::BeginChild("PresetMenu", ImVec2(300, 300), true, ImGuiWindowFlags_None);
	for (const auto name : nameVec) {
		bool selected = presetMap.at(name);
		if (ImGui::Checkbox(name.c_str(), &selected)) {					// Checkboxes for each preset determine availability for shuffle.
			presetMap.insert_or_assign(name,selected);
		}
	}
	ImGui::EndChild();

	/* DEBUG
	if (ImGui::Button("Check State")) {									// Button logs game state and relevant info.
		gameWrapper->Execute([this](GameWrapper* gw) { cvarManager->executeCommand("CheckState"); });
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Check Game State");
	}
	*/
}