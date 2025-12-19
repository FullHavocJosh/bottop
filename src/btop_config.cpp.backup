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

#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <locale>
#include <optional>
#include <ranges>
#include <string_view>
#include <utility>

#include <fmt/core.h>
#include <sys/statvfs.h>

#include "btop_config.hpp"
#include "btop_shared.hpp"
#include "btop_tools.hpp"

using std::array;
using std::atomic;
using std::string_view;

namespace fs = std::filesystem;
namespace rng = std::ranges;

using namespace std::literals;
using namespace Tools;

//* Functions and variables for reading and writing the bottop config file
namespace Config {

	atomic<bool> locked (false);
	atomic<bool> writelock (false);
	bool write_new;

	const vector<array<string, 2>> descriptions = {
		{"color_theme", 		"#* Name of a bottop formatted \".theme\" file, \"Default\" and \"TTY\" for builtin themes.\n"
								"#* Themes should be placed in \"../share/bottop/themes\" relative to binary or \"$HOME/.config/bottop/themes\""},

		{"theme_background", 	"#* If the theme set background should be shown, set to False if you want terminal background transparency."},

		{"truecolor", 			"#* Sets if 24-bit truecolor should be used, will convert 24-bit colors to 256 color (6x6x6 color cube) if false."},

		{"force_tty", 			"#* Set to true to force tty mode regardless if a real tty has been detected or not.\n"
								"#* Will force 16-color mode and TTY theme, set all graph symbols to \"tty\" and swap out other non tty friendly symbols."},

		{"presets",				"#* Define presets for the layout of the boxes. Preset 0 is always all boxes shown with default settings. Max 9 presets.\n"
								"#* Format: \"box_name:P:G,box_name:P:G\" P=(0 or 1) for alternate positions, G=graph symbol to use for box.\n"
								"#* Use whitespace \" \" as separator between different presets.\n"
								"#* Example: \"cpu:0:default,mem:0:tty,proc:1:default cpu:0:braille,proc:0:tty\""},

		{"vim_keys",			"#* Set to True to enable \"h,j,k,l,g,G\" keys for directional control in lists.\n"
								"#* Conflicting keys for h:\"help\" and k:\"kill\" is accessible while holding shift."},

		{"rounded_corners",		"#* Rounded corners on boxes, is ignored if TTY mode is ON."},

		{"terminal_sync", 		"#* Use terminal synchronized output sequences to reduce flickering on supported terminals."},

		{"graph_symbol", 		"#* Default symbols to use for graph creation, \"braille\", \"block\" or \"tty\".\n"
								"#* \"braille\" offers the highest resolution but might not be included in all fonts.\n"
								"#* \"block\" has half the resolution of braille but uses more common characters.\n"
								"#* \"tty\" uses only 3 different symbols but will work with most fonts and should work in a real TTY.\n"
								"#* Note that \"tty\" only has half the horizontal resolution of the other two, so will show a shorter historical view."},

		{"graph_symbol_cpu", 	"# Graph symbol to use for graphs in cpu box, \"default\", \"braille\", \"block\" or \"tty\"."},
#ifdef GPU_SUPPORT
		{"graph_symbol_gpu", 	"# Graph symbol to use for graphs in gpu box, \"default\", \"braille\", \"block\" or \"tty\"."},
#endif
		{"graph_symbol_mem", 	"# Graph symbol to use for graphs in cpu box, \"default\", \"braille\", \"block\" or \"tty\"."},

		{"graph_symbol_net", 	"# Graph symbol to use for graphs in cpu box, \"default\", \"braille\", \"block\" or \"tty\"."},

		{"graph_symbol_proc", 	"# Graph symbol to use for graphs in cpu box, \"default\", \"braille\", \"block\" or \"tty\"."},

		{"shown_boxes", 		"#* Manually set which boxes to show. Available values are \"azerothcore cpu mem net proc\" and \"gpu0\" through \"gpu5\", separate values with whitespace.\n"
								"#* Default is \"azerothcore\" to show only remote AzerothCore monitoring. Add \"cpu mem net proc\" to enable local system monitoring."},

		{"update_ms", 			"#* Update time in milliseconds, recommended 2000 ms or above for better sample times for graphs."},

		{"proc_sorting",		"#* Processes sorting, \"pid\" \"program\" \"arguments\" \"threads\" \"user\" \"memory\" \"cpu lazy\" \"cpu direct\",\n"
								"#* \"cpu lazy\" sorts top process over time (easier to follow), \"cpu direct\" updates top process directly."},

		{"proc_reversed",		"#* Reverse sorting order, True or False."},

		{"proc_tree",			"#* Show processes as a tree."},

		{"proc_colors", 		"#* Use the cpu graph colors in the process list."},

		{"proc_gradient", 		"#* Use a darkening gradient in the process list."},

		{"proc_per_core", 		"#* If process cpu usage should be of the core it's running on or usage of the total available cpu power."},

		{"proc_mem_bytes", 		"#* Show process memory as bytes instead of percent."},

		{"proc_cpu_graphs",     "#* Show cpu graph for each process."},

		{"proc_info_smaps",		"#* Use /proc/[pid]/smaps for memory information in the process info box (very slow but more accurate)"},

		{"proc_left",			"#* Show proc box on left side of screen instead of right."},

		{"proc_filter_kernel",  "#* (Linux) Filter processes tied to the Linux kernel(similar behavior to htop)."},

		{"proc_aggregate",		"#* In tree-view, always accumulate child process resources in the parent process."},

		{"keep_dead_proc_usage", "#* Should cpu and memory usage display be preserved for dead processes when paused."},

		{"cpu_graph_upper", 	"#* Sets the CPU stat shown in upper half of the CPU graph, \"total\" is always available.\n"
								"#* Select from a list of detected attributes from the options menu."},

		{"cpu_graph_lower", 	"#* Sets the CPU stat shown in lower half of the CPU graph, \"total\" is always available.\n"
								"#* Select from a list of detected attributes from the options menu."},
	#ifdef GPU_SUPPORT
		{"custom_gpu_name5",	"#* Custom gpu5 model name, empty string to disable."},
	#endif
	#ifdef AZEROTHCORE_SUPPORT
		{"azerothcore_ssh_host",	"#* SSH host for AzerothCore monitoring (format: user@hostname[:port]).\n"
									"#* Can be set via environment variable: BOTTOP_AC_SSH_HOST"},
		{"azerothcore_db_host",		"#* Database host for AzerothCore monitoring.\n"
									"#* Can be set via environment variable: BOTTOP_AC_DB_HOST"},
		{"azerothcore_db_user",		"#* Database username for AzerothCore monitoring.\n"
									"#* Can be set via environment variable: BOTTOP_AC_DB_USER"},
		{"azerothcore_db_pass",		"#* Database password for AzerothCore monitoring.\n"
									"#* SECURITY: Recommended to set via environment variable: BOTTOP_AC_DB_PASS"},
		{"azerothcore_db_name",		"#* Database name for AzerothCore monitoring.\n"
									"#* Can be set via environment variable: BOTTOP_AC_DB_NAME"},
		{"azerothcore_container",	"#* Docker container name for AzerothCore server.\n"
									"#* Can be set via environment variable: BOTTOP_AC_CONTAINER"},
		{"azerothcore_ra_username",	"#* RA (Remote Administrator) username for WorldServer console access.\n"
									"#* Account must have GM level 3 or higher.\n"
									"#* Can be set via environment variable: BOTTOP_AC_RA_USERNAME"},
		{"azerothcore_ra_password",	"#* RA (Remote Administrator) password for WorldServer console access.\n"
									"#* SECURITY: Recommended to set via environment variable: BOTTOP_AC_RA_PASSWORD"},
		{"azerothcore_config_path",	"#* Path to worldserver.conf on remote server for expected values (optional)."},
	#endif
	};

