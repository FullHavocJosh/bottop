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

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <set>
#include <fstream>

#include "btop_draw.hpp"
#include "btop_config.hpp"
#include "btop_theme.hpp"
#include "btop_shared.hpp"
#include "btop_tools.hpp"
#include "btop_input.hpp"
#include "btop_menu.hpp"
#ifdef AZEROTHCORE_SUPPORT
#include "btop_azerothcore.hpp"
#endif
#include <fmt/format.h>


using std::array;
using std::clamp;
using std::cmp_equal;
using std::cmp_greater;
using std::cmp_less;
using std::cmp_less_equal;
using std::floor;
using std::max;
using std::min;
using std::round;
using std::to_string;
using std::views::iota;

using namespace Tools;
using namespace std::literals; // for operator""s
namespace rng = std::ranges;

namespace Symbols {
	const string meter = "■";

	const array<string, 10> superscript = { "⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹" };

	const std::unordered_map<string, vector<string>> graph_symbols = {
		{ "braille_up", {
			" ", "⢀", "⢠", "⢰", "⢸",
			"⡀", "⣀", "⣠", "⣰", "⣸",
			"⡄", "⣄", "⣤", "⣴", "⣼",
			"⡆", "⣆", "⣦", "⣶", "⣾",
			"⡇", "⣇", "⣧", "⣷", "⣿"
		}},
		{"braille_down", {
			" ", "⠈", "⠘", "⠸", "⢸",
			"⠁", "⠉", "⠙", "⠹", "⢹",
			"⠃", "⠋", "⠛", "⠻", "⢻",
			"⠇", "⠏", "⠟", "⠿", "⢿",
			"⡇", "⡏", "⡟", "⡿", "⣿"
		}},
		{"block_up", {
			" ", "▗", "▗", "▐", "▐",
			"▖", "▄", "▄", "▟", "▟",
			"▖", "▄", "▄", "▟", "▟",
			"▌", "▙", "▙", "█", "█",
			"▌", "▙", "▙", "█", "█"
		}},
		{"block_down", {
			" ", "▝", "▝", "▐", "▐",
			"▘", "▀", "▀", "▜", "▜",
			"▘", "▀", "▀", "▜", "▜",
			"▌", "▛", "▛", "█", "█",
			"▌", "▛", "▛", "█", "█"
		}},
		{"tty_up", {
			" ", "░", "░", "▒", "▒",
			"░", "░", "▒", "▒", "█",
			"░", "▒", "▒", "▒", "█",
			"▒", "▒", "▒", "█", "█",
			"▒", "█", "█", "█", "█"
		}},
		{"tty_down", {
			" ", "░", "░", "▒", "▒",
			"░", "░", "▒", "▒", "█",
			"░", "▒", "▒", "▒", "█",
			"▒", "▒", "▒", "█", "█",
			"▒", "█", "█", "█", "█"
		}}
	};

}

namespace Draw {

	string banner_gen(int y, int x, bool centered, bool redraw) {
		static string banner;
		static size_t width = 0;
		if (redraw) banner.clear();
		if (banner.empty()) {
			string b_color, bg, fg, oc, letter;
			auto lowcolor = Config::getB("lowcolor");
			auto tty_mode = Config::getB("tty_mode");
			for (size_t z = 0; const auto& line : Global::Banner_src) {
				if (const auto w = ulen(line[1]); w > width) width = w;
				if (tty_mode) {
					fg = (z > 2) ? "\x1b[31m" : "\x1b[91m";
					bg = (z > 2) ? "\x1b[90m" : "\x1b[37m";
				}
				else {
					fg = Theme::hex_to_color(line[0], lowcolor);
					int bg_i = 120 - z * 12;
					bg = Theme::dec_to_color(bg_i, bg_i, bg_i, lowcolor);
				}
				for (size_t i = 0; i < line[1].size(); i += 3) {
					if (line[1][i] == ' ') {
						letter = Mv::r(1);
						i -= 2;
					}
					else
						letter = line[1].substr(i, 3);

					b_color = (letter == "█") ? fg : bg;
					if (b_color != oc) banner += b_color;
					banner += letter;
					oc = b_color;
				}
				if (++z < Global::Banner_src.size()) banner += Mv::l(ulen(line[1])) + Mv::d(1);
			}
			banner += Mv::r(18 - Global::Version.size())
					+ Theme::c("main_fg") + Fx::b + Fx::i + "v" + Global::Version + Fx::reset;
		}
		if (redraw) return "";
		return (centered ? Mv::to(y, Term::width / 2 - width / 2) : Mv::to(y, x)) + banner;
	}

	TextEdit::TextEdit() {}
	TextEdit::TextEdit(string text, bool numeric) : numeric(numeric), text(std::move(text)) {
		pos = this->text.size();
		upos = ulen(this->text);
	}

	bool TextEdit::command(const std::string_view key) {
		if (key == "left" and upos > 0) {
			upos--;
			pos = uresize(text, upos).size();
		}
		else if (key == "right" and pos < text.size()) {
			upos++;
			pos = uresize(text, upos).size();
		}
		else if (key == "home" and not text.empty() and pos > 0) {
			pos = upos = 0;
		}
		else if (key == "end" and not text.empty() and pos < text.size()) {
			pos = text.size();
			upos = ulen(text);
		}
		else if (key == "backspace" and pos > 0) {
			if (pos == text.size()) {
				text = uresize(text, --upos);
				pos = text.size();
			}
			else {
				const string first = uresize(text, --upos);
				pos = first.size();
				text = first + luresize(text.substr(pos), ulen(text) - upos - 1);
			}
		}
		else if (key == "delete" and pos < text.size()) {
			const string first = uresize(text, upos + 1);
			text = uresize(first, ulen(first) - 1) + text.substr(first.size());
		}
		else if (key == "space" and not numeric) {
			text.insert(pos++, 1, ' ');
			upos++;
		}
		else if (ulen(key) == 1 and text.size() < text.max_size() - 20) {
			if (numeric and not isint(key)) return false;
			if (key.size() == 1) {
				text.insert(pos++, 1, key.at(0));
				upos++;
			}
			else {
				const auto first = fmt::format("{}{}", uresize(text, upos), key);
				text = first + text.substr(pos);
				upos++;
				pos = first.size();
			}
		}
		else
			return false;

		return true;
	}

	string TextEdit::operator()(const size_t limit) {
		string out;
		size_t c_upos = upos;
		if (text.empty())
			return Fx::ul + " " + Fx::uul;
		if (limit > 0 and ulen(text) + 1 > limit) {
			try {
				const size_t half = (size_t)round((double)limit / 2);
				string first;

				if (upos + half > ulen(text))
					first = luresize(text.substr(0, pos), limit - (ulen(text) - upos));
				else if (upos - half < 1)
					first = text.substr(0, pos);
				else
					first = luresize(text.substr(0, pos), half);

				out = first + uresize(text.substr(pos), limit - ulen(first));
				c_upos = ulen(first);
			}
			catch (const std::exception& e) {
				Logger::error("In TextEdit::operator() : " + string{e.what()});
				return "";
			}
		}
		else
			out = text;

		if (c_upos == 0)
			return Fx::ul + uresize(out, 1) + Fx::uul + luresize(out, ulen(out) - 1);
		else if (c_upos == ulen(out))
			return out + Fx::ul + " " + Fx::uul;
		else
			return uresize(out, c_upos) + Fx::ul + luresize(uresize(out, c_upos + 1), 1) + Fx::uul + luresize(out, ulen(out) - c_upos - 1);
	}

	void TextEdit::clear() {
		this->text.clear();
	}

	string createBox(
			const int x, const int y, const int width, const int height, string line_color, bool fill, const std::string_view title,
			const std::string_view title2, const int num
	) {
		string out;

		if (line_color.empty())
			line_color = Theme::c("div_line");

		auto tty_mode = Config::getB("tty_mode");
		auto rounded = Config::getB("rounded_corners");
		const string numbering = (num == 0) ? "" : Theme::c("hi_fg") + (tty_mode ? std::to_string(num) : Symbols::superscript.at(clamp(num, 0, 9)));
		const auto& right_up = (tty_mode or not rounded ? Symbols::right_up : Symbols::round_right_up);
		const auto& left_up = (tty_mode or not rounded ? Symbols::left_up : Symbols::round_left_up);
		const auto& right_down = (tty_mode or not rounded ? Symbols::right_down : Symbols::round_right_down);
		const auto& left_down = (tty_mode or not rounded ? Symbols::left_down : Symbols::round_left_down);

		out = Fx::reset + line_color;

		//? Draw horizontal lines
		for (const int& hpos : {y, y + height - 1}) {
			out += Mv::to(hpos, x) + Symbols::h_line * (width - 1);
		}

		//? Draw vertical lines and fill if enabled
		for (const int& hpos : iota(y + 1, y + height - 1)) {
			out += Mv::to(hpos, x) + Symbols::v_line
				+  ((fill) ? string(width - 2, ' ') : Mv::r(width - 2))
				+  Symbols::v_line;
		}

		//? Draw corners
		out += 	Mv::to(y, x) + left_up
			+	Mv::to(y, x + width - 1) + right_up
			+	Mv::to(y + height - 1, x) +left_down
			+	Mv::to(y + height - 1, x + width - 1) + right_down;

		//? Draw titles if defined
		if (not title.empty()) {
			out += fmt::format(
				"{}{}{}{}{}{}{}{}{}", Mv::to(y, x + 2), Symbols::title_left, Fx::b, numbering, Theme::c("title"), title, Fx::ub,
				line_color, Symbols::title_right
			);
		}
		if (not title2.empty()) {
			out += fmt::format(
				"{}{}{}{}{}{}{}{}{}", Mv::to(y + height - 1, x + 2), Symbols::title_left_down, Fx::b, numbering, Theme::c("title"), title2, Fx::ub,
				line_color, Symbols::title_right_down
			);
		}

		return out + Fx::reset + Mv::to(y + 1, x + 1);
	}

	bool update_clock(bool force) {
		const auto& clock_format = Config::getS("clock_format");
		if (not Cpu::shown or clock_format.empty()) {
			if (clock_format.empty() and not Global::clock.empty()) Global::clock.clear();
			return false;
		}

		static const std::unordered_map<string, string> clock_custom_format = {
			{"/user", Tools::username()},
			{"/host", Tools::hostname()},
			{"/uptime", ""}
		};

		static time_t c_time{};
		static size_t clock_len{};
		static string clock_str;

		if (auto n_time = time(nullptr); not force and n_time == c_time)
			return false;
		else {
			c_time = n_time;
			const auto new_clock = Tools::strf_time(clock_format);
			if (not force and new_clock == clock_str) return false;
			clock_str = new_clock;
		}

		auto& out = Global::clock;
		auto cpu_bottom = Config::getB("cpu_bottom");
		const auto& x = Cpu::x;
		const auto y = (cpu_bottom ? Cpu::y + Cpu::height - 1 : Cpu::y);
		const auto& width = Cpu::width;
		const auto& title_left = (cpu_bottom ? Symbols::title_left_down : Symbols::title_left);
		const auto& title_right = (cpu_bottom ? Symbols::title_right_down : Symbols::title_right);


		for (const auto& [c_format, replacement] : clock_custom_format) {
			if (clock_str.contains(c_format)) {
				if (c_format == "/uptime") {
					string upstr = sec_to_dhms(system_uptime());
					if (upstr.size() > 8) upstr.resize(upstr.size() - 3);
					clock_str = s_replace(clock_str, c_format, upstr);
				}
				else {
					clock_str = s_replace(clock_str, c_format, replacement);
				}
			}

		}

		clock_str = uresize(clock_str, std::max(10, width - 66 - (Term::width >= 100 and Config::getB("show_battery") and Cpu::has_battery ? 22 : 0)));
		out.clear();

		if (clock_str.size() != clock_len) {
			if (not Global::resized and clock_len > 0)
				out = Mv::to(y, x+(width / 2)-(clock_len / 2)) + Fx::ub + Theme::c("cpu_box") + Symbols::h_line * clock_len;
			clock_len = clock_str.size();
		}

		out += Mv::to(y, x+(width / 2)-(clock_len / 2)) + Fx::ub + Theme::c("cpu_box") + title_left
			+ Theme::c("title") + Fx::b + clock_str + Theme::c("cpu_box") + Fx::ub + title_right;

		return true;
	}

