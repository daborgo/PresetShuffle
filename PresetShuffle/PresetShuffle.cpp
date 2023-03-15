#include "pch.h"
#include "PresetShuffle.h"
#include <random>
#include <iostream>
#include <string>
using namespace std;


BAKKESMOD_PLUGIN(PresetShuffle, "Preset Shuffle", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
bool psEnabled = true;				// Determines state of the plugin.
bool lockEnabled = false;			// Determines if preset is locked.
int gameTime = 0;					// Counter for game time in which player has a car on the field.
bool gameFlag = true;				// Determines if game has been "completed" (minimum 1 minute on-field time).
bool loading = false;				// Determines if game state is loading screen.
bool readied = false;				// Determines if all players are readied.

void PresetShuffle::onLoad() {		// Function runs on plugin load.
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("ps_enabled", "1", "Enable Preset Shuffle", true, true, 0, true, 1)	// Cvar stores plugin enabled state.
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		psEnabled = cvar.getBoolValue();
			});
	cvarManager->registerCvar("game_range", "3", "Number of games before swapping presets");		// Cvar stores range of games.
	cvarManager->registerCvar("game_count", "0", "Current game count");								// Cvar stores current game count.
	cvarManager->registerCvar("LockPreset", "0", "Lock current preset", true, true, 0, true, 1)		// Cvar stores preset lock state.
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		lockEnabled = cvar.getBoolValue();
			});
	cvarManager->registerCvar("PresetStore", "", "Stores preset info across sessions.")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		for (int i = 0; i < nameVec.size(); i++) {
			presetMap.insert_or_assign(nameVec.at(i), std::stoi(cvar.getStringValue().substr(i, 1)));
		}
			});
	cvarManager->registerNotifier("ShufflePreset", [this](std::vector<std::string> args) {			// Notifier allows shufflePreset button.
		shufflePreset();
			}, "", PERMISSION_ALL);

	loadHooks();
	updateMap();

	CVarWrapper storageCvar = cvarManager->getCvar("PresetStore");
	presetMap.clear();
	for(int i = 0; i<nameVec.size(); i++){
		presetMap.insert_or_assign(nameVec.at(i), std::stoi(storageCvar.getStringValue().substr(i, 1)));
	}

	/* DEBUG
	cvarManager->registerNotifier("CheckState", [this](std::vector<std::string> args) {				// Notifier allows checkState button (for debugging).
		checkState();
		}, "", PERMISSION_ALL);
	*/
}

void PresetShuffle::onUnload() {	// Functions runs on plugin unload.
	CVarWrapper countCvar = cvarManager->getCvar("game_count");
	CVarWrapper storageCvar = cvarManager->getCvar("PresetStore");
	countCvar.setValue(0);			// Resets game count to 0 upon exiting game.
	string set = "";
	for (const auto name : nameVec) {
		set += std::to_string(presetMap.at(name));
	}
	storageCvar.setValue(set);
}

void PresetShuffle::loadHooks() {	// Function loads game hooks.
	if (!psEnabled) { return; }
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated",		// Hooked event runs when game time is updated (once on server load, constantly when game time decreases).
		[this](std::string eventName) {
			if (lockEnabled) { return; }
			ServerWrapper server = gameWrapper->GetCurrentGameState();
			CVarWrapper countCvar = cvarManager->getCvar("game_count");
			if (!server) { return; }
			if (server.GetGameCar().IsNull()) { return; }
			int pid = server.GetPlaylist().GetPlaylistId();
			if (!(gameWrapper->IsInOnlineGame() || gameWrapper->IsInGame() && pid != 9 && pid != 19 && pid != 20 && pid != 21)) { return; }		// Null check if user is not in online game or local game (excluding training/workshop).
			gameTime++;
			if (gameTime >= 60 && gameFlag) {		// Condition determines "completed" game, updates variables.
				countCvar.setValue(countCvar.getIntValue() + 1);
				gameFlag = false;
			}
		});
	gameWrapper->HookEvent("Function TAGame.PRI_TA.ServerReadyUp",						// Hooked event runs when whole server readies.
		[this](std::string eventName) {
			readied = true;
		});
	gameWrapper->HookEvent("Function TAGame.LoadingScreen_TA.HandlePreLoadMap",			// Hooked event runs immediately before menu screen loaded.
		[this](std::string eventName) {
			//		LOG("PreLoad");
			if (!gameFlag) {		// Condition resets variables if most recent game was completed.
				gameTime = 0;
				gameFlag = true;
			}
			CVarWrapper rangeCvar = cvarManager->getCvar("game_range");
			CVarWrapper countCvar = cvarManager->getCvar("game_count");
			if (lockEnabled) {		// Condition keeps game counter from updating if preset is locked.
				countCvar.setValue(0);
			}
			if (countCvar.getIntValue() >= rangeCvar.getIntValue()) {		// Condition shuffles preset if game counter reaches game range.
				LOG("Preset switch activated!");
				shufflePreset();
			}
			readied = false;
			loading = true;
		});
	gameWrapper->HookEvent("Function TAGame.LoadingScreen_TA.HandlePostLoadMap",		// Hooked event runs when all assets are loaded by loading screen.
		[this](std::string eventName) {
			//		LOG("PostLoad");
			loading = false;
		});
	gameWrapper->HookEvent("Function TAGame.LoadoutSet_TA.AddListeners",				// Hooked event runs when garage is updated in any way.
		[this](std::string eventName) {
			updateMap();
		});
}