	//? ============================================================================
	//? BOTTOP NOTE: Config Options Status
	//? ============================================================================
	//? Most config options below are inherited from btop++ and are now UNUSED in bottop
	//? because all stock monitoring boxes (CPU/GPU/MEM/NET/PROC) have been disabled.
	//? 
	//? USED in bottop:
	//?   - color_theme, shown_boxes, graph_symbol, clock_format, log_level
	//?   - vim_keys, tty_mode, force_tty, lowcolor, rounded_corners
	//?   - theme_background, truecolor, show_uptime, background_update
	//?   - azerothcore_* (all AzerothCore-specific options)
	//?
	//? UNUSED in bottop (kept for compatibility, no effect):
	//?   - All CPU options (cpu_*, show_cpu_*, custom_cpu_name, etc.)
	//?   - All GPU options (gpu_*, custom_gpu_name*, show_gpu_info, etc.)
	//?   - All MEM options (mem_*, show_swap, swap_disk, show_disks, etc.)
	//?   - All NET options (net_*, base_10_bitrate, etc.)
	//?   - All PROC options (proc_*, selected_*, show_detailed, etc.)
	//? ============================================================================

	std::unordered_map<std::string_view, string> strings = {
		{"color_theme", "Default"},
		{"shown_boxes", "azerothcore"},
		{"graph_symbol", "braille"},
		{"presets", "cpu:1:default,proc:0:default cpu:0:default,mem:0:default,net:0:default cpu:0:block,net:0:tty"},
		{"graph_symbol_cpu", "default"},
		{"graph_symbol_gpu", "default"},
		{"graph_symbol_mem", "default"},
		{"graph_symbol_net", "default"},
		{"graph_symbol_proc", "default"},
		{"proc_sorting", "cpu lazy"},
		{"cpu_graph_upper", "Auto"},
		{"cpu_graph_lower", "Auto"},
		{"cpu_sensor", "Auto"},
		{"selected_battery", "Auto"},
		{"cpu_core_map", ""},
		{"temp_scale", "celsius"},
	#ifdef __linux__
		{"freq_mode", "first"},
	#endif
		{"clock_format", "%X"},
		{"custom_cpu_name", ""},
		{"disks_filter", ""},
		{"io_graph_speeds", ""},
		{"net_iface", ""},
		{"base_10_bitrate", "Auto"},
		{"log_level", "WARNING"},
		{"proc_filter", ""},
		{"proc_command", ""},
		{"selected_name", ""},
	#ifdef GPU_SUPPORT
		{"custom_gpu_name0", ""},
		{"custom_gpu_name1", ""},
		{"custom_gpu_name2", ""},
		{"custom_gpu_name3", ""},
		{"custom_gpu_name4", ""},
		{"custom_gpu_name5", ""},
		{"show_gpu_info", "Auto"},
		{"shown_gpus", "nvidia amd intel"},
	#endif
	#ifdef AZEROTHCORE_SUPPORT
		{"azerothcore_ssh_host", ""},
		{"azerothcore_db_host", ""},
		{"azerothcore_db_user", ""},
		{"azerothcore_db_pass", ""},
		{"azerothcore_db_name", "acore_characters"},
		{"azerothcore_container", ""},
		{"azerothcore_ra_username", ""},
		{"azerothcore_ra_password", ""},
		{"azerothcore_config_path", ""}
	#endif
	};
	std::unordered_map<std::string_view, string> stringsTmp;