	//* Meter class ------------------------------------------------------------------------------------------------------------>
	Meter::Meter() {}

	Meter::Meter(const int width, string color_gradient, bool invert)
		: width(width), color_gradient(std::move(color_gradient)), invert(invert) {}

	string Meter::operator()(int value) {
		if (width < 1) return "";
		value = clamp(value, 0, 100);
		if (not cache.at(value).empty()) return cache.at(value);
		auto& out = cache.at(value);
		for (const int& i : iota(1, width + 1)) {
			int y = round((double)i * 100.0 / width);
			if (value >= y)
				out += Theme::g(color_gradient).at(invert ? 100 - y : y) + Symbols::meter;
			else {
				out += Theme::c("meter_bg") + Symbols::meter * (width + 1 - i);
				break;
			}
		}
		out += Fx::reset;
		return out;
	}

	//* Graph class ------------------------------------------------------------------------------------------------------------>
	void Graph::_create(const deque<long long>& data, int data_offset) {
		bool mult = (data.size() - data_offset > 1);
		const auto& graph_symbol = Symbols::graph_symbols.at(symbol + '_' + (invert ? "down" : "up"));
		array<int, 2> result;
		const float mod = (height == 1) ? 0.3 : 0.1;
		long long data_value = 0;
		if (mult and data_offset > 0) {
			last = data.at(data_offset - 1);
			if (max_value > 0) last = clamp((last + offset) * 100 / max_value, 0ll, 100ll);
		}

		//? Horizontal iteration over values in <data>
		for (const int& i : iota(data_offset, (int)data.size())) {
			// if (tty_mode and mult and i % 2 != 0) continue;
			if (not tty_mode and mult) current = not current;
			if (i < 0) {
				data_value = 0;
				last = 0;
			}
			else {
				data_value = data.at(i);
				if (max_value > 0) data_value = clamp((data_value + offset) * 100 / max_value, 0ll, 100ll);
			}

			//? Vertical iteration over height of graph
			for (const int& horizon : iota(0, height)) {
				const int cur_high = (height > 1) ? round(100.0 * (height - horizon) / height) : 100;
				const int cur_low = (height > 1) ? round(100.0 * (height - (horizon + 1)) / height) : 0;
				//? Calculate previous + current value to fit two values in 1 braille character
				for (int ai = 0; const auto& value : {last, data_value}) {
					const int clamp_min = (no_zero and horizon == height - 1 and not (mult and i == data_offset and ai == 0)) ? 1 : 0;
					if (value >= cur_high)
						result[ai++] = 4;
					else if (value <= cur_low)
						result[ai++] = clamp_min;
					else {
						result[ai++] = clamp((int)round((float)(value - cur_low) * 4 / (cur_high - cur_low) + mod), clamp_min, 4);
					}
				}
				//? Generate graph symbol from 5x5 2D vector
				if (height == 1) {
					if (result.at(0) + result.at(1) == 0) graphs.at(current).at(horizon) += Mv::r(1);
					else {
						if (not color_gradient.empty()) graphs.at(current).at(horizon) += Theme::g(color_gradient).at(clamp(max(last, data_value), 0ll, 100ll));
						graphs.at(current).at(horizon) += graph_symbol.at((result.at(0) * 5 + result.at(1)));
					}
				}
				else graphs.at(current).at(horizon) += graph_symbol.at((result.at(0) * 5 + result.at(1)));
			}
			if (mult and i >= 0) last = data_value;
		}
		last = data_value;
		out.clear();
		if (height == 1) {
			//if (not color_gradient.empty())
			//	out += (last < 1 ? Theme::c("inactive_fg") : Theme::g(color_gradient).at(clamp(last, 0ll, 100ll)));
			out += graphs.at(current).at(0);
		}
		else {
			for (const int& i : iota(1, height + 1)) {
				if (i > 1) out += Mv::d(1) + Mv::l(width);
				if (not color_gradient.empty())
					out += (invert) ? Theme::g(color_gradient).at(i * 100 / height) : Theme::g(color_gradient).at(100 - ((i - 1) * 100 / height));
				out += (invert) ? graphs.at(current).at(height - i) : graphs.at(current).at(i-1);
			}
		}
		if (not color_gradient.empty()) out += Fx::reset;
	}

	Graph::Graph() {}

	Graph::Graph(int width, int height, const string& color_gradient,
				 const deque<long long>& data, const string& symbol,
				 bool invert, bool no_zero, long long max_value, long long offset)
	: width(width), height(height), color_gradient(color_gradient),
	  invert(invert), no_zero(no_zero), offset(offset) {
		if (Config::getB("tty_mode") or symbol == "tty") this->symbol = "tty";
		else if (symbol != "default") this->symbol = symbol;
		else this->symbol = Config::getS("graph_symbol");
		if (this->symbol == "tty") tty_mode = true;

		if (max_value == 0 and offset > 0) max_value = 100;
		this->max_value = max_value;
		const int value_width = (tty_mode ? data.size() : ceil((double)data.size() / 2));
		int data_offset = (value_width > width) ? data.size() - width * (tty_mode ? 1 : 2) : 0;

		if (not tty_mode and (data.size() - data_offset) % 2 != 0) {
			data_offset--;
		}

		//? Populate the two switching graph vectors and fill empty space if data size < width
		for (const int& i : iota(0, height * 2)) {
			if (tty_mode and i % 2 != current) continue;
			graphs[(i % 2 != 0)].push_back((value_width < width) ? ((height == 1) ? Mv::r(1) : " "s) * (width - value_width) : "");
		}
		if (data.size() == 0) return;
		this->_create(data, data_offset);
	}

	string& Graph::operator()(const deque<long long>& data, bool data_same) {
		if (data_same) return out;

		//? Make room for new characters on graph
		if (not tty_mode) current = not current;
		for (const int& i : iota(0, height)) {
			if (height == 1 and graphs.at(current).at(i).at(1) == '[') {
				if (graphs.at(current).at(i).at(3) == 'C') graphs.at(current).at(i).erase(0, 4);
				else graphs.at(current).at(i).erase(0, graphs.at(current).at(i).find_first_of('m') + 4);
			}
			else if (graphs.at(current).at(i).at(0) == ' ') graphs.at(current).at(i).erase(0, 1);
			else graphs.at(current).at(i).erase(0, 3);
		}
		this->_create(data, (int)data.size() - 1);
		return out;
	}

	string& Graph::operator()() {
		return out;
	}
	//*------------------------------------------------------------------------------------------------------------------------->

}

namespace Cpu {
	// Cpu namespace variables kept for calc_sizes() function
	int width_p = 100, height_p = 32;
	int min_width = 60, min_height = 8;
	int x = 1, y = 1, width = 20, height;
	int b_columns, b_column_size;
	int b_x, b_y, b_width, b_height;
	float max_observed_pwr = 1.0f;

	int graph_up_height, graph_low_height;
	int graph_up_width, graph_low_width;
	int gpu_meter_width;
	bool shown = false, redraw = false, mid_line = false;  // shown = false (disabled)
	string box;
	vector<Draw::Graph> graphs_upper;
	vector<Draw::Graph> graphs_lower;
	Draw::Meter cpu_meter;
	vector<Draw::Meter> gpu_meters;
	vector<Draw::Graph> core_graphs;
	vector<Draw::Graph> temp_graphs;
	vector<Draw::Graph> gpu_temp_graphs;
	vector<Draw::Graph> gpu_mem_graphs;

	// Cpu draw function stubbed out - stock btop monitoring disabled
	string draw([[maybe_unused]] const cpu_info& cpu, [[maybe_unused]] const vector<Gpu::gpu_info>& gpus, 
	            [[maybe_unused]] bool force_redraw, [[maybe_unused]] bool data_same) {
		return "";
	}
}

#ifdef GPU_SUPPORT
namespace Gpu {
	int width_p = 100, height_p = 32;
	int min_width = 41, min_height = 8;
	int width = 41, total_height;
	vector<int> x_vec = {}, y_vec = {}, b_height_vec = {};
	int b_width;
	vector<int> b_x_vec = {}, b_y_vec = {};
	vector<bool> redraw = {};
	int shown = 0;
	int count = 0;
	vector<int> shown_panels = {};
	int graph_up_height;
	vector<Draw::Graph> graph_upper_vec = {}, graph_lower_vec = {};
	vector<Draw::Graph> temp_graph_vec = {};
	vector<Draw::Graph> mem_used_graph_vec = {}, mem_util_graph_vec = {};
	vector<Draw::Meter> gpu_meter_vec = {};
	vector<Draw::Meter> pwr_meter_vec = {};
	vector<Draw::Meter> enc_meter_vec = {};
	vector<string> box = {};

	// Gpu draw function stubbed out - stock btop monitoring disabled
	string draw([[maybe_unused]] const gpu_info& gpu, [[maybe_unused]] unsigned long index, 
	            [[maybe_unused]] bool force_redraw, [[maybe_unused]] bool data_same) {
		return "";
	}

}
#endif

namespace Mem {
	int width_p = 45, height_p = 36;
	int min_width = 36, min_height = 10;
	int x = 1, y, width = 20, height;
	int mem_width, disks_width, divider, item_height, mem_size, mem_meter, graph_height, disk_meter;
	int disks_io_h = 0;
	int disks_io_half = 0;
	bool shown = true, redraw = true;
	string box;
	std::unordered_map<string, Draw::Meter> mem_meters;
	std::unordered_map<string, Draw::Graph> mem_graphs;
	std::unordered_map<string, Draw::Meter> disk_meters_used;
	std::unordered_map<string, Draw::Meter> disk_meters_free;
	std::unordered_map<string, Draw::Graph> io_graphs;

	// Mem draw function stubbed out - stock btop monitoring disabled
	string draw([[maybe_unused]] const mem_info& mem, [[maybe_unused]] bool force_redraw, [[maybe_unused]] bool data_same) {
		return "";
	}

}

namespace Net {
	int width_p = 45, height_p = 32;
	int min_width = 36, min_height = 6;
	int x = 1, y, width = 20, height;
	int b_x, b_y, b_width, b_height, d_graph_height, u_graph_height;
	bool shown = false, redraw = false;  // shown = false (disabled)
	[[maybe_unused]] const int MAX_IFNAMSIZ = 15;
	string old_ip;
	std::unordered_map<string, Draw::Graph> graphs;
	string box;

	// Net draw function stubbed out - stock btop monitoring disabled
	string draw([[maybe_unused]] const net_info& net, [[maybe_unused]] bool force_redraw, [[maybe_unused]] bool data_same) {
		return "";
	}

}

namespace Proc {
	int width_p = 55, height_p = 68;
	int min_width = 44, min_height = 16;
	int x, y, width = 20, height;
	int start, selected, select_max;
	bool shown = false, redraw = false;  // shown = false (disabled)
	int selected_pid = 0, selected_depth = 0;
	string selected_name;
	std::unordered_map<size_t, Draw::Graph> p_graphs;
	std::unordered_map<size_t, bool> p_wide_cmd;
	std::unordered_map<size_t, int> p_counters;
	int counter = 0;
	Draw::TextEdit filter;
	Draw::Graph detailed_cpu_graph;
	Draw::Graph detailed_mem_graph;
	int user_size, thread_size, prog_size, cmd_size, tree_size;
	int dgraph_x, dgraph_width, d_width, d_x, d_y;

	string box;

