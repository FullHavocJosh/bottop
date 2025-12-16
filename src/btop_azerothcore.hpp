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

#pragma once

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <atomic>
#include <unordered_map>

namespace AzerothCore {

	//* Hardcoded WotLK Zone ID to Name mapping
	//* Source: https://wowpedia.fandom.com/wiki/AreaId
	const std::unordered_map<int, std::string> ZONE_NAMES = {
		// Classic Eastern Kingdoms
		{1, "Dun Morogh"}, {3, "Badlands"}, {4, "Blasted Lands"}, {8, "Swamp of Sorrows"},
		{10, "Duskwood"}, {11, "Wetlands"}, {12, "Elwynn Forest"}, {14, "Durotar"},
		{15, "Dustwallow Marsh"}, {16, "Azshara"}, {17, "Northern Barrens"}, {28, "Western Plaguelands"},
		{33, "Stranglethorn Vale"}, {36, "Alterac Mountains"}, {38, "Loch Modan"}, {40, "Westfall"},
		{41, "Deadwind Pass"}, {44, "Redridge Mountains"}, {45, "Arathi Highlands"}, {46, "Burning Steppes"},
		{47, "The Hinterlands"}, {51, "Searing Gorge"}, {85, "Tirisfal Glades"}, {130, "Silverpine Forest"},
		{139, "Eastern Plaguelands"}, {141, "Teldrassil"}, {148, "Darkshore"}, {215, "Mulgore"},
		{267, "Hillsbrad Foothills"}, {331, "Ashenvale"}, {357, "Feralas"}, {361, "Felwood"},
		{400, "Thousand Needles"}, {405, "Desolace"}, {406, "Stonetalon Mountains"}, {440, "Tanaris"},
		{490, "Un'Goro Crater"}, {493, "Moonglade"}, {618, "Winterspring"}, {717, "The Stockade"},
		{718, "Wailing Caverns"}, {719, "Blackfathom Deeps"}, {721, "Gnomeregan"}, {722, "Razorfen Kraul"},
		{796, "Scarlet Monastery"}, {1176, "Zul'Farrak"}, {1337, "Uldaman"}, {1377, "Silithus"},
		{1497, "Undercity"}, {1519, "Stormwind City"}, {1537, "Ironforge"}, {1577, "The Cape of Stranglethorn"},
		{1637, "Orgrimmar"}, {1638, "Thunder Bluff"}, {1657, "Darnassus"}, {1977, "Zul'Gurub"},
		{2017, "Stratholme"}, {2557, "Dire Maul"}, {2597, "Alterac Valley"}, {3277, "Warsong Gulch"},
		{3358, "Arathi Basin"}, {3428, "Ahn'Qiraj"}, {3429, "Ruins of Ahn'Qiraj"},
		
		// The Burning Crusade
		{3430, "Eversong Woods"}, {3433, "Ghostlands"}, {3456, "Naxxramas"}, {3457, "Karazhan"},
		{3483, "Hellfire Peninsula"}, {3487, "Silvermoon City"}, {3518, "Nagrand"}, {3519, "Terokkar Forest"},
		{3520, "Shadowmoon Valley"}, {3521, "Zangarmarsh"}, {3522, "Blade's Edge Mountains"},
		{3523, "Netherstorm"}, {3524, "Azuremyst Isle"}, {3525, "Bloodmyst Isle"}, {3557, "The Exodar"},
		{3562, "Hellfire Ramparts"}, {3606, "Hyjal Summit"}, {3607, "Serpentshrine Cavern"},
		{3618, "Gruul's Lair"}, {3702, "The Shattered Halls"}, {3713, "The Blood Furnace"},
	{3717, "The Steamvault"}, {3789, "Shadow Labyrinth"}, {3790, "Auchenai Crypts"},
	{3791, "Sethekk Halls"}, {3792, "Mana-Tombs"}, {3847, "The Slave Pens"}, {3849, "The Underbog"},
	{3703, "The Mechanar"}, {3711, "The Botanica"},
	
	// Wrath of the Lich King  
	{65, "Dragonblight"}, {66, "Zul'Drak"}, {67, "The Storm Peaks"}, {210, "Icecrown"},
	{394, "Grizzly Hills"}, {495, "Howling Fjord"}, {2817, "Crystalsong Forest"}, {3456, "Naxxramas"},
	{3520, "Shadowmoon Valley"}, {3537, "Borean Tundra"}, {4080, "Isle of Conquest"}, {4100, "The Culling of Stratholme"}, {4197, "Wintergrasp"},
	{4264, "Halls of Stone"}, {4265, "The Nexus"}, {4272, "Halls of Lightning"},
	{4273, "Ulduar"}, {4298, "Gundrak"}, {4374, "Sholazar Basin"}, {4395, "Dalaran"}, {4494, "Ahn'kahet: The Old Kingdom"},
	{4603, "Azjol-Nerub"}, {4710, "Isle of Conquest"}, {4813, "The Obsidian Sanctum"}, {4820, "Trial of the Champion"},
		{4987, "Icecrown Citadel"},
	};

