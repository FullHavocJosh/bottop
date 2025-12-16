// SPDX-License-Identifier: Apache-2.0
//
// Stub implementations for system monitoring
// bottop focuses on remote AzerothCore monitoring only
// This file provides empty function implementations to satisfy linkage requirements

#include "btop_shared.hpp"
#include "btop_tools.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include <atomic>
#include <tuple>

using std::string, std::vector;
namespace fs = std::filesystem;

namespace Shared {
	// Stub variables for linkage
	long coreCount = 1;
	long page_size = 4096;
	long clk_tck = 100;
	
	void init() {
		// Minimal initialization for AzerothCore-only monitoring
		// No system monitoring initialization needed
	}
}

namespace Tools {
	// Stub function for system uptime
	double system_uptime() {
		return 0.0;
	}
}

namespace Cpu {
	// Data structures
	cpu_info current_cpu;
	fs::path cpu_sensors_path;
	
	// Stub variables for linkage
	string cpuName = "N/A";
	string cpuHz = "";
	std::tuple<int, float, long, string> current_bat = {0, 0.0f, 0L, ""};  // Correct type: tuple
	bool got_sensors = false;
	bool has_battery = false;
	bool supports_watts = false;
	bool cpu_temp_only = false;
	vector<string> available_fields = {};
	vector<string> available_sensors = {};
	std::unordered_map<int, int> core_mapping = {};
	
	// Collection function (empty - no local monitoring)
	auto collect([[maybe_unused]] bool no_update) -> cpu_info& {
		return current_cpu;
	}
	
	auto get_cpuHz() -> string {
		return "";
	}
	
	auto get_battery() -> tuple<int, float, long, string> {
		return {0, 0.0f, 0L, ""};
	}
	
	auto get_core_mapping() -> std::unordered_map<int, int> {
		return core_mapping;
	}
}

namespace Mem {
	// Data structure
	mem_info current_mem;
	
	// Stub variables for linkage
	bool has_swap = false;
	int disk_ios = 0;
	
	// Collection function (empty - no local monitoring)
	auto collect([[maybe_unused]] bool no_update) -> mem_info& {
		return current_mem;
	}
	
	auto get_disks() -> vector<string> {
		return {};
	}
	
	// Defined as uint64_t in header
	uint64_t get_totalMem() {
		return 0;
	}
}

namespace Net {
	// Data structures
	net_info empty_net;
	std::unordered_map<string, net_info> current_net;
	
	// Stub variables for linkage (defined as extern in header)
	string selected_iface = "";
	std::unordered_map<string, uint64_t> graph_max = {};  // Correct type from header
	
	// Collection function (empty - no local monitoring)
	auto collect([[maybe_unused]] bool no_update) -> net_info& {
		return empty_net;
	}
}

namespace Proc {
	// Data structures
	detail_container detailed;
	vector<proc_info> current_procs;
	std::unordered_map<size_t, proc_info> old_procs;
	
	// Stub variables for linkage (defined as extern in header)
	std::atomic<int> numpids = 0;  // Correct type from header
	int filter_found = 0;
	
	// Collection function (empty - no local monitoring)
	auto collect([[maybe_unused]] bool no_update) -> vector<proc_info>& {
		return current_procs;
	}
	
	auto get_detailed([[maybe_unused]] size_t pid) -> detail_container& {
		return detailed;
	}
}