	int selection(const std::string_view cmd_key) {
		auto start = Config::getI("proc_start");
		auto selected = Config::getI("proc_selected");
		auto last_selected = Config::getI("proc_last_selected");
		const int select_max = (Config::getB("show_detailed") ? (Config::getB("pause_proc_list") ? Proc::select_max - 9 : Proc::select_max - 8) :
																(Config::getB("pause_proc_list") ? Proc::select_max - 1 : Proc::select_max));
		auto vim_keys = Config::getB("vim_keys");

		int numpids = Proc::numpids;
		if ((cmd_key == "up" or (vim_keys and cmd_key == "k")) and selected > 0) {
			if (start > 0 and selected == 1) start--;
			else selected--;
			if (Config::getI("proc_last_selected") > 0) Config::set("proc_last_selected", 0);
		}
		else if (cmd_key == "mouse_scroll_up" and start > 0) {
			start = max(0, start - 3);
		}
		else if (cmd_key == "mouse_scroll_down" and start < numpids - select_max) {
			start = min(numpids - select_max, start + 3);
		}
		else if (cmd_key == "down" or (vim_keys and cmd_key == "j")) {
			if (start < numpids - select_max and selected == select_max) start++;
			else if (selected == 0 and last_selected > 0) {
				selected = last_selected;
				Config::set("proc_last_selected", 0);
			}
			else selected++;
		}
		else if (cmd_key == "page_up") {
			if (selected > 0 and start == 0) selected = 0;
			else start = max(0, start - select_max);
		}
		else if (cmd_key == "page_down") {
			if (selected > 0 and start >= numpids - select_max) selected = select_max;
			else start = clamp(start + select_max, 0, max(0, numpids - select_max));
		}
		else if (cmd_key == "home" or (vim_keys and cmd_key == "g")) {
			start = 0;
			if (selected > 0) selected = 1;
		}
		else if (cmd_key == "end" or (vim_keys and cmd_key == "G")) {
			start = max(0, numpids - select_max);
			if (selected > 0) selected = select_max;
		}
		else if (cmd_key.starts_with("mousey")) {
			int mouse_y = std::atoi(cmd_key.substr(6).data());
			start = clamp((int)round((double)mouse_y * (numpids - select_max - 2) / (select_max - 2)), 0, max(0, numpids - select_max));
		}

		bool changed = false;
		if (start != Config::getI("proc_start")) {
			Config::set("proc_start", start);
			changed = true;
		}
		if (selected != Config::getI("proc_selected")) {
			Config::set("proc_selected", selected);
			changed = true;
		}
		return (not changed ? -1 : selected);
	}