	//* Zone metadata: continent, region, and expected level range
	struct ZoneMetadata {
		std::string continent;
		std::string region;
		int min_level;
		int max_level;
	};
	
	const std::unordered_map<int, ZoneMetadata> ZONE_METADATA = {
		// Eastern Kingdoms - Dun Morogh Region
		{1, {"Eastern Kingdoms", "Dun Morogh", 1, 10}},
		{38, {"Eastern Kingdoms", "Loch Modan", 10, 20}},
		{1537, {"Eastern Kingdoms", "Ironforge", 1, 60}},
		{721, {"Eastern Kingdoms", "Gnomeregan", 24, 34}},
		
		// Eastern Kingdoms - Elwynn/Westfall Region
		{12, {"Eastern Kingdoms", "Elwynn Forest", 1, 10}},
		{40, {"Eastern Kingdoms", "Westfall", 10, 20}},
		{1519, {"Eastern Kingdoms", "Stormwind City", 1, 60}},
		{717, {"Eastern Kingdoms", "The Stockade", 15, 25}},
		
		// Eastern Kingdoms - Redridge/Duskwood Region
		{44, {"Eastern Kingdoms", "Redridge Mountains", 15, 25}},
		{10, {"Eastern Kingdoms", "Duskwood", 18, 30}},
		{41, {"Eastern Kingdoms", "Deadwind Pass", 55, 60}},
		
		// Eastern Kingdoms - Stranglethorn Region
		{33, {"Eastern Kingdoms", "Stranglethorn Vale", 30, 45}},
		{1577, {"Eastern Kingdoms", "The Cape of Stranglethorn", 30, 45}},
		{1977, {"Eastern Kingdoms", "Zul'Gurub", 60, 60}},
		
		// Eastern Kingdoms - Swamp of Sorrows/Blasted Lands Region
		{8, {"Eastern Kingdoms", "Swamp of Sorrows", 45, 55}},
		{4, {"Eastern Kingdoms", "Blasted Lands", 50, 58}},
		
		// Eastern Kingdoms - Wetlands/Arathi Region
		{11, {"Eastern Kingdoms", "Wetlands", 20, 30}},
		{45, {"Eastern Kingdoms", "Arathi Highlands", 30, 40}},
		{3358, {"Eastern Kingdoms", "Arathi Basin", 20, 60}},
		
		// Eastern Kingdoms - Badlands/Searing Gorge Region
		{3, {"Eastern Kingdoms", "Badlands", 35, 45}},
		{51, {"Eastern Kingdoms", "Searing Gorge", 43, 50}},
		{46, {"Eastern Kingdoms", "Burning Steppes", 50, 58}},
		{1337, {"Eastern Kingdoms", "Uldaman", 35, 45}},
		
		// Eastern Kingdoms - Hinterlands Region
		{47, {"Eastern Kingdoms", "The Hinterlands", 40, 50}},
		
		// Eastern Kingdoms - Alterac/Hillsbrad Region
		{36, {"Eastern Kingdoms", "Alterac Mountains", 30, 40}},
		{267, {"Eastern Kingdoms", "Hillsbrad Foothills", 20, 30}},
		{2597, {"Eastern Kingdoms", "Alterac Valley", 51, 60}},
		
		// Eastern Kingdoms - Tirisfal/Silverpine Region
		{85, {"Eastern Kingdoms", "Tirisfal Glades", 1, 10}},
		{130, {"Eastern Kingdoms", "Silverpine Forest", 10, 20}},
		{1497, {"Eastern Kingdoms", "Undercity", 1, 60}},
		{796, {"Eastern Kingdoms", "Scarlet Monastery", 26, 45}},
		
		// Eastern Kingdoms - Plaguelands Region
		{28, {"Eastern Kingdoms", "Western Plaguelands", 51, 58}},
		{139, {"Eastern Kingdoms", "Eastern Plaguelands", 53, 60}},
		{2017, {"Eastern Kingdoms", "Stratholme", 58, 60}},
		{2057, {"Eastern Kingdoms", "Scholomance", 58, 60}},
		
		// Eastern Kingdoms - Quel'Thalas (Blood Elf Starting Zones)
		{3430, {"Eastern Kingdoms", "Eversong Woods", 1, 10}},
		{3433, {"Eastern Kingdoms", "Ghostlands", 10, 20}},
		{3487, {"Eastern Kingdoms", "Silvermoon City", 1, 60}},
		
		// Kalimdor - Teldrassil/Darkshore Region
		{141, {"Kalimdor", "Teldrassil", 1, 10}},
		{148, {"Kalimdor", "Darkshore", 10, 20}},
		{1657, {"Kalimdor", "Darnassus", 1, 60}},
		{719, {"Kalimdor", "Blackfathom Deeps", 18, 24}},
		
		// Kalimdor - Azuremyst (Draenei Starting Zones)
		{3524, {"Kalimdor", "Azuremyst Isle", 1, 10}},
		{3525, {"Kalimdor", "Bloodmyst Isle", 10, 20}},
		{3557, {"Kalimdor", "The Exodar", 1, 60}},
		
		// Kalimdor - Ashenvale/Felwood Region
		{331, {"Kalimdor", "Ashenvale", 18, 30}},
		{361, {"Kalimdor", "Felwood", 48, 55}},
		{493, {"Kalimdor", "Moonglade", 1, 60}},
		{2557, {"Kalimdor", "Dire Maul", 56, 60}},
		
		// Kalimdor - Winterspring Region
		{618, {"Kalimdor", "Winterspring", 53, 60}},
		
		// Kalimdor - Azshara Region
		{16, {"Kalimdor", "Azshara", 45, 55}},
		
		// Kalimdor - Durotar/Barrens Region
		{14, {"Kalimdor", "Durotar", 1, 10}},
		{17, {"Kalimdor", "Northern Barrens", 10, 25}},
		{1637, {"Kalimdor", "Orgrimmar", 1, 60}},
		{718, {"Kalimdor", "Wailing Caverns", 15, 25}},
		{722, {"Kalimdor", "Razorfen Kraul", 24, 34}},
		
		// Kalimdor - Mulgore Region
		{215, {"Kalimdor", "Mulgore", 1, 10}},
		{1638, {"Kalimdor", "Thunder Bluff", 1, 60}},
		
		// Kalimdor - Stonetalon/Desolace Region
		{406, {"Kalimdor", "Stonetalon Mountains", 15, 27}},
		{405, {"Kalimdor", "Desolace", 30, 40}},
		
		// Kalimdor - Dustwallow Marsh Region
		{15, {"Kalimdor", "Dustwallow Marsh", 35, 45}},
		
		// Kalimdor - Thousand Needles/Feralas Region
		{400, {"Kalimdor", "Thousand Needles", 25, 35}},
		{357, {"Kalimdor", "Feralas", 40, 50}},
		
		// Kalimdor - Tanaris Region
		{440, {"Kalimdor", "Tanaris", 40, 50}},
		{1176, {"Kalimdor", "Zul'Farrak", 42, 46}},
		
		// Kalimdor - Un'Goro Crater Region
		{490, {"Kalimdor", "Un'Goro Crater", 48, 55}},
		
		// Kalimdor - Silithus Region
		{1377, {"Kalimdor", "Silithus", 55, 60}},
		{3428, {"Kalimdor", "Ahn'Qiraj", 60, 60}},
		{3429, {"Kalimdor", "Ruins of Ahn'Qiraj", 60, 60}},
		
		// Outland - Hellfire Peninsula Region
		{3483, {"Outland", "Hellfire Peninsula", 58, 63}},
		{3562, {"Outland", "Hellfire Ramparts", 60, 62}},
		{3702, {"Outland", "The Shattered Halls", 70, 70}},
		{3713, {"Outland", "The Blood Furnace", 61, 63}},
		
		// Outland - Zangarmarsh Region
		{3521, {"Outland", "Zangarmarsh", 60, 64}},
		{3847, {"Outland", "The Slave Pens", 62, 64}},
		{3849, {"Outland", "The Underbog", 63, 65}},
		{3717, {"Outland", "The Steamvault", 70, 70}},
		
		// Outland - Terokkar Forest Region
		{3519, {"Outland", "Terokkar Forest", 62, 65}},
		{3789, {"Outland", "Shadow Labyrinth", 70, 70}},
		{3790, {"Outland", "Auchenai Crypts", 64, 66}},
		{3791, {"Outland", "Sethekk Halls", 67, 69}},
		{3792, {"Outland", "Mana-Tombs", 64, 66}},
		
		// Outland - Nagrand Region
		{3518, {"Outland", "Nagrand", 64, 67}},
		
		// Outland - Blade's Edge Mountains Region
		{3522, {"Outland", "Blade's Edge Mountains", 65, 68}},
		{3618, {"Outland", "Gruul's Lair", 70, 70}},
		
	// Outland - Netherstorm Region
	{3523, {"Outland", "Netherstorm", 67, 70}},
	{3703, {"Outland", "The Mechanar", 69, 70}},
	{3711, {"Outland", "The Botanica", 70, 70}},
	
	// Outland - Shadowmoon Valley Region
	{3520, {"Outland", "Shadowmoon Valley", 67, 70}},
		
		// Outland - Raids
		{3457, {"Outland", "Karazhan", 70, 70}},
		{3606, {"Outland", "Hyjal Summit", 70, 70}},
		{3607, {"Outland", "Serpentshrine Cavern", 70, 70}},
		
		// Northrend - Borean Tundra Region
		{3537, {"Northrend", "Borean Tundra", 68, 72}},
		{4265, {"Northrend", "The Nexus", 71, 73}},
		
		// Northrend - Howling Fjord Region
		{495, {"Northrend", "Howling Fjord", 68, 72}},
		
		// Northrend - Dragonblight Region
		{65, {"Northrend", "Dragonblight", 71, 74}},
		{4100, {"Northrend", "The Culling of Stratholme", 80, 80}},
		{4813, {"Northrend", "The Obsidian Sanctum", 80, 80}},
		{3456, {"Northrend", "Naxxramas", 80, 80}},
		
		// Northrend - Grizzly Hills Region
		{394, {"Northrend", "Grizzly Hills", 73, 75}},
		
	// Northrend - Zul'Drak Region
	{66, {"Northrend", "Zul'Drak", 74, 77}},
	{4298, {"Northrend", "Gundrak", 76, 78}},
	
	// Northrend - Sholazar Basin Region
	{4374, {"Northrend", "Sholazar Basin", 76, 78}},
		
		// Northrend - Storm Peaks Region
		{67, {"Northrend", "The Storm Peaks", 77, 80}},
		{4264, {"Northrend", "Halls of Stone", 77, 79}},
		{4272, {"Northrend", "Halls of Lightning", 80, 80}},
		{4273, {"Northrend", "Ulduar", 80, 80}},
		
		// Northrend - Icecrown Region
		{210, {"Northrend", "Icecrown", 77, 80}},
		{4987, {"Northrend", "Icecrown Citadel", 80, 80}},
		{4820, {"Northrend", "Trial of the Champion", 80, 80}},
		
		// Northrend - Crystalsong Forest/Dalaran Region
		{2817, {"Northrend", "Crystalsong Forest", 74, 77}},
		{4395, {"Northrend", "Dalaran", 68, 80}},
		
	// Northrend - Wintergrasp Region
	{4197, {"Northrend", "Wintergrasp", 77, 80}},
	{4080, {"Northrend", "Isle of Conquest", 77, 80}},
	{4710, {"Northrend", "Isle of Conquest", 77, 80}},
	
	// Northrend - Dungeons
	{4494, {"Northrend", "Ahn'kahet: The Old Kingdom", 73, 75}},
	{4603, {"Northrend", "Azjol-Nerub", 72, 74}},
	
	// PvP Battlegrounds (categorize under original continents)
	{3277, {"Eastern Kingdoms", "Warsong Gulch", 10, 60}},
	};