void PresetShuffle::shufflePreset() {	// Function shuffles preset.
	if (!psEnabled || loading) { LOG("Error");  return; }		// Null check if plugin is disabled or game state is loading screen.
	bool flag = true;
	std::vector<std::string> checked;
	for (const auto& pair : presetMap) {		// Stores checked presets in a separate vector.
		if (pair.second) {
			checked.push_back(pair.first);
			flag = false;
		}
	}
	if (flag) { return; }	// Null check for no selected presets.
	CVarWrapper countCvar = cvarManager->getCvar("game_count");
	LoadoutSaveWrapper lsw = gameWrapper->GetUserLoadoutSave();
	ArrayWrapper presets = lsw.GetPresets();					// ArrayWrapper stores preset set.
	if (checked.size() == 1) {			// Case swaps to one selected preset.
		auto it = std::find(nameVec.begin(), nameVec.end(), checked.at(0));
		auto index = std::distance(nameVec.begin(), it);
		lsw.EquipPreset(lsw.GetPreset(index));
	}
	int current = lsw.GetIndex(lsw.GetEquippedLoadout());		// int stores index of current preset
	auto index = current;
	while (index == current && checked.size() > 1) {			// Loops until different index than current loadout, prevents same preset kept after shuffle.
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> dist6(0, checked.size() - 1);	// Random distribution formula.
		int rand = dist6(rng);
		auto it = std::find(nameVec.begin(), nameVec.end(), checked.at(rand));
		index = std::distance(nameVec.begin(), it);										// Finds index of random checked preset in original map.
	}
	lsw.EquipPreset(lsw.GetPreset(index));
	countCvar.setValue(0);
	gameFlag = false;
}

void PresetShuffle::updateMap() {		// Function updates maps and vectors when garage is modified.
	if (readied || loading) { return; }			// Null checks if all players are readied or if game state is loading screen.
	LoadoutSaveWrapper lsw = gameWrapper->GetUserLoadoutSave();
	ArrayWrapper presets = lsw.GetPresets();
	if (presetMap.size() > presets.Count()) {		// Deletes extraneous presets from map.
		for (int i = presets.Count(); i < presetMap.size(); i++) {
			presetMap.erase(nameVec.at(i));
		}
	}
	nameVec.clear();
	for (int i = 0; i < presets.Count(); i++) {		// Updates containers.
		presetMap.insert({presets.Get(i).GetName(),true});
		nameVec.push_back(presets.Get(i).GetName());
	}
	CVarWrapper storageCvar = cvarManager->getCvar("PresetStore");
	string set = "";
	for (const auto name : nameVec) {
		set += std::to_string(presetMap.at(name));
	}
	storageCvar.setValue(set);
}

/* DEBUG
void PresetShuffle::checkState() {
	if (!psEnabled || loading) { return; }
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	CVarWrapper countCvar = cvarManager->getCvar("game_count");
	CVarWrapper rangeCvar = cvarManager->getCvar("game_range");
	if (!server) {
		LOG("***IN MENU***\n\t\tGame Count: " + std::to_string(countCvar.getIntValue()) + "-" + std::to_string(rangeCvar.getIntValue()) + "\n\t\tGame Flag: " + std::to_string(gameFlag) + "\n\t\tGame Time: " + std::to_string(gameTime));
		LOG(std::to_string(gameWrapper->GetUserLoadoutSave().GetIndex(gameWrapper->GetUserLoadoutSave().GetEquippedLoadout())));
		return;
	}
	int pid = server.GetPlaylist().GetPlaylistId();
	if (gameWrapper->IsInOnlineGame()) {
		LOG("IN ONLINE GAME\n\t\tGame Count: " + std::to_string(countCvar.getIntValue()) + "-" + std::to_string(rangeCvar.getIntValue()) + "\n\t\tGame Flag: " + std::to_string(gameFlag) + "\n\t\tGame Time: " + std::to_string(gameTime));
	}
	else if (gameWrapper->IsInGame() && pid != 9 && pid != 19 && pid != 20 && pid != 21) {
		LOG("IN LOCAL GAME\n\t\tGame Count: " + std::to_string(countCvar.getIntValue()) + "-" + std::to_string(rangeCvar.getIntValue()) + "\n\t\tGame Flag: " + std::to_string(gameFlag) + "\n\t\tGame Time: " + std::to_string(gameTime));
	}
}
*/