	string draw(const vector<proc_info>& plist, bool force_redraw, bool data_same) {
		if (Runner::stopping) return "";
		auto proc_tree = Config::getB("proc_tree");
		bool show_detailed = (Config::getB("show_detailed") and cmp_equal(Proc::detailed.last_pid, Config::getI("detailed_pid")));
		bool proc_gradient = (Config::getB("proc_gradient") and not Config::getB("lowcolor") and Theme::gradients.contains("proc"));
		auto proc_colors = Config::getB("proc_colors");
		auto tty_mode = Config::getB("tty_mode");
		auto& graph_symbol = (tty_mode ? "tty" : Config::getS("graph_symbol_proc"));
		auto& graph_bg = Symbols::graph_symbols.at((graph_symbol == "default" ? Config::getS("graph_symbol") + "_up" : graph_symbol + "_up")).at(6);
		auto mem_bytes = Config::getB("proc_mem_bytes");
		auto vim_keys = Config::getB("vim_keys");
		auto show_graphs = Config::getB("proc_cpu_graphs");
		const auto pause_proc_list = Config::getB("pause_proc_list");
		start = Config::getI("proc_start");
		selected = Config::getI("proc_selected");
		const int y = show_detailed ? Proc::y + 8 : Proc::y;
		const int height = show_detailed ? Proc::height - 8 : Proc::height;
		const int select_max = show_detailed ? (pause_proc_list ? Proc::select_max - 9 : Proc::select_max - 8) : 
												(pause_proc_list ? Proc::select_max - 1 : Proc::select_max);
		auto totalMem = Mem::get_totalMem();
		int numpids = Proc::numpids;
		if (force_redraw) redraw = true;
		string out;
		out.reserve(width * height);

		//* Redraw elements not needed to be updated every cycle
		if (redraw) {
			out = box;
			const string title_left = Theme::c("proc_box") + Symbols::title_left;
			const string title_right = Theme::c("proc_box") + Symbols::title_right;
			const string title_left_down = Theme::c("proc_box") + Symbols::title_left_down;
			const string title_right_down = Theme::c("proc_box") + Symbols::title_right_down;
			for (const auto& key : {"T", "K", "S", "enter"})
				if (Input::mouse_mappings.contains(key)) Input::mouse_mappings.erase(key);

			//? Adapt sizes of text fields
			user_size = (width < 75 ? 5 : 10);
			thread_size = (width < 75 ? - 1 : 4);
			prog_size = (width > 70 ? 16 : ( width > 55 ? 8 : width - user_size - thread_size - 33));
			cmd_size = (width > 55 ? width - prog_size - user_size - thread_size - 33 : -1);
			tree_size = width - user_size - thread_size - 23;
			if (not show_graphs) {
				cmd_size += 5;
				tree_size += 5;
			}

			//? Detailed box
			if (show_detailed) {
				bool alive = detailed.status != "Dead";
				dgraph_x = x;
				dgraph_width = max(width / 3, width - 121);
				d_width = width - dgraph_width - 1;
				d_x = x + dgraph_width + 1;
				d_y = Proc::y;

				//? Create cpu and mem graphs if process is alive
				if (alive or pause_proc_list) {
					detailed_cpu_graph = Draw::Graph{dgraph_width - 1, 7, "cpu", detailed.cpu_percent, graph_symbol, false, true};
					detailed_mem_graph = Draw::Graph{d_width / 3, 1, "", detailed.mem_bytes, graph_symbol, false, false, detailed.first_mem};
				}

				//? Draw structure of details box
				const string pid_str = to_string(detailed.entry.pid);
				out += Mv::to(y, x) + Theme::c("proc_box") + Symbols::div_left + Symbols::h_line + title_left + Theme::c("hi_fg") + Fx::b
				+ (tty_mode ? "4" : Symbols::superscript.at(4)) + Theme::c("title") + "proc"
					+ Fx::ub + title_right + Symbols::h_line * (width - 10) + Symbols::div_right
					+ Mv::to(d_y, dgraph_x + 2) + title_left + Fx::b + Theme::c("title") + pid_str + Fx::ub + title_right
					+ title_left + Fx::b + Theme::c("title") + uresize(detailed.entry.name, dgraph_width - pid_str.size() - 7, true) + Fx::ub + title_right;

				out += Mv::to(d_y, d_x - 1) + Theme::c("proc_box") + Symbols::div_up + Mv::to(y, d_x - 1) + Symbols::div_down + Theme::c("div_line");
				for (const int& i : iota(1, 8)) out += Mv::to(d_y + i, d_x - 1) + Symbols::v_line;

				const string t_color = (not alive or selected > 0 ? Theme::c("inactive_fg") : Theme::c("title"));
				const string hi_color = (not alive or selected > 0 ? t_color : Theme::c("hi_fg"));
				int mouse_x = d_x + 2;
				out += Mv::to(d_y, d_x + 1);
				if (width > 55) {
					out += Fx::ub + title_left + hi_color + Fx::b + 't' + t_color + "erminate" + Fx::ub + title_right;
					if (alive and selected == 0) Input::mouse_mappings["t"] = {d_y, mouse_x, 1, 9};
					mouse_x += 11;
				}
				out += title_left + hi_color + Fx::b + (vim_keys ? 'K' : 'k') + t_color + "ill" + Fx::ub + title_right
					+ title_left + hi_color + Fx::b + 's' + t_color + "ignals" + Fx::ub + title_right
					+ title_left + hi_color + Fx::b + 'N' + t_color + "ice" + Fx::ub + title_right;
				if (alive and selected == 0) {
					Input::mouse_mappings["k"] = {d_y, mouse_x, 1, 4};
					mouse_x += 6;
					Input::mouse_mappings["s"] = {d_y, mouse_x, 1, 7};
				    mouse_x += 9;
					Input::mouse_mappings["N"] = {d_y, mouse_x, 1, 5};
				}

				//? Labels
				const int item_fit = floor((double)(d_width - 2) / 10);
				const int item_width = floor((double)(d_width - 2) / min(item_fit, 8));
				out += Mv::to(d_y + 1, d_x + 1) + Fx::b + Theme::c("title")
										+ cjust("Status:", item_width)
										+ cjust("Elapsed:", item_width);
				if (item_fit >= 3) out += cjust("IO/R:", item_width);
				if (item_fit >= 4) out += cjust("IO/W:", item_width);
				if (item_fit >= 5) out += cjust("Parent:", item_width);
				if (item_fit >= 6) out += cjust("User:", item_width);
				if (item_fit >= 7) out += cjust("Threads:", item_width);
				if (item_fit >= 8) out += cjust("Nice:", item_width);


				//? Command line
				for (int i = 0; const auto& l : {'C', 'M', 'D'})
				out += Mv::to(d_y + 5 + i++, d_x + 1) + l;

				out += Theme::c("main_fg") + Fx::ub;
				const auto san_cmd = replace_ascii_control(detailed.entry.cmd);
				const int cmd_size = ulen(san_cmd, true);
				for (int num_lines = min(3, (int)ceil((double)cmd_size / (d_width - 5))), i = 0; i < num_lines; i++) {
					out += Mv::to(d_y + 5 + (num_lines == 1 ? 1 : i), d_x + 3)
						+ cjust(luresize(san_cmd, cmd_size - (d_width - 5) * i, true), d_width - 5, true, true);
				}

			}

			//? Filter
			auto filtering = Config::getB("proc_filtering"); // ? filter(20) : Config::getS("proc_filter"))
			const auto filter_text = (filtering) ? filter(max(6, width - 58)) : uresize(Config::getS("proc_filter"), max(6, width - 58));
			out += Mv::to(y, x+9) + title_left + (not filter_text.empty() ? Fx::b : "") + Theme::c("hi_fg") + 'f'
				+ Theme::c("title") + (not filter_text.empty() ? ' ' + filter_text : "ilter")
				+ (not filtering and not filter_text.empty() ? Theme::c("hi_fg") + " del" : "")
				+ (filtering ? Theme::c("hi_fg") + ' ' + Symbols::enter : "") + Fx::ub + title_right;
			if (not filtering) {
				int f_len = (filter_text.empty() ? 6 : ulen(filter_text) + 2);
				Input::mouse_mappings["f"] = {y, x + 10, 1, f_len};
				if (filter_text.empty() and Input::mouse_mappings.contains("delete"))
					Input::mouse_mappings.erase("delete");
				else if (not filter_text.empty())
					Input::mouse_mappings["delete"] = {y, x + 11 + f_len, 1, 3};
			}

			//? per-core, reverse, tree and sorting
			const auto& sorting = Config::getS("proc_sorting");
			const int sort_len = sorting.size();
			const int sort_pos = x + width - sort_len - 8;

			if (width > 55 + sort_len) {
				out += Mv::to(y, sort_pos - 25) + title_left + (Config::getB("proc_per_core") ? Fx::b : "") + Theme::c("title")
					+ "per-" + Theme::c("hi_fg") + 'c' + Theme::c("title") + "ore" + Fx::ub + title_right;
				Input::mouse_mappings["c"] = {y, sort_pos - 24, 1, 8};
			}
			if (width > 45 + sort_len) {
				out += Mv::to(y, sort_pos - 15) + title_left + (Config::getB("proc_reversed") ? Fx::b : "") + Theme::c("hi_fg")
					+ 'r' + Theme::c("title") + "everse" + Fx::ub + title_right;
				Input::mouse_mappings["r"] = {y, sort_pos - 14, 1, 7};
			}
			if (width > 35 + sort_len) {
				out += Mv::to(y, sort_pos - 6) + title_left + (Config::getB("proc_tree") ? Fx::b : "") + Theme::c("title") + "tre"
					+ Theme::c("hi_fg") + 'e' + Fx::ub + title_right;
				Input::mouse_mappings["e"] = {y, sort_pos - 5, 1, 4};
			}
			out += Mv::to(y, sort_pos) + title_left + Fx::b + Theme::c("hi_fg") + Symbols::left + " " + Theme::c("title") + sorting + " " + Theme::c("hi_fg")
				+ Symbols::right + Fx::ub + title_right;
				Input::mouse_mappings["left"] = {y, sort_pos + 1, 1, 2};
				Input::mouse_mappings["right"] = {y, sort_pos + sort_len + 3, 1, 2};

			//? select, info and signal buttons
			const string down_button = (selected == select_max and start == numpids - select_max ? Theme::c("inactive_fg") : Theme::c("hi_fg")) + Symbols::down;
			const string t_color = (selected == 0 ? Theme::c("inactive_fg") : Theme::c("title"));
			const string hi_color = (selected == 0 ? Theme::c("inactive_fg") : Theme::c("hi_fg"));
			int mouse_x = x + 14;
			out += Mv::to(y + height - 1, x + 1) + title_left_down + Fx::b + hi_color + Symbols::up + Theme::c("title") + " select " + down_button + Fx::ub + title_right_down
				+ title_left_down + Fx::b + t_color + "info " + hi_color + Symbols::enter + Fx::ub + title_right_down;
				if (selected > 0) Input::mouse_mappings["enter"] = {y + height - 1, mouse_x, 1, 6};
				mouse_x += 8;
			if (width > 60) {
				out += title_left_down + Fx::b + hi_color + 't' + t_color + "erminate" + Fx::ub + title_right_down;
				if (selected > 0) Input::mouse_mappings["t"] = {y + height - 1, mouse_x, 1, 9};
				mouse_x += 11;
			}
			if (width > 55) {
				out += title_left_down + Fx::b + hi_color + (vim_keys ? 'K' : 'k') + t_color + "ill" + Fx::ub + title_right_down;
				if (selected > 0) Input::mouse_mappings["k"] = {y + height - 1, mouse_x, 1, 4};
				mouse_x += 6;
			}
			out += title_left_down + Fx::b + hi_color + 's' + t_color + "ignals" + Fx::ub + title_right_down;
			if (selected > 0) Input::mouse_mappings["s"] = {y + height - 1, mouse_x, 1, 7};
		    mouse_x += 9;
		    out += title_left_down + Fx::b + hi_color + 'N' + t_color + "ice" + Fx::ub + title_right_down;
		    if (selected > 0) Input::mouse_mappings["N"] = {y + height -1, mouse_x, 1, 5};

			//? Labels for fields in list
			if (not proc_tree)
				out += Mv::to(y+1, x+1) + Theme::c("title") + Fx::b
					+ rjust("Pid:", 8) + ' '
					+ ljust("Program:", prog_size) + ' '
					+ (cmd_size > 0 ? ljust("Command:", cmd_size) : "") + ' ';
			else
				out += Mv::to(y+1, x+1) + Theme::c("title") + Fx::b
					+ ljust("Tree:", tree_size) + ' ';

			out += (thread_size > 0 ? Mv::l(4) + "Threads: " : "")
					+ ljust("User:", user_size) + ' '
					+ rjust((mem_bytes ? "MemB" : "Mem%"), 5) + ' '
					+ rjust("Cpu%", (show_graphs ? 10 : 5)) + Fx::ub;
		}
		//* End of redraw block

		//? Draw details box if shown
		if (show_detailed) {
			bool alive = detailed.status != "Dead";
			const int item_fit = floor((double)(d_width - 2) / 10);
			const int item_width = floor((double)(d_width - 2) / min(item_fit, 8));

			//? Graph part of box
			string cpu_str = (alive or pause_proc_list ? fmt::format("{:.2f}", detailed.entry.cpu_p) : "");
			if (alive or pause_proc_list) {
				cpu_str.resize(4);
				if (cpu_str.ends_with('.')) { cpu_str.pop_back(); cpu_str.pop_back(); }
			}
			out += Mv::to(d_y + 1, dgraph_x + 1) + Fx::ub + detailed_cpu_graph(detailed.cpu_percent, (redraw or data_same or not alive))
				+ Mv::to(d_y + 1, dgraph_x + 1) + Theme::c("title") + Fx::b + rjust(cpu_str, 4) + "%";
			for (int i = 0; const auto& l : {'C', 'P', 'U'})
					out += Mv::to(d_y + 3 + i++, dgraph_x + 1) + l;

			//? Info part of box
			const string stat_color = (not alive ? Theme::c("inactive_fg") : (detailed.status == "Running" ? Theme::c("proc_misc") : Theme::c("main_fg")));
			out += Mv::to(d_y + 2, d_x + 1) + stat_color + Fx::ub
									+ cjust(detailed.status, item_width) + Theme::c("main_fg")
									+ cjust(detailed.elapsed, item_width);
			if (item_fit >= 3) out += cjust(detailed.io_read, item_width);
			if (item_fit >= 4) out += cjust(detailed.io_write, item_width);
			if (item_fit >= 5) out += cjust(detailed.parent, item_width, true);
			if (item_fit >= 6) out += cjust(detailed.entry.user, item_width, true);
			if (item_fit >= 7) out += cjust(to_string(detailed.entry.threads), item_width);
			if (item_fit >= 8) out += cjust(to_string(detailed.entry.p_nice), item_width);


			const double mem_p = detailed.mem_bytes.back() * 100.0 / totalMem;
			string mem_str = fmt::format("{:.2f}", mem_p);
			mem_str.resize(4);
			if (mem_str.ends_with('.')) mem_str.pop_back();
			out += Mv::to(d_y + 4, d_x + 1) + Theme::c("title") + Fx::b + rjust((item_fit > 4 ? "Memory: " : "M:") + rjust(mem_str, 4) + "% ", (d_width / 3) - 2)
				+ Theme::c("inactive_fg") + Fx::ub + graph_bg * (d_width / 3) + Mv::l(d_width / 3)
				+ Theme::c("proc_misc") + detailed_mem_graph(detailed.mem_bytes, (redraw or data_same or not alive)) + ' '
				+ Theme::c("title") + Fx::b + detailed.memory;
		}

		//? Check bounds of current selection and view
		if (start > 0 and numpids <= select_max)
			start = 0;
		if (start > numpids - select_max)
			start = max(0, numpids - select_max);
		if (selected > select_max)
			selected = select_max;
		if (selected > numpids)
			selected = numpids;

		//* Iteration over processes
		int lc = 0;
		for (int n=0; auto& p : plist) {
			if (p.filtered or (proc_tree and p.tree_index == plist.size()) or n++ < start) continue;
			bool is_selected = (lc + 1 == selected);
			if (is_selected) {
				selected_pid = (int)p.pid;
				selected_name = p.name;
				selected_depth = p.depth;
			}

			//? Update graphs for processes with above 0.0% cpu usage, delete if below 0.1% 10x times
			bool has_graph = show_graphs ? p_counters.contains(p.pid) : false;
			if (show_graphs and ((p.cpu_p > 0 and not has_graph) or (not data_same and has_graph))) {
				if (not has_graph) {
					p_graphs[p.pid] = Draw::Graph{5, 1, "", {}, graph_symbol};
					p_counters[p.pid] = 0;
				}
				else if (p.cpu_p < 0.1 and ++p_counters[p.pid] >= 10) {
					if (p_graphs.contains(p.pid)) p_graphs.erase(p.pid);
					p_counters.erase(p.pid);
				}
				else
					p_counters[p.pid] = 0;
			}

			out += Fx::reset;

			//? Set correct gradient colors if enabled
			string c_color, m_color, t_color, g_color, end;
			if (is_selected) {
				c_color = m_color = t_color = g_color = Fx::b;
				end = Fx::ub;
				out += Theme::c("selected_bg") + Theme::c("selected_fg") + Fx::b;
			}
			else {
				int calc = (selected > lc) ? selected - lc : lc - selected;
				if (proc_colors) {
					end = Theme::c("main_fg") + Fx::ub;
					array<string, 3> colors;
					for (int i = 0; int v : {(int)round(p.cpu_p), (int)round(p.mem * 100 / totalMem), (int)p.threads / 3}) {
						if (proc_gradient) {
							int val = (min(v, 100) + 100) - calc * 100 / select_max;
							if (val < 100) colors[i++] = Theme::g("proc_color").at(max(0, val));
							else colors[i++] = Theme::g("process").at(clamp(val - 100, 0, 100));
						}
						else
							colors[i++] = Theme::g("process").at(clamp(v, 0, 100));
					}
					c_color = colors.at(0); m_color = colors.at(1); t_color = colors.at(2);
				}
				else {
					c_color = m_color = t_color = Fx::b;
					end = Fx::ub;
				}
				if (proc_gradient) {
					g_color = Theme::g("proc").at(clamp(calc * 100 / select_max, 0, 100));
				}
			}

			const auto san_cmd = replace_ascii_control(p.cmd);

			if (not p_wide_cmd.contains(p.pid)) p_wide_cmd[p.pid] = ulen(san_cmd) != ulen(san_cmd, true);

			//? Normal view line
			if (not proc_tree) {
				out += Mv::to(y+2+lc, x+1)
					+ g_color + rjust(to_string(p.pid), 8) + ' '
					+ c_color + ljust(p.name, prog_size, true) + ' ' + end
					+ (cmd_size > 0 ? g_color + ljust(san_cmd, cmd_size, true, p_wide_cmd[p.pid]) + Mv::to(y+2+lc, x+11+prog_size+cmd_size) + ' ' : "");
			}
			//? Tree view line
			else {
				const string prefix_pid = p.prefix + to_string(p.pid);
				int width_left = tree_size;
				out += Mv::to(y+2+lc, x+1) + g_color + uresize(prefix_pid, width_left) + ' ';
				width_left -= ulen(prefix_pid);
				if (width_left > 0) {
					out += c_color + uresize(p.name, width_left - 1) + end + ' ';
					width_left -= (ulen(p.name) + 1);
				}
				if (width_left > 7) {
					const string_view cmd = width_left > 40 ? rtrim(san_cmd) : p.short_cmd;
					if (not cmd.empty() and cmd != p.name) {
						out += g_color + '(' + uresize(string{cmd}, width_left - 3, p_wide_cmd[p.pid]) + ") ";
						width_left -= (ulen(string{cmd}, true) + 3);
					}
				}
				out += string(max(0, width_left), ' ') + Mv::to(y+2+lc, x+2+tree_size);
			}
			//? Common end of line
			string cpu_str = fmt::format("{:.2f}", p.cpu_p);
			if (p.cpu_p < 10 or (p.cpu_p >= 100 and p.cpu_p < 1000)) cpu_str.resize(3);
			else if (p.cpu_p >= 10'000) {
				cpu_str = fmt::format("{:.2f}", p.cpu_p / 1000);
				cpu_str.resize(3);
				if (cpu_str.ends_with('.')) cpu_str.pop_back();
				cpu_str += "k";
			}
			string mem_str = (mem_bytes ? floating_humanizer(p.mem, true) : "");
			if (not mem_bytes) {
				double mem_p = clamp((double)p.mem * 100 / totalMem, 0.0, 100.0);
				mem_str = mem_p < 0.01 ? "0" : fmt::format("{:.1f}", mem_p);
				if (mem_str.size() > 3) mem_str.resize(3);
				if (mem_str.ends_with('.')) mem_str.pop_back();
				mem_str += '%';
			}

			// Shorten process thread representation when larger than 5 digits: 10000 -> 10K ...
			const std::string proc_threads_string = [&] {
				if (p.threads > 9999) {
					return std::to_string(p.threads / 1000) + 'K';
				} else {
					return std::to_string(p.threads);
				}
			}();

			out += (thread_size > 0 ? t_color + rjust(proc_threads_string, thread_size) + ' ' + end : "" )
				+ g_color + ljust((cmp_greater(p.user.size(), user_size) ? p.user.substr(0, user_size - 1) + '+' : p.user), user_size) + ' '
				+ m_color + rjust(mem_str, 5) + end + ' '
				+ (is_selected ? "" : Theme::c("inactive_fg")) + (show_graphs ? graph_bg * 5: "")
				+ (p_graphs.contains(p.pid) ? Mv::l(5) + c_color + p_graphs.at(p.pid)({(p.cpu_p >= 0.1 and p.cpu_p < 5 ? 5ll : (long long)round(p.cpu_p))}, data_same) : "") + end + ' '
				+ c_color + rjust(cpu_str, 4) + "  " + end;
			if (lc++ > height - 5) break;
			else if (lc > height - 5 and pause_proc_list) break;
		}

		out += Fx::reset;
		while (lc++ < height - 3) out += Mv::to(y+lc+1, x+1) + string(width - 2, ' ');
		if (pause_proc_list) {
			fmt::format_to(std::back_inserter(out), "{}{}{}{}{:^{}}{}",
				Mv::to(y + height - 2, x + 1),
				Theme::c("proc_pause_bg"), Theme::c("title"), 
				Fx::b, "Process list paused", width - 2,
				Fx::reset);
		}

		//? Draw scrollbar if needed
		if (numpids > select_max) {
			const int scroll_pos = clamp((int)round((double)start * select_max / (numpids - select_max)), 0, height - 5);
			out += Mv::to(y + 1, x + width - 2) + Fx::b + Theme::c("main_fg") + Symbols::up
				+ Mv::to(y + height - 2, x + width - 2) + Symbols::down;

			for (int i = y + 2; i < y + height - 2; i++) {
				out += Mv::to(i, x + width - 2) + ((i == y + 2 + scroll_pos) ? "█" : " ");
			}
		}

		//? Current selection and number of processes
		string location = to_string(start + selected) + '/' + to_string(numpids);
		string loc_clear = Symbols::h_line * max((size_t)0, 9 - location.size());
		out += Mv::to(y + height - 1, x+width - 3 - max(9, (int)location.size())) + Fx::ub + Theme::c("proc_box") + loc_clear
			+ Symbols::title_left_down + Theme::c("title") + Fx::b + location + Fx::ub + Theme::c("proc_box") + Symbols::title_right_down;

		//? Clear out left over graphs from dead processes at a regular interval
		if (not data_same and ++counter >= 100) {
			counter = 0;

			std::erase_if(p_graphs, [&](const auto& pair) {
				return rng::find(plist, pair.first, &proc_info::pid) == plist.end();
			});

			std::erase_if(p_counters, [&](const auto& pair) {
				return rng::find(plist, pair.first, &proc_info::pid) == plist.end();
			});

			std::erase_if(p_wide_cmd, [&](const auto& pair) {
				return rng::find(plist, pair.first, &proc_info::pid) == plist.end();
			});
		}

		//? Draw hide button if detailed view is shown
		if (show_detailed) {
			const bool greyed_out = selected_pid != Config::getI("detailed_pid") && selected > 0; 
			fmt::format_to(std::back_inserter(out), "{}{}{}{}{}{}{}{}{}{}{}",
				Mv::to(d_y, d_x + d_width - 10), 
				Theme::c("proc_box"), Symbols::title_left, Fx::b,
				greyed_out ? Theme::c("inactive_fg") : Theme::c("title"), "hide ",
				greyed_out ? "" : Theme::c("hi_fg"), Symbols::enter,
				Fx::ub, Theme::c("proc_box"), Symbols::title_right);
			if (not greyed_out) Input::mouse_mappings["enter"] = {d_y, d_x + d_width - 9, 1, 6};
		}

		if (selected == 0 and selected_pid != 0) {
			selected_pid = 0;
			selected_name.clear();
		}
		redraw = false;
		return out + Fx::reset;
	}

}