	//* Get zone name from ID, returns "Unknown Zone (ID)" if not found
	inline std::string get_zone_name(int zone_id) {
		auto it = ZONE_NAMES.find(zone_id);
		if (it != ZONE_NAMES.end()) {
			return it->second;
		}
		return "Unknown Zone (" + std::to_string(zone_id) + ")";
	}
	
	//* Get zone metadata (continent, region, and expected levels) from ID
	inline ZoneMetadata get_zone_metadata(int zone_id) {
		auto it = ZONE_METADATA.find(zone_id);
		if (it != ZONE_METADATA.end()) {
			return it->second;
		}
		return {"Unknown", "", 1, 80};  // Default for unknown zones
	}

	//* Format uptime as 2-digit display (09s, 01m, 2h, 01d)
	inline std::string format_uptime(double uptime_hours) {
		int total_seconds = (int)(uptime_hours * 3600);
		
		if (total_seconds < 60) {
			// Show seconds (09s format)
			std::string result = std::to_string(total_seconds);
			if (result.length() == 1) result = "0" + result;
			return result + "s";
		} else if (total_seconds < 3600) {
			// Show minutes (01m format)
			int minutes = total_seconds / 60;
			std::string result = std::to_string(minutes);
			if (result.length() == 1) result = "0" + result;
			return result + "m";
		} else if (total_seconds < 86400) {
			// Show hours (2h format, no leading zero for hours)
			int hours = total_seconds / 3600;
			return std::to_string(hours) + "h";
		} else {
			// Show days (01d format)
			int days = total_seconds / 86400;
			std::string result = std::to_string(days);
			if (result.length() == 1) result = "0" + result;
			return result + "d";
		}
	}