	std::unordered_map<std::string_view, bool> bools = {
		{"theme_background", true},
		{"truecolor", true},
		{"rounded_corners", true},
		{"proc_reversed", false},
		{"proc_tree", false},
		{"proc_colors", true},
		{"proc_gradient", true},
		{"proc_per_core", false},
		{"proc_mem_bytes", true},
		{"proc_cpu_graphs", true},
		{"proc_info_smaps", false},
		{"proc_left", false},
		{"proc_filter_kernel", false},
		{"cpu_invert_lower", true},
		{"cpu_single_graph", false},
		{"cpu_bottom", false},
		{"show_uptime", true},
		{"show_cpu_watts", true},
		{"check_temp", true},
		{"show_coretemp", true},
		{"show_cpu_freq", true},
		{"background_update", true},
		{"mem_graphs", true},
		{"mem_below_net", false},
		{"zfs_arc_cached", true},
		{"show_swap", true},
		{"swap_disk", true},
		{"show_disks", true},
		{"only_physical", true},
		{"use_fstab", true},
		{"zfs_hide_datasets", false},
		{"show_io_stat", true},
		{"io_mode", false},
		{"base_10_sizes", false},
		{"io_graph_combined", false},
		{"net_auto", true},
		{"net_sync", true},
		{"show_battery", true},
		{"show_battery_watts", true},
		{"vim_keys", false},
		{"tty_mode", false},
		{"disk_free_priv", false},
		{"force_tty", false},
		{"lowcolor", false},
		{"show_detailed", false},
		{"proc_filtering", false},
		{"proc_aggregate", false},
		{"pause_proc_list", false},
		{"keep_dead_proc_usage", false},
	#ifdef GPU_SUPPORT
		{"nvml_measure_pcie_speeds", true},
		{"rsmi_measure_pcie_speeds", true},
		{"gpu_mirror_graph", true},
	#endif
	#ifdef AZEROTHCORE_SUPPORT
		{"azerothcore_enabled", true},
	#endif
		{"terminal_sync", true}
	};
	std::unordered_map<std::string_view, bool> boolsTmp;