namespace Draw {
	void calcSizes() {
		atomic_wait(Runner::active);
		Config::unlock();
		auto boxes = Config::getS("shown_boxes");
		auto cpu_bottom = Config::getB("cpu_bottom");
		auto mem_below_net = Config::getB("mem_below_net");
		auto proc_left = Config::getB("proc_left");

		Cpu::box.clear();

		Mem::box.clear();
		Net::box.clear();
		Proc::box.clear();
		Global::clock.clear();
		Global::overlay.clear();
		Runner::pause_output = false;
		Runner::redraw = true;
		Proc::p_counters.clear();
		Proc::p_graphs.clear();
		if (Menu::active) Menu::redraw = true;

		Input::mouse_mappings.clear();

		Cpu::x = Mem::x = Net::x = Proc::x = 1;
		Cpu::y = Mem::y = Net::y = Proc::y = 1;
		Cpu::width = Mem::width = Net::width = Proc::width = 0;
		Cpu::height = Mem::height = Net::height = Proc::height = 0;
		Cpu::redraw = Mem::redraw = Net::redraw = Proc::redraw = true;

	#ifdef AZEROTHCORE_SUPPORT
		Draw::AzerothCore::shown = boxes.contains("azerothcore");
		if (Draw::AzerothCore::shown) {
			Draw::AzerothCore::redraw = true;
		}
	#endif
		// Disable all stock btop boxes - bottop only uses AzerothCore box
		Cpu::shown = false;  // Disabled: boxes.contains("cpu");
	#ifdef GPU_SUPPORT
		Gpu::box.clear();
		Gpu::width = 0;
		Gpu::shown_panels.clear();
		// Disabled GPU box
		// if (Gpu::count > 0) {
		// 	std::istringstream iss(boxes, std::istringstream::in);
		// 	string current;
		// 	while (iss >> current) {
		// 		if (current.starts_with("gpu"))
		// 			Gpu::shown_panels.push_back(current.back()-'0');
		// 	}
		// }
		Gpu::shown = 0;  // Disabled: Gpu::shown_panels.size();

		// Calculate the minimum possible GPU height, store in total_height
		// The actual total_height value will of course be overwritten later
		Gpu::total_height = 0;
		// Disabled GPU calculations
		// for (int i = 0; i < Gpu::shown; i++) {
		// 	using namespace Gpu;
		// 	total_height += 4 + gpu_b_height_offsets[shown_panels[i]];
		// }
	#endif
		Mem::shown = false;  // Disabled: boxes.contains("mem");
		Net::shown = false;  // Disabled: boxes.contains("net");
		Proc::shown = false; // Disabled: boxes.contains("proc");

		//* Calculate and draw cpu box outlines
		if (Cpu::shown) {
			using namespace Cpu;
		#ifdef GPU_SUPPORT
			// inline GPU information
			int gpus_extra_height =
				Config::getS("show_gpu_info") == "On" ? Gpu::count
				: Config::getS("show_gpu_info") == "Auto" ? Gpu::count - Gpu::shown
				: 0;
		#endif
            const bool show_temp = (Config::getB("check_temp") and got_sensors);
			width = round((double)Term::width * width_p / 100);
		#ifdef GPU_SUPPORT
			if (Gpu::shown != 0 and not (Mem::shown or Net::shown or Proc::shown)) {
				height = Term::height - Gpu::total_height - gpus_extra_height;
			} else {
				height = max(8, (int)ceil((double)Term::height * (trim(boxes) == "cpu" ? 100 : height_p/(Gpu::shown+1) + (Gpu::shown != 0)*5) / 100));
			}
			if (height <= Term::height-gpus_extra_height) height += gpus_extra_height;
		#else
			height = max(8, (int)ceil((double)Term::height * (trim(boxes) == "cpu" ? 100 : height_p) / 100));
		#endif
			x = 1;
			y = cpu_bottom ? Term::height - height + 1 : 1;

		#ifdef GPU_SUPPORT
			b_columns = max(2, (int)ceil((double)(Shared::coreCount + 1) / (height - gpus_extra_height - 5)));
		#else
			b_columns = max(1, (int)ceil((double)(Shared::coreCount + 1) / (height - 5)));
		#endif
			if (b_columns * (21 + 12 * show_temp) < width - (width / 3)) {
				b_column_size = 2;
				b_width =  max(29, (21 + 12 * show_temp) * b_columns - (b_columns - 1));
			}
			else if (b_columns * (15 + 6 * show_temp) < width - (width / 3)) {
				b_column_size = 1;
				b_width = (15 + 6 * show_temp) * b_columns - (b_columns - 1);
			}
			else if (b_columns * (8 + 6 * show_temp) < width - (width / 3)) {
				b_column_size = 0;
			}
			else {
				b_columns = (width - width / 3) / (8 + 6 * show_temp);
				b_column_size = 0;
			}

			if (b_column_size == 0) b_width = (8 + 6 * show_temp) * b_columns + 1;
		#ifdef GPU_SUPPORT
			//gpus_extra_height = max(0, gpus_extra_height - 1);
			b_height = min(height - 2, (int)ceil((double)Shared::coreCount / b_columns) + 4 + gpus_extra_height);
		#else
			b_height = min(height - 2, (int)ceil((double)Shared::coreCount / b_columns) + 4);
		#endif

			b_x = x + width - b_width - 1;
			b_y = y + ceil((double)(height - 2) / 2) - ceil((double)b_height / 2) + 1;

			box = createBox(x, y, width, height, Theme::c("cpu_box"), true, (cpu_bottom ? "" : "cpu"), (cpu_bottom ? "cpu" : ""), 1);

			auto& custom = Config::getS("custom_cpu_name");
			static const bool hasCpuHz = not Cpu::get_cpuHz().empty();
		#ifdef __linux__
			static const bool freq_range = Config::getS("freq_mode") == "range";
		#else
			static const bool freq_range = false;
		#endif
			const string cpu_title = uresize(
					(custom.empty() ? Cpu::cpuName : custom),
					b_width - (Config::getB("show_cpu_freq") and hasCpuHz ? (freq_range ? 24 : 14) : 5)
			);
			box += createBox(b_x, b_y, b_width, b_height, "", false, cpu_title);
		}

	#ifdef GPU_SUPPORT
		//* Calculate and draw gpu box outlines
		if (Gpu::shown != 0) {
			using namespace Gpu;
			x_vec.resize(shown); y_vec.resize(shown);
			b_x_vec.resize(shown); b_y_vec.resize(shown);
			b_height_vec.resize(shown);
			box.resize(shown);
			graph_upper_vec.resize(shown); graph_lower_vec.resize(shown);
			temp_graph_vec.resize(shown);
			mem_used_graph_vec.resize(shown); mem_util_graph_vec.resize(shown);
			gpu_meter_vec.resize(shown);
			pwr_meter_vec.resize(shown);
			enc_meter_vec.resize(shown);
			redraw.resize(shown);
			total_height = 0;
			for (auto i = 0; i < shown; ++i) {
				redraw[i] = true;
				int height = 0;
				width = Term::width;
				if (Cpu::shown)
					if (not (Mem::shown or Net::shown or Proc::shown))
						height = min_height;
					else height = Cpu::height;
				else
					if (not (Mem::shown or Net::shown or Proc::shown))
						height = (Term::height - total_height) / (Gpu::shown - i) + (i == 0) * ((Term::height - total_height) % (Gpu::shown - i));
					else
						height = max(min_height, (int)ceil((double)Term::height * height_p/Gpu::shown / 100));

				b_height_vec[i] = gpu_b_height_offsets[shown_panels[i]] + 2;
				height += (height+Cpu::height == Term::height-1);
				height = max(height, b_height_vec[i] + 2);
				x_vec[i] = 1; y_vec[i] = 1 + total_height + (not Config::getB("cpu_bottom"))*Cpu::shown*Cpu::height;
				box[i] = createBox(x_vec[i], y_vec[i], width, height, Theme::c("cpu_box"), true, std::string("gpu") + (char)(shown_panels[i]+'0'), "", (shown_panels[i]+5)%10); // TODO gpu_box
				b_width = clamp(width/2, min_width, 65);
				total_height += height;

				//? Main statistics box
				b_x_vec[i] = x_vec[i] + width - b_width - 1;
				b_y_vec[i] = y_vec[i] + ceil((double)(height - 2 - b_height_vec[i]) / 2) + 1;

				string name = Config::getS(std::string("custom_gpu_name") + (char)(shown_panels[i]+'0'));
				if (name.empty()) name = gpu_names[shown_panels[i]];

				box[i] += createBox(b_x_vec[i], b_y_vec[i], b_width, b_height_vec[i], "", false, name.substr(0, b_width-5));
				b_height_vec[i] = height - 2;
			}
		}
	#endif

		//* Calculate and draw mem box outlines
		if (Mem::shown) {
			using namespace Mem;
			auto show_disks = Config::getB("show_disks");
			auto swap_disk = Config::getB("swap_disk");
			auto mem_graphs = Config::getB("mem_graphs");

			width = round((double)Term::width * (Proc::shown ? width_p : 100) / 100);
		#ifdef GPU_SUPPORT
			height = ceil((double)Term::height * (100 - Net::height_p * Net::shown*4 / ((Gpu::shown != 0 and Cpu::shown) + 4)) / 100) - Cpu::height - Gpu::total_height;
		#else
			height = ceil((double)Term::height * (100 - Cpu::height_p * Cpu::shown - Net::height_p * Net::shown) / 100) + 1;
		#endif
			x = (proc_left and Proc::shown) ? Term::width - width + 1: 1;
			if (mem_below_net and Net::shown)
		#ifdef GPU_SUPPORT
				y = Term::height - height + 1 - (cpu_bottom ? Cpu::height : 0);
			else
				y = (cpu_bottom ? 1 : Cpu::height + 1) + Gpu::total_height;
		#else
				y = Term::height - height + 1 - (cpu_bottom ? Cpu::height : 0);
			else
				y = cpu_bottom ? 1 : Cpu::height + 1;
		#endif

			if (show_disks) {
				mem_width = ceil((double)(width - 3) / 2);
				mem_width += mem_width % 2;
				disks_width = width - mem_width - 2;
				divider = x + mem_width;
			}
			else
				mem_width = width - 1;

			item_height = has_swap and not swap_disk ? 6 : 4;
			if (height - (has_swap and not swap_disk ? 3 : 2) > 2 * item_height)
				mem_size = 3;
			else if (mem_width > 25)
				mem_size = 2;
			else
				mem_size = 1;

			mem_meter = max(0, mem_width - (mem_size > 2 ? 7 : 17));
			if (mem_size == 1) mem_meter += 6;

			if (mem_graphs) {
				graph_height = max(1, (int)round((double)((height - (has_swap and not swap_disk ? 2 : 1)) - (mem_size == 3 ? 2 : 1) * item_height) / item_height));
				if (graph_height > 1) mem_meter += 6;
			}
			else
				graph_height = 0;

			if (show_disks) {
				disk_meter = max(-14, width - mem_width - 23);
				if (disks_width < 25) disk_meter += 14;
			}

			box = createBox(x, y, width, height, Theme::c("mem_box"), true, "mem", "", 2);
			box += Mv::to(y, (show_disks ? divider + 2 : x + width - 9)) + Theme::c("mem_box") + Symbols::title_left + (show_disks ? Fx::b : "")
				+ Theme::c("hi_fg") + 'd' + Theme::c("title") + "isks" + Fx::ub + Theme::c("mem_box") + Symbols::title_right;
			Input::mouse_mappings["d"] = {y, (show_disks ? divider + 3 : x + width - 8), 1, 5};
			if (show_disks) {
				box += Mv::to(y, divider) + Symbols::div_up + Mv::to(y + height - 1, divider) + Symbols::div_down + Theme::c("div_line");
				for (auto i : iota(1, height - 1))
					box += Mv::to(y + i, divider) + Symbols::v_line;
			}
		}

		//* Calculate and draw net box outlines
		if (Net::shown) {
			using namespace Net;
			width = round((double)Term::width * (Proc::shown ? width_p : 100) / 100);
		#ifdef GPU_SUPPORT
			height = Term::height - Cpu::height - Gpu::total_height - Mem::height;
		#else
			height = Term::height - Cpu::height - Mem::height;
		#endif
			x = (proc_left and Proc::shown) ? Term::width - width + 1 : 1;
			if (mem_below_net and Mem::shown)
			#ifdef GPU_SUPPORT
				y = (cpu_bottom ? 1 : Cpu::height + 1) + Gpu::total_height;
			#else
				y = cpu_bottom ? 1 : Cpu::height + 1;
			#endif
			else
				y = Term::height - height + 1 - (cpu_bottom ? Cpu::height : 0);

			b_width = (width > 45) ? 27 : 19;
			b_height = (height > 10) ? 9 : height - 2;
			b_x = x + width - b_width - 1;
			b_y = y + ((height - 2) / 2) - b_height / 2 + 1;
			d_graph_height = round((double)(height - 2) / 2);
			u_graph_height = height - 2 - d_graph_height;

			box = createBox(x, y, width, height, Theme::c("net_box"), true, "net", "", 3);
			box += createBox(b_x, b_y, b_width, b_height, "", false, "download", "upload");
		}

		//* Calculate and draw proc box outlines
		if (Proc::shown) {
			using namespace Proc;
			width = Term::width - (Mem::shown ? Mem::width : (Net::shown ? Net::width : 0));
		#ifdef GPU_SUPPORT
			height = Term::height - Cpu::height - Gpu::total_height;
		#else
			height = Term::height - Cpu::height;
		#endif
			x = proc_left ? 1 : Term::width - width + 1;
		#ifdef GPU_SUPPORT
			y = ((cpu_bottom and Cpu::shown) ? 1 : Cpu::height + 1) + Gpu::total_height;
		#else
			y = (cpu_bottom and Cpu::shown) ? 1 : Cpu::height + 1;
		#endif
			select_max = height - 3;
			box = createBox(x, y, width, height, Theme::c("proc_box"), true, "proc", "", 4);
		}
		
	#ifdef AZEROTHCORE_SUPPORT
		//* Calculate and draw AzerothCore box outlines (3-pane layout)
		if (Draw::AzerothCore::shown) {
			using namespace Draw::AzerothCore;
			width = Term::width;
			height = Term::height;
			x = 1;
			y = 1;
			
			//* Calculate layout:
			//* Top section split: SERVER PERFORMANCE (left ~70%) | BOT DISTRIBUTION (right ~30%)
			//* Bottom section: ZONES (full width)
			
			// Distribution pane has fixed width based on content requirements
			// Content width calculation:
			// - 1 left padding + 15 name + 2 indicator + 3 percent = 21 chars
			// - Add 2 for borders = 23 total
			dist_width = 23;
			
			// Calculate distribution pane height dynamically based on detected brackets
			// Formula: 3 (factions) + 5 (continents) + (N+1) (levels) + 2 (borders) + 1 (spacing)
			int num_level_brackets = ::AzerothCore::expected_values.bracket_definitions.size();
			if (num_level_brackets == 0) num_level_brackets = 8;  // Default to 8 if not loaded yet
			
			// Height calculation (exact content fit):
			// - Factions: 1 title + 2 items = 3 lines
			// - Continents: 1 title + 4 items = 5 lines
			// - Levels: 1 title + N items = (N+1) lines
			// Total: 3 + 5 + (N+1) + 2 borders + 1 spacing = N + 12
			dist_height = num_level_brackets + 12;
			dist_x = width - dist_width + 1;
			dist_y = y;
			
			// Performance pane takes remaining width on left (no gap)
			perf_width = width - dist_width;
			perf_height = dist_height;
			perf_x = x;
			perf_y = y;
			
			// Zones pane at bottom, full width
			zones_height = height - perf_height;
			zones_y = y + perf_height;
			
			// Create boxes
			perf_box = createBox(perf_x, perf_y, perf_width, perf_height, Theme::c("cpu_box"), true, "server performance", "", 9);
			dist_box = createBox(dist_x, dist_y, dist_width, dist_height, Theme::c("mem_box"), true, "bot distribution", "", 8);
			zones_box = createBox(x, zones_y, width, zones_height, Theme::c("proc_box"), true, "zones", "", 10);
			box = perf_box + dist_box + zones_box;
			
			zone_select_max = zones_height - 4;  // Account for header and borders
			redraw = true;
		}
	#endif
	}
}