	//* Configuration for AzerothCore monitoring
	struct ServerConfig {
		std::string ssh_host = "root@testing-azerothcore.rollet.family";
		std::string db_host = "testing-ac-database";
		std::string db_user = "root";
		std::string db_pass = "password";
		std::string db_name = "acore_characters";
		std::string container = "testing-ac-worldserver";
		std::string config_path = "";  // Path to AzerothCore worldserver.conf on remote server
		std::string ra_username = "";  // RA (Remote Administrator) console username
		std::string ra_password = "";  // RA (Remote Administrator) console password
		int update_interval = 5;
		
		// InfluxDB metrics (optional - if not set, falls back to MySQL query timing)
		std::string influx_host = "";  // e.g., "127.0.0.1"
		std::string influx_port = "";  // e.g., "8086"
		std::string influx_db = "";    // e.g., "worldserver"
	};

	//* WorldServer performance metrics from "server info" command
	struct ServerPerformance {
		// Version and build info
		std::string revision;              // Git revision hash (e.g., "ece1060fa05d+")
		std::string branch;                // Branch name (e.g., "Testing-Playerbot")
		std::string build_date;            // Build date/time
		std::string build_type;            // Build type (RelWithDebInfo, Debug, Release)
		
		// Player counts
		int connected_players = 0;         // Currently connected players
		int characters_in_world = 0;       // Total characters in world (including bots)
		int connection_peak = 0;           // Peak connections this session
		