	std::unordered_map<std::string_view, int> ints = {
		{"update_ms", 30000},
		{"net_download", 100},
		{"net_upload", 100},
		{"detailed_pid", 0},
		{"selected_pid", 0},
		{"selected_depth", 0},
		{"proc_start", 0},
		{"proc_selected", 0},
		{"proc_last_selected", 0}
	};
	std::unordered_map<std::string_view, int> intsTmp;

	// Returns a valid config dir or an empty optional
	// The config dir might be read only, a warning is printed, but a path is returned anyway
	[[nodiscard]] std::optional<fs::path> get_config_dir() noexcept {
		fs::path config_dir;
		{
			std::error_code error;
			if (const auto xdg_config_home = std::getenv("XDG_CONFIG_HOME"); xdg_config_home != nullptr) {
				if (fs::exists(xdg_config_home, error)) {
					config_dir = fs::path(xdg_config_home) / "bottop";
				}
			} else if (const auto home = std::getenv("HOME"); home != nullptr) {
				error.clear();
				if (fs::exists(home, error)) {
					config_dir = fs::path(home) / ".config" / "bottop";
				}
				if (error) {
					fmt::print(stderr, "\033[0;31mWarning: \033[0m{} could not be accessed: {}\n", config_dir.string(), error.message());
					config_dir = "";
				}
			}
		}

		// FIXME: This warnings can be noisy if the user deliberately has a non-writable config dir
		//  offer an alternative | disable messages by default | disable messages if config dir is not writable | disable messages with a flag
		// FIXME: Make happy path not branch
		if (not config_dir.empty()) {
			std::error_code error;
			if (fs::exists(config_dir, error)) {
				if (fs::is_directory(config_dir, error)) {
					struct statvfs stats {};
					if ((fs::status(config_dir, error).permissions() & fs::perms::owner_write) == fs::perms::owner_write and
						statvfs(config_dir.c_str(), &stats) == 0 and (stats.f_flag & ST_RDONLY) == 0) {
						return config_dir;
					} else {
						fmt::print(stderr, "\033[0;31mWarning: \033[0m`{}` is not writable\n", fs::absolute(config_dir).string());
						// If the config is readable we can still use the provided config, but changes will not be persistent
						if ((fs::status(config_dir, error).permissions() & fs::perms::owner_read) == fs::perms::owner_read) {
							fmt::print(stderr, "\033[0;31mWarning: \033[0mLogging is disabled, config changes are not persistent\n");
							return config_dir;
						}
					}
				} else {
					fmt::print(stderr, "\033[0;31mWarning: \033[0m`{}` is not a directory\n", fs::absolute(config_dir).string());
				}
			} else {
				// Doesn't exist
				if (fs::create_directories(config_dir, error)) {
					return config_dir;
				} else {
					fmt::print(stderr, "\033[0;31mWarning: \033[0m`{}` could not be created: {}\n", fs::absolute(config_dir).string(), error.message());
				}
			}
		} else {
			fmt::print(stderr, "\033[0;31mWarning: \033[0mCould not determine config path: Make sure `$XDG_CONFIG_HOME` or `$HOME` is set\n");
		}
		fmt::print(stderr, "\033[0;31mWarning: \033[0mLogging is disabled, config changes are not persistent\n");
		return {};
	}