#ifdef AZEROTHCORE_SUPPORT
namespace Draw {
	namespace AzerothCore {
		int width_p = 100, height_p = 100;
		int min_width = 80, min_height = 20;
		int x = 1, y = 1, width = 80, height = 25;
		bool shown = false, redraw = false;
		string box;
		
		//* Three-pane layout (SERVER PERFORMANCE left, BOT DISTRIBUTION right, ZONES bottom)
		int perf_width = 50, perf_height = 12;
		int perf_x = 1, perf_y = 1;
		int dist_width = 30, dist_height = 12;
		int dist_x = 51, dist_y = 1;
		int zones_height = 13;
		int zones_y = 13;
		string perf_box;
		string dist_box;
		string zones_box;
		
		//* Zone list scrolling and filtering
		size_t zone_offset = 0;
		size_t zone_select_max = 10;
		Draw::TextEdit zone_filter;
		
	//* Zone selection and expansion
	int selected_zone = 0;
	bool zone_selection_active = false;
	bool zone_filtering = false;
	std::set<size_t> expanded_zones;
	std::set<std::string> expanded_continents;  // Track which continents are expanded
	
	//* Zone scrolling
	int zone_scroll_offset = 0;  // First visible line in zones list
	
	//* Zone sorting
	Draw::AzerothCore::ZoneSortColumn zone_sort_column = Draw::AzerothCore::ZoneSortColumn::NONE;
	bool zone_sort_reverse = false;
	
	//* Display list for zone navigation (defined in header)
	std::vector<Draw::AzerothCore::DisplayItem> zone_display_list;
		
		//* Update intervals (milliseconds since epoch)
		uint64_t last_perf_update = 0;
		uint64_t last_dist_update = 0;
		uint64_t last_zones_update = 0;
		
		//* Graph tracking
		long long last_graph_max = 0;
		
		std::unordered_map<string, Draw::Graph> graphs;
		std::unordered_map<string, Draw::Meter> meters;