		// Uptime
		std::string uptime;                // Formatted uptime string
		long long uptime_seconds = 0;      // Total uptime in seconds
		
		// Performance metrics (all in milliseconds)
		long long update_time_diff = 0;    // Current update time diff in ms (most recent)
		long long mean = 0;                // Mean of last 500 diffs in ms
		long long median = 0;              // Median of last 500 diffs in ms
		long long p95 = 0;                 // 95th percentile in ms
		long long p99 = 0;                 // 99th percentile in ms
		long long max = 0;                 // Maximum in ms
		bool available = false;            // Whether server info data is available
		bool is_cached = false;            // Whether this data is from cache (not fresh)
	};
	
	//* Bot statistics and server performance
	struct BotStats {
		int total = 0;
		int online_players = 0;
		int max_players = 0;
		int active_connections = 0;
		double uptime_hours = 0.0;
		double update_time_avg = 0.0;  // Average update time in ms (deprecated - use ServerPerformance)
		double update_time_diff = 0.0;  // Update time diff in ms (deprecated - use ServerPerformance)
		ServerPerformance perf;         // Real server performance metrics from "server info"
	};
	
	//* Ollama chat statistics
	struct OllamaStats {
		bool enabled = false;
		int messages_per_hour = 0;     // Rate: messages per hour (last 60 minutes)
		int recent_messages = 0;       // Recent: messages in last 60 seconds
		double failure_rate_60s = 0.0; // Failure %: failure rate for last 60 seconds
	};