	bool _locked(const std::string_view name) {
		atomic_wait(writelock, true);
		if (not write_new and rng::find_if(descriptions, [&name](const auto& a) { return a.at(0) == name; }) != descriptions.end())
			write_new = true;
		return locked.load();
	}

	fs::path conf_dir;
	fs::path conf_file;

	vector<string> available_batteries = {"Auto"};

	vector<string> current_boxes;
	vector<string> preset_list = {"cpu:0:default,mem:0:default,net:0:default,proc:0:default"};
	int current_preset = -1;

	bool presetsValid(const string& presets) {
		vector<string> new_presets = {preset_list.at(0)};

		for (int x = 0; const auto& preset : ssplit(presets)) {
			if (++x > 9) {
				validError = "Too many presets entered!";
				return false;
			}
			for (int y = 0; const auto& box : ssplit(preset, ',')) {
				if (++y > 4) {
					validError = "Too many boxes entered for preset!";
					return false;
				}
				const auto& vals = ssplit(box, ':');
				if (vals.size() != 3) {
					validError = "Malformatted preset in config value presets!";
					return false;
				}
				if (not is_in(vals.at(0), "cpu", "mem", "net", "proc", "gpu0", "gpu1", "gpu2", "gpu3", "gpu4", "gpu5")) {
					validError = "Invalid box name in config value presets!";
					return false;
				}
				if (not is_in(vals.at(1), "0", "1")) {
					validError = "Invalid position value in config value presets!";
					return false;
				}
				if (not v_contains(valid_graph_symbols_def, vals.at(2))) {
					validError = "Invalid graph name in config value presets!";
					return false;
				}
			}
			new_presets.push_back(preset);
		}

		preset_list = std::move(new_presets);
		return true;
	}

	//* Apply selected preset
	bool apply_preset(const string& preset) {
		string boxes;

		for (const auto& box : ssplit(preset, ',')) {
			const auto& vals = ssplit(box, ':');
			boxes += vals.at(0) + ' ';
		}
		if (not boxes.empty()) boxes.pop_back();

		auto min_size = Term::get_min_size(boxes);
		if (Term::width < min_size.at(0) or Term::height < min_size.at(1)) {
			return false;
		}

		for (const auto& box : ssplit(preset, ',')) {
			const auto& vals = ssplit(box, ':');
			if (vals.at(0) == "cpu") {
				set("cpu_bottom", (vals.at(1) != "0"));
			} else if (vals.at(0) == "mem") {
				set("mem_below_net", (vals.at(1) != "0"));
			} else if (vals.at(0) == "proc") {
				set("proc_left", (vals.at(1) != "0"));
			}
			if (vals.at(0).starts_with("gpu")) {
				set("graph_symbol_gpu", vals.at(2));
			} else {
				set(strings.find("graph_symbol_" + vals.at(0))->first, vals.at(2));
			}
		}

		if (set_boxes(boxes)) {
			set("shown_boxes", boxes);
			return true;
		}
		return false;
	}

	void lock() {
		atomic_wait(writelock);
		locked = true;
	}

	string validError;

