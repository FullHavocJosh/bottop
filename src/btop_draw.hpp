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

#include <array>
#include <chrono>
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::array;
using std::deque;
using std::string;
using std::vector;

namespace Symbols {
	const string h_line				= "─";
	const string v_line				= "│";
	const string dotted_v_line		= "╎";
	const string left_up			= "┌";
	const string right_up			= "┐";
	const string left_down			= "└";
	const string right_down			= "┘";
	const string round_left_up		= "╭";
	const string round_right_up		= "╮";
	const string round_left_down	= "╰";
	const string round_right_down	= "╯";
	const string title_left_down	= "┘";
	const string title_right_down	= "└";
	const string title_left			= "┐";
	const string title_right		= "┌";
	const string div_right			= "┤";
	const string div_left			= "├";
	const string div_up				= "┬";
	const string div_down			= "┴";


	const string up = "↑";
	const string down = "↓";
	const string left = "←";
	const string right = "→";
	const string enter = "↵";
}

namespace Draw {

	//* Generate if needed and return the bottop banner
	string banner_gen(int y=0, int x=0, bool centered=false, bool redraw=false);

	//* An editable text field
	class TextEdit {
		size_t pos{};
		size_t upos{};
		bool numeric = false;
	public:
		string text;
		TextEdit();
		explicit TextEdit(string text, bool numeric=false);
		bool command(const std::string_view key);
		string operator()(const size_t limit=0);
		void clear();
	};

	//* Create a box and return as a string
	string createBox(
			const int x, const int y, const int width, const int height, string line_color = "", bool fill = false,
			const std::string_view title = "", const std::string_view title2 = "", const int num = 0
	);

	bool update_clock(bool force = false);

	//* Class holding a percentage meter
	class Meter {
		int width;
		string color_gradient;
		bool invert;
		array<string, 101> cache;
	public:
		Meter();
		Meter(const int width, string color_gradient, bool invert = false);

		//* Return a string representation of the meter with given value
		string operator()(int value);
	};

	//* Class holding a percentage graph
	class Graph {
		int width, height;
		string color_gradient;
		string out, symbol = "default";
		bool invert, no_zero;
		long long offset;
		long long last = 0, max_value = 0;
		bool current = true, tty_mode = false;
		std::unordered_map<bool, vector<string>> graphs = { {true, {}}, {false, {}}};

		//* Create two representations of the graph to switch between to represent two values for each braille character
		void _create(const deque<long long>& data, int data_offset);

	public:
		Graph();
		Graph(int width, int height,
			const string& color_gradient,
			const deque<long long>& data,
			const string& symbol="default",
			bool invert=false, bool no_zero=false,
			long long max_value=0, long long offset=0);

		//* Add last value from back of <data> and return string representation of graph
		string& operator()(const deque<long long>& data, bool data_same=false);

		//* Return string representation of graph
		string& operator()();
	};

	//* Calculate sizes of boxes, draw outlines and save to enabled boxes namespaces
	void calcSizes();
}

namespace Proc {
	extern Draw::TextEdit filter;
	extern std::unordered_map<size_t, Draw::Graph> p_graphs;
	extern std::unordered_map<size_t, int> p_counters;
}

#ifdef AZEROTHCORE_SUPPORT
namespace Draw {
	namespace AzerothCore {
		extern int width_p, height_p;
		extern int min_width, min_height;
		extern int x, y, width, height;
		extern bool shown, redraw;
		extern string box;
		
		//* Three-pane layout (SERVER PERFORMANCE left, BOT DISTRIBUTION right, ZONES bottom)
		extern int perf_width, perf_height;        // Left pane (server performance)
		extern int perf_x, perf_y;
		extern int dist_width, dist_height;        // Right pane (bot distribution)
		extern int dist_x, dist_y;
		extern int zones_height;                   // Bottom pane (zone list)
		extern int zones_y;
		extern string perf_box;                    // Performance box outline
		extern string dist_box;                    // Distribution box outline
		extern string zones_box;                   // Zones box outline
		
		//* Zone list scrolling and filtering
		extern size_t zone_offset;
		extern size_t zone_select_max;
		extern Draw::TextEdit zone_filter;         // Filter for zones
		
		//* Display list for zone navigation
		struct DisplayItem {
			enum Type { CONTINENT, REGION, ZONE };
			Type type;
			size_t zone_index;  // Index in data.zones (only for ZONE type)
			string name;
		};
		extern std::vector<DisplayItem> zone_display_list;
		
		//* Zone selection and expansion
		extern int selected_zone;
		extern bool zone_selection_active;
		extern bool zone_filtering;                // Whether zone filter is active
		extern std::set<size_t> expanded_zones;
		extern std::set<std::string> expanded_continents;  // Track which continents are expanded
		
		//* Zone scrolling
		extern int zone_scroll_offset;  // First visible line in zones list
		
		//* Zone sorting
		enum class ZoneSortColumn { NONE, NAME, BOTS, MIN_LEVEL, MAX_LEVEL, ALIGNMENT };
		extern ZoneSortColumn zone_sort_column;
		extern bool zone_sort_reverse;
		
		//* Update intervals (tracked as time_t in cpp)
		extern uint64_t last_perf_update;
		extern uint64_t last_dist_update;
		extern uint64_t last_zones_update;
		
		//* Graph tracking
		extern long long last_graph_max;
		
		string draw(bool force_redraw = false, bool data_same = false);
	}
}
#endif