	//* Docker container status
	struct ContainerStatus {
		std::string name;        // Container name (e.g., "testing-ac-worldserver")
		std::string short_name;  // Display name (e.g., "worldserver")
		std::string state;       // running, exited, restarting, paused, etc.
		std::string status;      // Status message (e.g., "Up 2 hours", "Exited (0) 5 minutes ago")
		bool is_running = false; // Quick check: state == "running"
	};

	//* Continent distribution
	struct Continent {
		std::string name;
		int count = 0;
		double percent = 0.0;
	};
	
	//* Faction distribution
	struct Faction {
		std::string name;  // Alliance, Horde, Neutral
		int count = 0;
		double percent = 0.0;
	};

	//* Zone detail (breakdown by level/class when expanded)
	struct ZoneDetail {
		std::string label;      // "Level 10-20", "Warrior", etc.
		int count = 0;
		double percent = 0.0;
	};

	//* Zone health information
	struct Zone {
		int zone_id = 0;          // Zone ID for querying details
		std::string name;
		std::string continent;    // Continent name (Kalimdor, Eastern Kingdoms, Outland, Northrend)
		std::string region;       // Region/area name (empty for direct continent zones)
		int total = 0;
		int expected_min = 0;     // Zone's expected min level (from metadata)
		int expected_max = 0;     // Zone's expected max level (from metadata)
		int actual_min = 0;       // Actual lowest bot level in zone (from database)
		int actual_max = 0;       // Actual highest bot level in zone (from database)
		double alignment = 0.0;   // % of bots within expected level range
		std::vector<ZoneDetail> details;  // Level/class breakdown (loaded on demand when expanded)
		
		bool is_healthy() const { return alignment >= 80.0; }
	};

	//* Level bracket definition (min-max level range)
	struct BracketDefinition {
		int min_level;
		int max_level;
		std::string range;  // e.g., "1-10"
		
	BracketDefinition(int min, int max) : min_level(min), max_level(max) {
		if (min == max) {
			// Single level bracket: "60" not "60-60"
			range = std::to_string(min);
		} else {
			// Range bracket: "1-10"
			range = std::to_string(min) + "-" + std::to_string(max);
		}
	}
	};
	