	bool intValid(const std::string_view name, const string& value) {
		int i_value;
		try {
			i_value = stoi(value);
		}
		catch (const std::invalid_argument&) {
			validError = "Invalid numerical value!";
			return false;
		}
		catch (const std::out_of_range&) {
			validError = "Value out of range!";
			return false;
		}
		catch (const std::exception& e) {
			validError = string{e.what()};
			return false;
		}

		if (name == "update_ms" and i_value < 100)
			validError = "Config value update_ms set too low (<100).";

		else if (name == "update_ms" and i_value > ONE_DAY_MILLIS)
			validError = fmt::format("Config value update_ms set too high (>{}).", ONE_DAY_MILLIS);

		else
			return true;

		return false;
	}

	bool validBoxSizes(const string& boxes) {
		auto min_size = Term::get_min_size(boxes);
		return (Term::width >= min_size.at(0) and Term::height >= min_size.at(1));
	}

	bool stringValid(const std::string_view name, const string& value) {
		if (name == "log_level" and not v_contains(Logger::log_levels, value))
			validError = "Invalid log_level: " + value;

		else if (name == "graph_symbol" and not v_contains(valid_graph_symbols, value))
			validError = "Invalid graph symbol identifier: " + value;

		else if (name.starts_with("graph_symbol_") and (value != "default" and not v_contains(valid_graph_symbols, value)))
			validError = fmt::format("Invalid graph symbol identifier for {}: {}", name, value);

		else if (name == "shown_boxes" and not Global::init_conf) {
			if (value.empty())
				validError = "No boxes selected!";
			else if (not validBoxSizes(value))
				validError = "Terminal too small to display entered boxes!";
			else if (not set_boxes(value))
				validError = "Invalid box name(s) in shown_boxes!";
			else
				return true;
		}

	#ifdef GPU_SUPPORT
		else if (name == "show_gpu_info" and not v_contains(show_gpu_values, value))
			validError = "Invalid value for show_gpu_info: " + value;
	#endif

		else if (name == "presets" and not presetsValid(value))
			return false;

		else if (name == "cpu_core_map") {
			const auto maps = ssplit(value);
			bool all_good = true;
			for (const auto& map : maps) {
				const auto map_split = ssplit(map, ':');
				if (map_split.size() != 2)
					all_good = false;
				else if (not isint(map_split.at(0)) or not isint(map_split.at(1)))
					all_good = false;

				if (not all_good) {
					validError = "Invalid formatting of cpu_core_map!";
					return false;
				}
			}
			return true;
		}
		else if (name == "io_graph_speeds") {
			const auto maps = ssplit(value);
			bool all_good = true;
			for (const auto& map : maps) {
				const auto map_split = ssplit(map, ':');
				if (map_split.size() != 2)
					all_good = false;
				else if (map_split.at(0).empty() or not isint(map_split.at(1)))
					all_good = false;

				if (not all_good) {
					validError = "Invalid formatting of io_graph_speeds!";
					return false;
				}
			}
			return true;
		}

		else
			return true;

		return false;
	}

	string getAsString(const std::string_view name) {
		if (auto it = bools.find(name); it != bools.end())
			return it->second ? "True" : "False";
		if (auto it = ints.find(name); it != ints.end())
			return to_string(it->second);
		if (auto it = strings.find(name); it != strings.end())
			return it->second;
		return "";
	}

	void flip(const std::string_view name) {
		if (_locked(name)) {
			if (boolsTmp.contains(name)) boolsTmp.at(name) = not boolsTmp.at(name);
			else boolsTmp.insert_or_assign(name, (not bools.at(name)));
		}
		else bools.at(name) = not bools.at(name);
	}

	void unlock() {
		if (not locked) return;
		atomic_wait(Runner::active);
		atomic_lock lck(writelock, true);
		try {
			if (Proc::shown) {
				ints.at("selected_pid") = Proc::selected_pid;
				strings.at("selected_name") = Proc::selected_name;
				ints.at("proc_start") = Proc::start;
				ints.at("proc_selected") = Proc::selected;
				ints.at("selected_depth") = Proc::selected_depth;
			}

			for (auto& item : stringsTmp) {
				strings.at(item.first) = item.second;
			}
			stringsTmp.clear();

			for (auto& item : intsTmp) {
				ints.at(item.first) = item.second;
			}
			intsTmp.clear();

			for (auto& item : boolsTmp) {
				bools.at(item.first) = item.second;
			}
			boolsTmp.clear();
		}
		catch (const std::exception& e) {
			Global::exit_error_msg = "Exception during Config::unlock() : " + string{e.what()};
			clean_quit(1);
		}

		locked = false;
	}