		string draw(bool force_redraw, [[maybe_unused]] bool data_same) {
			try {
				if (Runner::stopping) return "";
				if (not shown) return "";
				if (force_redraw) redraw = true;
				
				const auto& data = ::AzerothCore::current_data;
				const auto& enabled = ::AzerothCore::enabled;
				const auto& active = ::AzerothCore::active;
				const auto& load_hist = ::AzerothCore::load_history;
				
				string out;
				out.reserve(width * height);
				
				//* Redraw box outlines for all three panes
				if (redraw or force_redraw) {
					out += perf_box + dist_box + zones_box;
					graphs.clear();
					meters.clear();
					redraw = false;
				}
				
				// Get theme colors
				string main_fg = Theme::c("main_fg");
				string hi_fg = Theme::c("hi_fg");
				string div_line = Theme::c("div_line");
				string title = Theme::c("title");
				
				// If basic theme colors aren't loaded, skip this frame
				if (main_fg.empty() or title.empty()) {
					return "";
				}
			
			//! ======== LEFT PANE: SERVER PERFORMANCE ========
			int cy = perf_y + 1;
			
			if (!enabled) {
				out += Mv::to(cy + 2, perf_x + 4) + main_fg + "Monitor is disabled";
				out += Mv::to(cy + 3, perf_x + 4) + "Set azerothcore_enabled=true in config";
				return out;
			}
			
			if (!active) {
				out += Mv::to(cy + 2, perf_x + 4) + main_fg + "Not connected to server";
				if (!data.error.empty()) {
					out += Mv::to(cy + 4, perf_x + 4) + hi_fg + "Error: " + main_fg + data.error;
				}
				return out;
			}
			
			//* Blank line at top
			cy++;
			
		//* Server URL - extract just hostname
		if (!data.server_url.empty()) {
			// Extract hostname from formats like "root@hostname.domain.com" or "hostname.domain.com"
			string display_hostname = data.server_url;
			
			// Remove user@ prefix if present
			size_t at_pos = display_hostname.find('@');
			if (at_pos != string::npos) {
				display_hostname = display_hostname.substr(at_pos + 1);
			}
			
			// Take only first part before first dot
			size_t dot_pos = display_hostname.find('.');
			if (dot_pos != string::npos) {
				display_hostname = display_hostname.substr(0, dot_pos);
			}
			
			// Clear the line first to prevent ghosting
			out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
			out += Mv::to(cy++, perf_x + 2) + title + "Server: " + main_fg + display_hostname;
		}
			
		//* Server Status (with Uptime when ONLINE)
		using ::AzerothCore::ServerStatus;
		// Clear the entire line first to prevent ghosting
		out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
		out += Mv::to(cy++, perf_x + 2) + title + "Status: ";
		if (data.status == ServerStatus::ONLINE) {
			out += Theme::c("proc_misc") + "ONLINE " + main_fg + "[" 
				+ ::AzerothCore::format_uptime(data.stats.uptime_hours) + "]";
		} else if (data.status == ServerStatus::OFFLINE) {
			out += Theme::c("title") + "OFFLINE";
		} else if (data.status == ServerStatus::RESTARTING) {
			out += Theme::c("title") + "RESTARTING";
		} else if (data.status == ServerStatus::REBUILDING) {
			// Yellow color for REBUILDING status
			out += string("\x1b[93mREBUILDING ") + main_fg + "[" 
				+ to_string((int)data.rebuild_progress) + "%]";
		} else {
			out += Theme::c("inactive_fg") + "ERROR";
		}
		
		//* Container statuses - show when not ONLINE
		if (data.status != ServerStatus::ONLINE && !data.containers.empty()) {
			// Blank line for spacing
			cy++;
			
			// Show each container status
			for (const auto& container : data.containers) {
				out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
				
				// Color based on state
				string state_color;
				if (container.is_running) {
					state_color = Theme::c("proc_misc");  // Green
				} else if (container.state == "restarting") {
					state_color = "\x1b[93m";  // Yellow
				} else if (container.state == "paused") {
					state_color = "\x1b[93m";  // Yellow
				} else {
					state_color = "\x1b[91m";  // Red for exited/dead
				}
				
				// Format: "  worldserver: running"
				out += Mv::to(cy++, perf_x + 4) + title + container.short_name + ": " 
					+ state_color + container.state;
			}
		}
		
		//* Ollama stats - inline with server stats (not right-aligned anymore)
		// Debug logging for Ollama data
		static std::ofstream ollama_draw_log("/tmp/bottop_ollama_draw.txt", std::ios::app);
		ollama_draw_log << "Draw function - enabled: " << data.ollama.enabled 
		                << " rate=" << data.ollama.messages_per_hour 
		                << " recent=" << data.ollama.recent_messages 
		                << " failure=" << data.ollama.failure_rate_60s << std::endl;
		ollama_draw_log.flush();
		
		// Ollama Status
		out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
		out += Mv::to(cy++, perf_x + 2) + title + "Ollama: ";
		if (data.ollama.enabled) {
			out += Theme::c("proc_misc") + "ENABLED";
			
			// Ollama messages since update (recent_messages = last 60 seconds)
			out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
			out += Mv::to(cy++, perf_x + 2) + title + "  Messages: " + main_fg 
				+ to_string(data.ollama.recent_messages) + " (60s)";
			
			// Ollama failure rate
			out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
			out += Mv::to(cy++, perf_x + 2) + title + "  Failure: " + main_fg 
				+ to_string((int)data.ollama.failure_rate_60s) + "%";
		} else {
			out += Theme::c("inactive_fg") + "NOT DETECTED";
		}
		
	//* WorldServer Performance Metrics
		if (data.stats.perf.available) {
			// Clear the line first to prevent ghosting
			out += Mv::to(cy, perf_x + 2) + string(perf_width - 4, ' ');
			
			// Display actual server performance from "server info" - consolidated single line
			// Mean update time with color coding
			string diff_color;
			if (data.stats.perf.mean < 50) {
				diff_color = Theme::c("proc_misc");  // Green - excellent
			} else if (data.stats.perf.mean < 100) {
				diff_color = "\x1b[92m";  // Light green - good
			} else if (data.stats.perf.mean < 150) {
				diff_color = "\x1b[93m";  // Yellow - acceptable
			} else {
				diff_color = "\x1b[91m";  // Red - poor
			}
			
			// Format: "WorldServer UpdateTime: XXms [Mean]" or "WorldServer UpdateTime: XXms [Cached Mean]"
			// Pad [Mean] with spaces to match [Cached Mean] length to prevent overlap
			string metric_type = data.stats.perf.is_cached ? "[Cached Mean]" : "[Mean]       ";
			
			out += Mv::to(cy++, perf_x + 2) + title + "WorldServer UpdateTime: " 
				+ diff_color + to_string(data.stats.perf.mean) + "ms " 
				+ Theme::c("graph_text") + metric_type;
		}
	
	//* Response time bar graph area - display as vertical bars
	int graph_height = perf_height - (cy - perf_y) - 2;  // Reserve 2 lines at bottom for padding
	int scale_width = 6;  // Width for scale labels (e.g., "100ms")
	int graph_width = perf_width - 4 - scale_width;
	
	// Clear the graph area to prevent ghosting
	for (int clear_line = 0; clear_line < graph_height; clear_line++) {
		out += Mv::to(cy + clear_line, perf_x + 2) + string(perf_width - 4, ' ');
	}
		
		if (load_hist.size() > 1 and graph_height > 0 and graph_width > 0) {
			// Fixed max value at 150ms for consistent scale
			const long long max_val = 150;
				
				// Only recreate graph if it doesn't exist
				bool need_recreate = not graphs.contains("load") or last_graph_max != max_val;
				
				if (need_recreate) {
					try {
						// Fixed 150ms max - values above will be clamped
						graphs["load"] = Draw::Graph(graph_width, graph_height, "cpu", load_hist, "braille", false, true, max_val);
						last_graph_max = max_val;
					} catch (const std::exception& e) {
						try {
							graphs["load"] = Draw::Graph(graph_width, graph_height, "", load_hist, "braille", false, true, max_val);
							last_graph_max = max_val;
						} catch (...) {
							out += Mv::to(cy, perf_x + 4) + Theme::c("inactive_fg") + "Graph init error";
							cy++;
							goto skip_graph;
						}
					}
				}
				
				try {
					// Draw scale on the left and graph on the right
					string graph_out = graphs["load"](load_hist, data_same and not need_recreate);
					auto graph_lines = ssplit(graph_out, '\n');
					
				// Draw scale labels and graph lines
				// Avoid division by zero
				int safe_height = max(graph_height - 1, 1);
				for (int i = 0; i < min((int)graph_lines.size(), graph_height); i++) {
					// Calculate the value for this line (top = max, bottom = 0)
					long long scale_val = max_val * (graph_height - i - 1) / safe_height;
					string scale_label = to_string(scale_val) + "ms";
					
					// Draw scale label in dark grey (right-aligned) and graph line
					out += Mv::to(cy + i, perf_x + 2) 
						+ Theme::c("inactive_fg") + rjust(scale_label, scale_width - 1) + " "
						+ graph_lines[i];
				}
				cy += graph_height;
				} catch (const std::exception& e) {
					out += Mv::to(cy, perf_x + 4) + Theme::c("inactive_fg") + "Graph error: " + string(e.what());
					cy++;
				}
			} else {
				out += Mv::to(cy, perf_x + 4) + Theme::c("inactive_fg") + "Collecting data...";
				cy++;
		}
		skip_graph:
		
	//! ======== RIGHT PANE: BOT DISTRIBUTION ========
	int dist_cy = dist_y + 1;
	
	// Clear the entire distribution pane to prevent ghosting
	for (int clear_line = 0; clear_line < dist_height - 2; clear_line++) {
		out += Mv::to(dist_y + 1 + clear_line, dist_x + 1) + string(dist_width - 2, ' ');
	}
		
		// Faction distribution - colored text only, no bars
		if (!data.factions.empty() and dist_cy < dist_y + dist_height - 1) {
			out += Mv::to(dist_cy++, dist_x + 1) + title + "Factions:";
			for (const auto& faction : data.factions) {
				if (dist_cy >= dist_y + dist_height - 1) break;
				
				// Color: Red for Horde, Blue for Alliance
				string faction_color;
				if (faction.name == "Horde") {
					faction_color = "\x1b[38;2;220;20;60m";  // Red (RGB: 220, 20, 60 - Crimson)
				} else if (faction.name == "Alliance") {
					faction_color = "\x1b[38;2;0;120;255m";  // Blue (RGB: 0, 120, 255)
				} else {
					faction_color = Theme::c("hi_fg");  // Default color for others
				}
				
				string percent_str = to_string((int)faction.percent) + "%";
				out += Mv::to(dist_cy, dist_x + 1) + faction_color + ljust(faction.name, 15);
				out += faction_color + rjust(percent_str, 4);
				dist_cy++;
			}
		}
			
			// Continent distribution - text only, no bars
			if (!data.continents.empty() and dist_cy < dist_y + dist_height - 1) {
				out += Mv::to(dist_cy++, dist_x + 1) + title + "Continents:";
				for (const auto& continent : data.continents) {
					if (dist_cy >= dist_y + dist_height - 1) break;
					
					string percent_str = to_string((int)continent.percent) + "%";
					out += Mv::to(dist_cy, dist_x + 1) + Theme::c("inactive_fg") + ljust(continent.name, 15);
					out += main_fg + rjust(percent_str, 4);
					dist_cy++;
				}
			}
			
			// Level distribution (compact) - show ALL brackets even if empty
			if (!data.levels.empty() and dist_cy < dist_y + dist_height - 1) {
				out += Mv::to(dist_cy++, dist_x + 1) + title + "Levels:";
				
				// Get bracket definitions from loaded config (or use defaults)
				const auto& bracket_defs = ::AzerothCore::expected_values.bracket_definitions;
				
				// If no brackets loaded yet, use defaults
				std::vector<std::string> bracket_names;
				if (bracket_defs.empty()) {
					bracket_names = {"1-10", "11-20", "21-30", "31-40", "41-50", "51-60", "61-70", "71-80"};
				} else {
					for (const auto& def : bracket_defs) {
						bracket_names.push_back(def.range);
					}
				}
				
				// Display all brackets in order
				for (const auto& bracket_name : bracket_names) {
					if (dist_cy >= dist_y + dist_height - 1) break;
					
					// Find this bracket in the data
					auto it = std::find_if(data.levels.begin(), data.levels.end(),
						[&bracket_name](const auto& lb) { return lb.range == bracket_name; });
		
		double percent = (it != data.levels.end()) ? it->percent : 0.0;
		
		// Find expected percentage from server config for color indicator
		double expected_percent = 0.0;
		bool has_expected = false;
		if (::AzerothCore::expected_values.loaded) {
			auto exp_it = std::find_if(::AzerothCore::expected_values.level_distribution.begin(),
			                            ::AzerothCore::expected_values.level_distribution.end(),
			                            [&bracket_name](const auto& lb) { return lb.range == bracket_name; });
			if (exp_it != ::AzerothCore::expected_values.level_distribution.end()) {
				expected_percent = exp_it->percent;
				has_expected = true;
			}
		}
		
		// 3-tier color system based on deviation from expected
		// Green: ±0-3% (on target)
		// Yellow: ±3-6% (warning)
		// Red: ±6-9%+ (critical)
		string line_color = Theme::c("inactive_fg");  // Default grey
		if (has_expected && expected_percent > 0) {
			double deviation = abs(percent - expected_percent);
			
			if (deviation <= 3.0) {
				// Green - on target (±3%)
				line_color = Theme::c("proc_misc");  // Bright green
			} else if (deviation <= 6.0) {
				// Yellow - warning (±6%)
				line_color = Theme::c("available_end");  // Bright yellow/orange
			} else if (deviation <= 9.0) {
				// Red - critical (±9%)
				line_color = Theme::c("used_end");  // Bright red
			} else {
				// Beyond 9% - use title color (bright white) to indicate severe deviation
				line_color = Theme::c("title");
			}
		}
		
		string bracket_str = ljust(bracket_name, 13);
		string percent_str = to_string((int)percent) + "%";
		
		out += Mv::to(dist_cy, dist_x + 1) + line_color + bracket_str;
		out += line_color + rjust(percent_str, 4);
		dist_cy++;
	}
}
			
	//! ======== BOTTOM PANE: ZONES ========
	cy = zones_y + 1;
	
	// Clear the entire zones pane to prevent ghosting
	for (int clear_line = 0; clear_line < zones_height - 2; clear_line++) {
		out += Mv::to(zones_y + 1 + clear_line, x + 1) + string(width - 2, ' ');
	}
	cy = zones_y + 1;  // Reset cy after clearing
	
	//* Auto-expand all continents on first run
	static bool first_run = true;
	if (first_run && !data.zones.empty()) {
		for (const auto& zone : data.zones) {
			expanded_continents.insert(zone.continent);
		}
		first_run = false;
	}
	
	//* Create sorted index vector for zones
	std::vector<size_t> sorted_indices(data.zones.size());
	for (size_t i = 0; i < data.zones.size(); i++) {
		sorted_indices[i] = i;
	}
		
	// Sort indices based on current sort column
	std::sort(sorted_indices.begin(), sorted_indices.end(), [&data](size_t a_idx, size_t b_idx) {
		const auto& a = data.zones[a_idx];
		const auto& b = data.zones[b_idx];
		
		int result = 0;
		
		// Primary sort: expected level bracket (if no column sort active)
		if (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::NONE) {
			// Sort by expected_min first
			if (a.expected_min != b.expected_min) {
				result = (a.expected_min < b.expected_min) ? -1 : 1;
			}
			// If expected_min is same, sort by expected_max
			else if (a.expected_max != b.expected_max) {
				result = (a.expected_max < b.expected_max) ? -1 : 1;
			}
			// If both levels same, sort by name
			else {
				result = a.name.compare(b.name);
			}
		}
		// Column sort (when user has selected a column)
		else {
			switch (zone_sort_column) {
				case Draw::AzerothCore::ZoneSortColumn::NAME:
					result = a.name.compare(b.name);
					break;
				case Draw::AzerothCore::ZoneSortColumn::BOTS:
					result = (a.total < b.total) ? -1 : (a.total > b.total) ? 1 : 0;
					break;
				case Draw::AzerothCore::ZoneSortColumn::MIN_LEVEL:
					result = (a.expected_min < b.expected_min) ? -1 : (a.expected_min > b.expected_min) ? 1 : 0;
					break;
				case Draw::AzerothCore::ZoneSortColumn::MAX_LEVEL:
					result = (a.expected_max < b.expected_max) ? -1 : (a.expected_max > b.expected_max) ? 1 : 0;
					break;
				case Draw::AzerothCore::ZoneSortColumn::ALIGNMENT:
					result = (a.alignment < b.alignment) ? -1 : (a.alignment > b.alignment) ? 1 : 0;
					break;
				case Draw::AzerothCore::ZoneSortColumn::NONE:
					// Already handled above
					break;
			}
			
			// Apply reverse sort if enabled (only for column sorts)
			if (zone_sort_reverse) {
				result = -result;
			}
			
			// Tertiary sort: if column result is tied, fall back to level bracket
			if (result == 0) {
				// Sort by expected_min first
				if (a.expected_min != b.expected_min) {
					result = (a.expected_min < b.expected_min) ? -1 : 1;
				}
				// If expected_min is same, sort by expected_max
				else if (a.expected_max != b.expected_max) {
					result = (a.expected_max < b.expected_max) ? -1 : 1;
				}
				// If both levels same, sort by name
				else {
					result = a.name.compare(b.name);
				}
			}
		}
		
		return result < 0;
	});
		
	//* Calculate total bots across all zones
	int total_bots = 0;
	std::unordered_map<string, int> continent_totals;
	for (const auto& zone : data.zones) {
		total_bots += zone.total;
		continent_totals[zone.continent] += zone.total;
	}
	
	//* Build display list for navigation using sorted indices
	zone_display_list.clear();
	string prep_continent = "";
	string prep_region = "";
	string filter_text_lower = zone_filter.text;
	std::transform(filter_text_lower.begin(), filter_text_lower.end(), filter_text_lower.begin(), ::tolower);
	
for (size_t sorted_idx = 0; sorted_idx < sorted_indices.size(); sorted_idx++) {
	size_t orig_idx = sorted_indices[sorted_idx];
	const auto& zone = data.zones[orig_idx];
	
	// Apply filter if active
	bool zone_matches_filter = true;
	if (!filter_text_lower.empty()) {
		string zone_name_lower = zone.name;
		std::transform(zone_name_lower.begin(), zone_name_lower.end(), zone_name_lower.begin(), ::tolower);
		if (zone_name_lower.find(filter_text_lower) == string::npos) {
			zone_matches_filter = false;
		}
	}
	
	// Skip zones that don't match filter
	if (!zone_matches_filter) {
		continue;
	}
	
	// Simply add the zone to the display list (no continent/region grouping)
	zone_display_list.push_back({Draw::AzerothCore::DisplayItem::ZONE, orig_idx, zone.name});
	}
			
		// Clamp selected zone to display list bounds
		if (selected_zone >= (int)zone_display_list.size()) {
			selected_zone = max(0, (int)zone_display_list.size() - 1);
		}
		
		//* Header with sort indicators
		string sort_indicator = zone_sort_reverse ? "▲" : "▼";
		string name_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::NAME) ? " " + sort_indicator : "";
		string bots_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::BOTS) ? " " + sort_indicator : "";
	string min_lvl_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::MIN_LEVEL) ? " " + sort_indicator : "";
	string max_lvl_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::MAX_LEVEL) ? " " + sort_indicator : "";
	string align_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::ALIGNMENT) ? " " + sort_indicator : "";
	
	// Use combined sort indicator for expected levels (if either min or max level is being sorted)
	string expected_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::MIN_LEVEL || 
	                        zone_sort_column == Draw::AzerothCore::ZoneSortColumn::MAX_LEVEL) ? " " + sort_indicator : "";
	
	// Column layout: Name (30) | Bots (6) | Expected (10) | Min Lvl (4) | Max Lvl (4) | Align (5)
	out += Mv::to(cy, x + 2) + title + "Name" + name_sort;
	out += Mv::to(cy, x + 34) + title + "Bots" + bots_sort;
	out += Mv::to(cy, x + 44) + "Expected" + expected_sort;
	out += Mv::to(cy, x + 57) + "Min";
	out += Mv::to(cy, x + 64) + "Max";
	out += Mv::to(cy, x + 71) + "Align" + align_sort;
	cy++;
	out += Mv::to(cy, x + 2) + div_line + Symbols::h_line * (width - 4);
	cy++;
	
	//* All zones total line
	out += Mv::to(cy, x + 2) + Theme::c("title") + "All Zones:";
	out += Mv::to(cy, x + 34) + Theme::c("hi_fg") + rjust(to_string(total_bots), 6);
	cy++;
		
			//* Zone list (hierarchical by Continent > Region > Zone)
			if (!data.zones.empty() && !zone_display_list.empty()) {
				int displayed_rows = 0;
				size_t list_height = zone_select_max;
				
				// Adjust scroll offset to keep selected item visible
				if (zone_selection_active) {
					// If selected item is above visible area, scroll up
					if (selected_zone < zone_scroll_offset) {
						zone_scroll_offset = selected_zone;
					}
					// If selected item is below visible area, scroll down
					else if (selected_zone >= zone_scroll_offset + (int)list_height) {
						zone_scroll_offset = selected_zone - (int)list_height + 1;
					}
				}
				
				// Clamp scroll offset
				int max_scroll = max(0, (int)zone_display_list.size() - (int)list_height);
				zone_scroll_offset = clamp(zone_scroll_offset, 0, max_scroll);
				
				for (size_t display_idx = zone_scroll_offset; display_idx < zone_display_list.size(); display_idx++) {
					// Check if we have room to display
					if (cy >= zones_y + zones_height - 2) break;
					if (displayed_rows >= (int)list_height) break;
					
					const auto& item = zone_display_list[display_idx];
					bool is_selected = zone_selection_active && (int)display_idx == selected_zone;
					
				// Only render zones (no continent/region headers)
				if (item.type == Draw::AzerothCore::DisplayItem::ZONE) {
					const auto& zone = data.zones[item.zone_index];
					
					string health_color = zone.is_healthy() ? Theme::c("proc_misc") : Theme::c("title");
					string status_icon = zone.is_healthy() ? "●" : "⚠";
					string selection_indicator = is_selected ? "► " : "  ";
					string row_fg = is_selected ? Theme::c("hi_fg") : main_fg;
					
					// Format expected level range as "min-max"
					string expected_range = to_string(zone.expected_min) + "-" + to_string(zone.expected_max);
					
					// Color actual levels based on whether they match expected
					string min_color = (zone.actual_min < zone.expected_min || zone.actual_min > zone.expected_max) ? 
					                   Theme::c("inactive_fg") : row_fg;
					string max_color = (zone.actual_max < zone.expected_min || zone.actual_max > zone.expected_max) ? 
					                   Theme::c("inactive_fg") : row_fg;
					
					out += Mv::to(cy, x + 2);
					out += row_fg + selection_indicator;
					out += health_color + status_icon + " ";
					out += row_fg + ljust(zone.name, 28);
					out += Mv::to(cy, x + 34) + row_fg + rjust(to_string(zone.total), 6);
					out += Mv::to(cy, x + 44) + (is_selected ? row_fg : Theme::c("inactive_fg")) + rjust(expected_range, 10);
					out += Mv::to(cy, x + 57) + min_color + rjust(to_string(zone.actual_min), 3);
					out += Mv::to(cy, x + 64) + max_color + rjust(to_string(zone.actual_max), 3);
					out += Mv::to(cy, x + 71) + (zone.alignment >= 80.0 ? Theme::c("proc_misc") 
						: zone.alignment >= 60.0 ? row_fg 
						: Theme::c("title")) 
						+ rjust(to_string((int)zone.alignment), 3) + "%";
					cy++;
					displayed_rows++;
				}
				}
				
			// Navigation help (clear bottom line first to prevent ghosting)
			out += Mv::to(zones_y + zones_height - 1, x + 1) + string(width - 2, ' ');
			if (zone_filtering) {
				// Show filter input box
				out += Mv::to(zones_y + zones_height - 1, x + 2) + Theme::c("hi_fg") 
					+ "Filter: " + zone_filter.text + "_";
			}
		else if (zone_selection_active) {
			out += Mv::to(zones_y + zones_height - 1, x + 2) + Theme::c("hi_fg") 
				+ "↑↓:Nav  PgUp/PgDn:FastScroll  Home/End:Jump  n/b/m/M/a:Sort  r:Reverse  f:Filter";
		} else {
				out += Mv::to(zones_y + zones_height - 1, x + 2) + Theme::c("graph_text") 
					+ "Arrow keys to navigate zones";
			}
			} else {
				out += Mv::to(cy, x + 4) + Theme::c("inactive_fg") + "No zones found";
			}
			
			//* Timestamp
			if (!data.timestamp.empty()) {
				out += Mv::to(zones_y + zones_height - 1, x + width - 20) + Theme::c("graph_text") + data.timestamp;
			}
			
			return out;
			} catch (const std::exception& e) {
				Logger::error("AzerothCore::draw() -> " + string(e.what()));
				return "";
			}
		}
	}
}
#endif
