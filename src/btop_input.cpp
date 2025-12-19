/* Copyright 2021 Aristocratos (jakob@qvantnet.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

indent = tab
tab-size = 4
*/

#include <limits>
#include <ranges>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <fmt/format.h>
#include <signal.h>
#include <sys/select.h>
#include <utility>

#include "btop_input.hpp"
#include "btop_tools.hpp"
#include "btop_config.hpp"
#include "btop_shared.hpp"
#include "btop_menu.hpp"
#include "btop_draw.hpp"
#ifdef AZEROTHCORE_SUPPORT
#include "btop_azerothcore.hpp"
#endif

using namespace Tools;
using namespace std::literals; // for operator""s
namespace rng = std::ranges;

namespace Input {

	//* Map for translating key codes to readable values
	const std::unordered_map<string, string> Key_escapes = {
		{"\033",	"escape"},
		{"\x12",	"ctrl_r"},
		{"\n",		"enter"},
		{" ",		"space"},
		{"\x7f",	"backspace"},
		{"\x08",	"backspace"},
		{"[A", 		"up"},
		{"OA",		"up"},
		{"[B", 		"down"},
		{"OB",		"down"},
		{"[D", 		"left"},
		{"OD",		"left"},
		{"[C", 		"right"},
		{"OC",		"right"},
		{"[2~",		"insert"},
		{"[4h",		"insert"},
		{"[3~",		"delete"},
		{"[P",		"delete"},
		{"[H",		"home"},
		{"[1~",		"home"},
		{"[F",		"end"},
		{"[4~",		"end"},
		{"[5~",		"page_up"},
		{"[6~",		"page_down"},
		{"\t",		"tab"},
		{"[Z",		"shift_tab"},
		{"OP",		"f1"},
		{"OQ",		"f2"},
		{"OR",		"f3"},
		{"OS",		"f4"},
		{"[15~",	"f5"},
		{"[17~",	"f6"},
		{"[18~",	"f7"},
		{"[19~",	"f8"},
		{"[20~",	"f9"},
		{"[21~",	"f10"},
		{"[23~",	"f11"},
		{"[24~",	"f12"}
	};

	sigset_t signal_mask;
	std::atomic<bool> polling (false);
	array<int, 2> mouse_pos;
	std::unordered_map<string, Mouse_loc> mouse_mappings;

	deque<string> history(50, "");
	string old_filter;
	string input;