	bool set_boxes(const string& boxes) {
		auto new_boxes = ssplit(boxes);
		for (auto& box : new_boxes) {
			if (not v_contains(valid_boxes, box)) {
				Logger::debug("set_boxes: Invalid box name: " + box);
				return false;
			}
		#ifdef GPU_SUPPORT
			if (box.starts_with("gpu")) {
				int gpu_num = stoi(box.substr(3)) + 1;
				if (gpu_num > Gpu::count) return false;
			}
		#endif
		}
		current_boxes = std::move(new_boxes);
		Logger::debug("set_boxes: Successfully set boxes to: " + boxes);
		for (const auto& b : current_boxes) {
			Logger::debug("  - Box: " + b);
		}
		return true;
	}

	bool toggle_box(const string& box) {
		auto old_boxes = current_boxes;
		auto box_pos = rng::find(current_boxes, box);
		if (box_pos == current_boxes.end())
			current_boxes.push_back(box);
		else
			current_boxes.erase(box_pos);

		string new_boxes;
		if (not current_boxes.empty()) {
			for (const auto& b : current_boxes) new_boxes += b + ' ';
			new_boxes.pop_back();
		}

		auto min_size = Term::get_min_size(new_boxes);

		if (Term::width < min_size.at(0) or Term::height < min_size.at(1)) {
			current_boxes = old_boxes;
			return false;
		}

		Config::set("shown_boxes", new_boxes);
		return true;
	}