	//* Level bracket distribution
	struct LevelBracket {
		std::string range;
		int count = 0;
		double percent = 0.0;
	};

	//* Server status enumeration
	enum class ServerStatus {
		ONLINE,      // Server is running normally
		OFFLINE,     // Server is down/not responding
		RESTARTING,  // Server is restarting
		REBUILDING,  // Server is rebuilding (database, etc.)
		ERROR        // Error connecting or querying
	};
	
	//* Complete server data snapshot
	struct ServerData {
		BotStats stats;
		OllamaStats ollama;
		std::vector<Continent> continents;
		std::vector<Faction> factions;
		std::vector<Zone> zones;
		std::vector<LevelBracket> levels;
		std::vector<ContainerStatus> containers;  // Docker container statuses
		std::string timestamp;
		std::string error;
		std::string server_url;  // SSH host URL for display
		ServerStatus status = ServerStatus::ONLINE;
		int consecutive_failures = 0;  // Track consecutive query failures
		double rebuild_progress = 0.0;  // Rebuild progress percentage (0-100)
	};
	
	//* Expected values configuration (from server .conf files)
	struct ExpectedValues {
		int bot_min = 0;
		int bot_max = 0;
		std::vector<LevelBracket> level_distribution;
		std::vector<BracketDefinition> bracket_definitions;  // Actual bracket ranges from config
		bool loaded = false;
	};

	//* SSH Client wrapper for libssh2
	class SSHClient {
	public:
		SSHClient(const std::string& host);
		~SSHClient();
		
		bool connect();
		std::string execute(const std::string& command);
		bool is_connected() const;
		std::string last_error() const;
		
	private:
		std::string host_;
		void* session_ = nullptr;  // LIBSSH2_SESSION*
		int sock_ = -1;
		std::string error_;
	};

	//* Query handler for AzerothCore bot data
	class Query {
	public:
		Query(SSHClient& ssh, const ServerConfig& config);
		
		ServerData fetch_all();
		std::vector<ZoneDetail> fetch_zone_details(int zone_id);  // Fetch level breakdown for a zone
		std::pair<bool, double> check_rebuild_status();  // Check if rebuilding and get progress (bool=rebuilding, double=progress 0-100)
		std::vector<ContainerStatus> fetch_container_statuses();  // Fetch status of all AzerothCore containers
		
	private:
		SSHClient& ssh_;
		ServerConfig config_;
		std::string excluded_account_ids_;  // Cached list of excluded account IDs (e.g., "1,2,3,4")
		
		std::string mysql_exec(const std::string& query);
		void cache_excluded_accounts();  // Fetch and cache excluded account IDs
		std::string get_excluded_accounts_filter();  // Get WHERE clause for excluding accounts
		ServerPerformance fetch_server_performance();  // Fetch real server performance from "server info"
		BotStats fetch_bot_stats();
		std::vector<Continent> fetch_continents();
		std::vector<Faction> fetch_factions();
		std::vector<Zone> fetch_zones();
		std::vector<LevelBracket> fetch_levels();
		OllamaStats fetch_ollama_stats();
	};

	//* Global state
	extern std::atomic<bool> enabled;
	extern std::atomic<bool> active;
	extern ServerConfig config;
	extern std::unique_ptr<SSHClient> ssh_client;
	extern std::unique_ptr<Query> query;
	extern ServerData current_data;
	extern ExpectedValues expected_values;
	
	//* Historical data for graphs (server load percentage over time)
	extern std::deque<long long> load_history;
	
	//* Check if server is online (returns true if container is running)
	bool check_server_online();
	
	//* Load expected values from server config file
	void load_expected_values();

	//* Initialize AzerothCore monitoring
	void init();

	//* Update data (call from collector thread)
	void collect();

	//* Cleanup
	void cleanup();

	//* Get box content for display
	std::string get_box();

}