	bool poll(const uint64_t timeout) {
		atomic_lock lck(polling);
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		struct timespec wait;
		struct timespec *waitptr = nullptr;

		if(timeout != std::numeric_limits<uint64_t>::max()) {
			wait.tv_sec = timeout / 1000;
			wait.tv_nsec = (timeout % 1000) * 1000000;
			waitptr = &wait;
		}

		if(pselect(STDIN_FILENO + 1, &fds, nullptr, nullptr, waitptr, &signal_mask) > 0) {
			input.clear();
			char buf[1024];
			ssize_t count = 0;
			while((count = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
				input.append(std::string_view(buf, count));
			}

			return true;
		}

		return false;
	}

	string get() {
		string key = input;
		if (not key.empty()) {
			//? Remove escape code prefix if present
			if (key.length() > 1 and key.at(0) == Fx::e.at(0)) {
				key.erase(0, 1);
			}
			//? Detect if input is an mouse event
			if (key.starts_with("[<")) {
				std::string_view key_view = key;
				string mouse_event;
				if (key_view.starts_with("[<0;") and key_view.find('M') != std::string_view::npos) {
					mouse_event = "mouse_click";
					key_view.remove_prefix(4);
				}
				// else if (key_view.starts_with("[<0;") and key_view.ends_with('m')) {
				// 	mouse_event = "mouse_release";
				// 	key_view.remove_prefix(4);
				// }
				else if (key_view.starts_with("[<64;")) {
					mouse_event = "mouse_scroll_up";
					key_view.remove_prefix(5);
				}
				else if (key_view.starts_with("[<65;")) {
					mouse_event = "mouse_scroll_down";
					key_view.remove_prefix(5);
				}
				else
					key.clear();

				if (Config::getB("proc_filtering")) {
					if (mouse_event == "mouse_click") return mouse_event;
					else return "";
				}

				//? Get column and line position of mouse and check for any actions mapped to current position
				if (not key.empty()) {
					try {
						const auto delim = key_view.find(';');
						mouse_pos[0] = stoi((string)key_view.substr(0, delim));
						mouse_pos[1] = stoi((string)key_view.substr(delim + 1, key_view.find('M', delim)));
					}
					catch (const std::invalid_argument&) { mouse_event.clear(); }
					catch (const std::out_of_range&) { mouse_event.clear(); }

					key = mouse_event;

					if (key == "mouse_click") {
						const auto& [col, line] = mouse_pos;

						for (const auto& [mapped_key, pos] : (Menu::active ? Menu::mouse_mappings : mouse_mappings)) {
							if (col >= pos.col and col < pos.col + pos.width and line >= pos.line and line < pos.line + pos.height) {
								key = mapped_key;
								break;
							}
						}
					}
				}

			}
			else if (auto it = Key_escapes.find(key); it != Key_escapes.end())
				key = it->second;
			else if (ulen(key) > 1)
				key.clear();

			if (not key.empty()) {
				history.push_back(key);
				history.pop_front();
			}
		}
		return key;
	}

	string wait() {
		while(not poll(std::numeric_limits<uint64_t>::max())) {}
		return get();
	}

	void interrupt() {
		kill(getpid(), SIGUSR1);
	}

	void clear() {
		// do not need it, actually
	}

	void process(const std::string_view key) {
	if (key.empty()) return;
	try {
		auto filtering = Config::getB("proc_filtering");
		auto vim_keys = Config::getB("vim_keys");
		// auto help_key = (vim_keys ? "H" : "h");  // Unused in bottop
		// auto kill_key = (vim_keys ? "K" : "k");  // Unused after stock box removal
		//? Global input actions
		if (not filtering) {
			bool keep_going = false;
			if (key == "q") {
				clean_quit(0);
			}
			else if (is_in(key, "escape", "m")) {
				Menu::show(Menu::Menus::Main);
				return;
			}
		// Disabled stock btop keybinds - only q, esc, z, and ctrl_r are active
		/*
		else if (is_in(key, "f1", "?", help_key)) {
			Menu::show(Menu::Menus::Help);
			return;
		}
		else if (is_in(key, "f2", "o")) {
			Menu::show(Menu::Menus::Options);
			return;
		}
		else if (key.size() == 1 and isint(key)) {
			auto intKey = std::atoi(key.data());
		#ifdef GPU_SUPPORT
			static const array<string, 10> boxes = {"gpu5", "cpu", "mem", "net", "proc", "gpu0", "gpu1", "gpu2", "gpu3", "gpu4"};
			if ((intKey == 0 and Gpu::count < 5) or (intKey >= 5 and intKey - 4 > Gpu::count))
				return;
		#else
		static const array<string, 10> boxes = {"", "cpu", "mem", "net", "proc"};
			if (intKey == 0 or intKey > 4)
				return;
		#endif
			atomic_wait(Runner::active);

			if (not Config::toggle_box(boxes.at(intKey))) {
				Menu::show(Menu::Menus::SizeError);
				return;
			}
			Config::current_preset = -1;
			Draw::calcSizes();
			Runner::run("all", false, true);
			return;
		}
		else if (is_in(key, "p", "P") and Config::preset_list.size() > 1) {
			const auto old_preset = Config::current_preset;
			if (key == "p") {
				if (++Config::current_preset >= (int)Config::preset_list.size()) Config::current_preset = 0;
			}
			else {
				if (--Config::current_preset < 0) Config::current_preset = Config::preset_list.size() - 1;
			}
			atomic_wait(Runner::active);
			if (not Config::apply_preset(Config::preset_list.at(Config::current_preset))) {
				Menu::show(Menu::Menus::SizeError);
				Config::current_preset = old_preset;
				return;
			}
			Draw::calcSizes();
			Runner::run("all", false, true);
			return;
		} else
		*/
		//? Config reload with ctrl_r - kept active for bottop
		if (is_in(key, "ctrl_r")) {
			kill(getpid(), SIGUSR2);
			return;
		}
		else
			keep_going = true;

			if (not keep_going) return;
		}

			#ifdef AZEROTHCORE_SUPPORT
			//? Input actions for AzerothCore zone navigation - arrow keys always work
			if (Draw::AzerothCore::shown and not filtering) {
				if (key == "up" or (vim_keys and key == "k")) {
					// Auto-activate zone navigation on first use
					if (not Draw::AzerothCore::zone_selection_active) {
						Draw::AzerothCore::zone_selection_active = true;
						Draw::AzerothCore::selected_zone = 0;
					}
					if (Draw::AzerothCore::selected_zone > 0) {
						Draw::AzerothCore::selected_zone--;
						Draw::AzerothCore::redraw = true;
					}
					return;
				}
				else if (key == "down" or (vim_keys and key == "j")) {
					// Auto-activate zone navigation on first use
					if (not Draw::AzerothCore::zone_selection_active) {
						Draw::AzerothCore::zone_selection_active = true;
						Draw::AzerothCore::selected_zone = 0;
					}
					// Navigate through display list, not raw zones
					if (Draw::AzerothCore::selected_zone < (int)Draw::AzerothCore::zone_display_list.size() - 1) {
						Draw::AzerothCore::selected_zone++;
						Draw::AzerothCore::redraw = true;
					}
					return;
				}
				else if (Draw::AzerothCore::zone_selection_active and (key == "left" or (vim_keys and key == "h"))) {
					// Left arrow collapses current item
					int idx = Draw::AzerothCore::selected_zone;
					
					// Check if selected item exists in display list
					if (idx >= 0 and idx < (int)Draw::AzerothCore::zone_display_list.size()) {
						const auto& item = Draw::AzerothCore::zone_display_list[idx];
						
						if (item.type == Draw::AzerothCore::DisplayItem::CONTINENT) {
							// Collapse continent
							if (Draw::AzerothCore::expanded_continents.contains(item.name)) {
								Draw::AzerothCore::expanded_continents.erase(item.name);
								Draw::AzerothCore::redraw = true;
							}
						}
						else if (item.type == Draw::AzerothCore::DisplayItem::ZONE) {
							// First try to collapse the zone if expanded
							if (Draw::AzerothCore::expanded_zones.contains(item.zone_index)) {
								Draw::AzerothCore::expanded_zones.erase(item.zone_index);
								Draw::AzerothCore::redraw = true;
							}
							// Otherwise collapse the continent
							else {
								const auto& zone = AzerothCore::current_data.zones[item.zone_index];
								if (Draw::AzerothCore::expanded_continents.contains(zone.continent)) {
									Draw::AzerothCore::expanded_continents.erase(zone.continent);
									Draw::AzerothCore::redraw = true;
								}
							}
						}
					}
					return;
				}
				else if (Draw::AzerothCore::zone_selection_active and (key == "enter" or key == "right" or (vim_keys and key == "l") or key == "space")) {
					// Right arrow/Enter/Space expands selected item
					int idx = Draw::AzerothCore::selected_zone;
					
					// Bounds check
					if (idx < 0 or idx >= (int)Draw::AzerothCore::zone_display_list.size()) {
						return;
					}
					
					const auto& item = Draw::AzerothCore::zone_display_list[idx];
					
					if (item.type == Draw::AzerothCore::DisplayItem::CONTINENT) {
						// Expand continent
						if (!Draw::AzerothCore::expanded_continents.contains(item.name)) {
							Draw::AzerothCore::expanded_continents.insert(item.name);
							Draw::AzerothCore::redraw = true;
						}
					}
					else if (item.type == Draw::AzerothCore::DisplayItem::ZONE) {
						const auto& zone = AzerothCore::current_data.zones[item.zone_index];
						
						// If continent is collapsed, expand it first
						if (!Draw::AzerothCore::expanded_continents.contains(zone.continent)) {
							Draw::AzerothCore::expanded_continents.insert(zone.continent);
							Draw::AzerothCore::redraw = true;
							return;
						}
						
						// Continent is expanded, toggle zone expansion
						if (Draw::AzerothCore::expanded_zones.contains(item.zone_index)) {
							Draw::AzerothCore::expanded_zones.erase(item.zone_index);
						} else {
							// Load zone details if not already loaded
							auto& mutable_zone = AzerothCore::current_data.zones[item.zone_index];
							if (mutable_zone.details.empty() && AzerothCore::query) {
								try {
									mutable_zone.details = AzerothCore::query->fetch_zone_details(mutable_zone.zone_id);
									Logger::debug("Loaded " + std::to_string(mutable_zone.details.size()) + 
									              " location clusters for zone: " + mutable_zone.name);
								} catch (const std::exception& e) {
									Logger::error("Failed to fetch zone details: " + std::string(e.what()));
								}
							}
							Draw::AzerothCore::expanded_zones.insert(item.zone_index);
						}
					Draw::AzerothCore::redraw = true;
				}
				return;
			}
			// Page Up/Down for faster scrolling
			else if (key == "page_up") {
				if (not Draw::AzerothCore::zone_selection_active) {
					Draw::AzerothCore::zone_selection_active = true;
					Draw::AzerothCore::selected_zone = 0;
				}
				// Move up by page size (10 items)
				Draw::AzerothCore::selected_zone = std::max(0, Draw::AzerothCore::selected_zone - 10);
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "page_down") {
				if (not Draw::AzerothCore::zone_selection_active) {
					Draw::AzerothCore::zone_selection_active = true;
					Draw::AzerothCore::selected_zone = 0;
				}
				// Move down by page size (10 items)
				int max_pos = (int)Draw::AzerothCore::zone_display_list.size() - 1;
				Draw::AzerothCore::selected_zone = std::min(max_pos, Draw::AzerothCore::selected_zone + 10);
				Draw::AzerothCore::redraw = true;
				return;
			}
			// Home/End for jumping to start/end
			else if (key == "home") {
				if (not Draw::AzerothCore::zone_selection_active) {
					Draw::AzerothCore::zone_selection_active = true;
				}
				Draw::AzerothCore::selected_zone = 0;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "end") {
				if (not Draw::AzerothCore::zone_selection_active) {
					Draw::AzerothCore::zone_selection_active = true;
				}
				Draw::AzerothCore::selected_zone = (int)Draw::AzerothCore::zone_display_list.size() - 1;
				Draw::AzerothCore::redraw = true;
				return;
			}
			// Sorting keyboard shortcuts (n=name, b=bots, m=min level, M=max level, a=alignment, r=reverse)
			else if (key == "n") {
				Draw::AzerothCore::zone_sort_column = Draw::AzerothCore::ZoneSortColumn::NAME;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "b") {
				Draw::AzerothCore::zone_sort_column = Draw::AzerothCore::ZoneSortColumn::BOTS;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "m") {
				Draw::AzerothCore::zone_sort_column = Draw::AzerothCore::ZoneSortColumn::MIN_LEVEL;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "M") {
				Draw::AzerothCore::zone_sort_column = Draw::AzerothCore::ZoneSortColumn::MAX_LEVEL;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "a") {
				Draw::AzerothCore::zone_sort_column = Draw::AzerothCore::ZoneSortColumn::ALIGNMENT;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "r") {
				Draw::AzerothCore::zone_sort_reverse = !Draw::AzerothCore::zone_sort_reverse;
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (key == "f" and not Draw::AzerothCore::zone_filtering) {
				// Activate filter mode
				Draw::AzerothCore::zone_filtering = true;
				Draw::AzerothCore::zone_filter.clear();
				Draw::AzerothCore::redraw = true;
				return;
			}
			else if (Draw::AzerothCore::zone_filtering) {
				// Handle filter input
				if (key == "escape" or key == "enter") {
					// Exit filter mode
					Draw::AzerothCore::zone_filtering = false;
					Draw::AzerothCore::redraw = true;
					return;
				}
				else if (Draw::AzerothCore::zone_filter.command(key)) {
					// Filter text was modified
					Draw::AzerothCore::redraw = true;
					return;
				}
			}
		}
#endif

	// Disable all other stock btop input handlers - bottop only uses q, esc, and z
	return;
			
		#ifdef AZEROTHCORE_SUPPORT
			//? Input actions for AzerothCore box
			if (Draw::AzerothCore::shown) {
				using namespace Draw::AzerothCore;
				bool keep_going = false;
				bool no_update = true;
				bool redraw = true;
				
				if (is_in(key, "up", "down", "page_up", "page_down", "home", "end") or (vim_keys and is_in(key, "j", "k", "g", "G"))) {
					const auto& zones = AzerothCore::current_data.zones;
					size_t total_zones = zones.size();
					
					if (total_zones > zone_select_max) {
						if (key == "up" or (vim_keys and key == "k")) {
							if (zone_offset > 0) zone_offset--;
						}
						else if (key == "down" or (vim_keys and key == "j")) {
							if (zone_offset + zone_select_max < total_zones) zone_offset++;
						}
						else if (key == "page_up") {
							zone_offset = (zone_offset > zone_select_max) ? zone_offset - zone_select_max : 0;
						}
						else if (key == "page_down") {
							zone_offset = std::min(zone_offset + zone_select_max, total_zones - zone_select_max);
						}
						else if (key == "home" or (vim_keys and key == "g")) {
							zone_offset = 0;
						}
						else if (key == "end" or (vim_keys and key == "G")) {
							zone_offset = (total_zones > zone_select_max) ? total_zones - zone_select_max : 0;
						}
					}
				}
				else keep_going = true;
				
				if (not keep_going) {
					Runner::run("azerothcore", no_update, redraw);
					return;
				}
			}
		#endif
		}

		catch (const std::exception& e) {
			throw std::runtime_error { fmt::format(R"(Input::process("{}"))", e.what()) };
		}
	}
}