	void load_env_overrides() {
		// Load sensitive settings from environment variables
		// This allows keeping passwords out of config files
		#ifdef AZEROTHCORE_SUPPORT
		if (const char* env_val = std::getenv("BOTTOP_AC_DB_PASS")) {
			strings["azerothcore_db_pass"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_DB_USER")) {
			strings["azerothcore_db_user"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_SSH_HOST")) {
			strings["azerothcore_ssh_host"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_DB_HOST")) {
			strings["azerothcore_db_host"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_DB_NAME")) {
			strings["azerothcore_db_name"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_CONTAINER")) {
			strings["azerothcore_container"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_RA_USERNAME")) {
			strings["azerothcore_ra_username"] = env_val;
		}
		if (const char* env_val = std::getenv("BOTTOP_AC_RA_PASSWORD")) {
			strings["azerothcore_ra_password"] = env_val;
		}
		#endif
	}

	void load(const fs::path& conf_file, vector<string>& load_warnings) {
		std::error_code error;
		if (conf_file.empty())
			return;
		else if (not fs::exists(conf_file, error)) {
			write_new = true;
			// Load environment variables even if config file doesn't exist
			load_env_overrides();
			return;
		}
		if (error) {
			return;
		}

		std::ifstream cread(conf_file);
		if (cread.good()) {
			vector<string> valid_names;
			valid_names.reserve(descriptions.size());
			for (const auto &n : descriptions)
				valid_names.push_back(n[0]);
			if (string v_string; cread.peek() != '#' or (getline(cread, v_string, '\n') and not v_string.contains(Global::Version)))
				write_new = true;
			while (not cread.eof()) {
				cread >> std::ws;
				if (cread.peek() == '#') {
					cread.ignore(SSmax, '\n');
					continue;
				}
				string name, value;
				getline(cread, name, '=');
				if (name.ends_with(' ')) name = trim(name);
				if (not v_contains(valid_names, name)) {
					cread.ignore(SSmax, '\n');
					continue;
				}
				cread >> std::ws;

				if (bools.contains(name)) {
					cread >> value;
					if (not isbool(value))
						load_warnings.push_back("Got an invalid bool value for config name: " + name);
					else
						bools.at(name) = stobool(value);
				}
				else if (ints.contains(name)) {
					cread >> value;
					if (not isint(value))
						load_warnings.push_back("Got an invalid integer value for config name: " + name);
					else if (not intValid(name, value)) {
						load_warnings.push_back(validError);
					}
					else
						ints.at(name) = stoi(value);
				}
				else if (strings.contains(name)) {
					if (cread.peek() == '"') {
						cread.ignore(1);
						getline(cread, value, '"');
					}
					else cread >> value;

					if (not stringValid(name, value))
						load_warnings.push_back(validError);
					else
						strings.at(name) = value;
				}

				cread.ignore(SSmax, '\n');
			}

			if (not load_warnings.empty()) write_new = true;
		}
		
		// Load environment variable overrides after reading config file
		// This allows environment variables to override config file values
		load_env_overrides();
	}

	void write() {
		if (conf_file.empty() or not write_new) return;
		Logger::debug("Writing new config file");
		if (geteuid() != Global::real_uid and seteuid(Global::real_uid) != 0) return;
		std::ofstream cwrite(conf_file, std::ios::trunc);
		cwrite.imbue(std::locale::classic());
		if (cwrite.good()) {
			cwrite << "#? Config file for bottop v. " << Global::Version << "\n";
			#ifdef AZEROTHCORE_SUPPORT
			cwrite << "\n#* SECURITY NOTE: Sensitive values can be set via environment variables:\n"
					<< "#* - BOTTOP_AC_DB_PASS    (database password)\n"
					<< "#* - BOTTOP_AC_DB_USER    (database username)\n"
					<< "#* - BOTTOP_AC_SSH_HOST   (SSH connection string)\n"
					<< "#* - BOTTOP_AC_DB_HOST    (database host)\n"
					<< "#* - BOTTOP_AC_DB_NAME    (database name)\n"
					<< "#* - BOTTOP_AC_CONTAINER  (docker container name)\n"
					<< "#* - BOTTOP_AC_RA_USERNAME (RA console username)\n"
					<< "#* - BOTTOP_AC_RA_PASSWORD (RA console password)\n"
					<< "#* Add these to ~/.zshrc_envvars to keep passwords out of this config file.\n";
			#endif
			for (const auto& [name, description] : descriptions) {
				// Skip writing sensitive values if they come from environment variables
				bool skip_value = false;
				#ifdef AZEROTHCORE_SUPPORT
				if (name == "azerothcore_db_pass" && std::getenv("BOTTOP_AC_DB_PASS")) {
					skip_value = true;
				}
				#endif
				
				cwrite << "\n" << (description.empty() ? "" : description + "\n")
						<< name << " = ";
				if (skip_value) {
					cwrite << "\"<set via BOTTOP_AC_DB_PASS environment variable>\"";
				}
				else if (strings.contains(name))
					cwrite << "\"" << strings.at(name) << "\"";
				else if (ints.contains(name))
					cwrite << ints.at(name);
				else if (bools.contains(name))
					cwrite << (bools.at(name) ? "True" : "False");
				cwrite << "\n";
			}
		}
	}

	static constexpr auto get_xdg_state_dir() -> std::optional<fs::path> {
		std::optional<fs::path> xdg_state_home;

		{
			const auto* xdg_state_home_ptr = std::getenv("XDG_STATE_HOME");
			if (xdg_state_home_ptr != nullptr) {
				xdg_state_home = std::make_optional(fs::path(xdg_state_home_ptr));
			} else {
				const auto* home_ptr = std::getenv("HOME");
				if (home_ptr != nullptr) {
					xdg_state_home = std::make_optional(fs::path(home_ptr) / ".local" / "state");
				}
			}
		}

		if (xdg_state_home.has_value()) {
			std::error_code err;
			fs::create_directories(xdg_state_home.value(), err);
			if (err) {
				return std::nullopt;
			}
			return xdg_state_home;
		}
		return std::nullopt;
	}

	auto get_log_file() -> std::optional<fs::path> {
		return get_xdg_state_dir().transform([](auto&& state_home) -> auto { return state_home / "bottop.log"; });
	}
}
