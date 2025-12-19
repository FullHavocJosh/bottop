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

#include "btop_azerothcore.hpp"
#include "btop_tools.hpp"
#include <libssh2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sys/stat.h>
#include <map>
#include <pwd.h>
#include <algorithm>
#include <fstream>
#include <fstream>
#include <regex>
#include <iostream>

using namespace Tools;

namespace AzerothCore {

	//* Global state
	std::atomic<bool> enabled{false};
	std::atomic<bool> active{false};
	ServerConfig config;
	std::unique_ptr<CommandExecutor> executor;  // Can be SSHClient or LocalExecutor
	std::unique_ptr<Query> query;
	ServerData current_data;
	ExpectedValues expected_values;
	
	//* Historical data for graphs
	std::deque<long long> load_history;
	
	//* Cached performance data (last known good values)
	::AzerothCore::ServerPerformance last_known_perf;
	uint64_t last_perf_update_time = 0;
	const uint64_t PERF_UPDATE_INTERVAL_MS = 5000;  // Update every 5 seconds
	
	//* Track previous server status for disconnection detection
	ServerStatus previous_status = ServerStatus::ONLINE;
	
	//* Config refresh tracking
	uint64_t last_config_refresh_time = 0;
	const uint64_t CONFIG_REFRESH_INTERVAL_MS = 90000;  // Refresh config every 90 seconds

	//* LocalExecutor implementation
	std::string LocalExecutor::execute(const std::string& command) {
		Logger::error("LOCAL EXEC: " + command);
		
		FILE* pipe = popen(command.c_str(), "r");
		if (!pipe) {
			error_ = "Failed to execute command locally";
			return "";
		}
		
		std::ostringstream output;
		char buffer[4096];
		while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
			output << buffer;
		}
		
		int status = pclose(pipe);
		if (status != 0 && status != -1) {
			error_ = "Command exited with status " + std::to_string(status);
		}
		
		return output.str();
	}

	//* SSHClient implementation
	SSHClient::SSHClient(const std::string& host) : host_(host) {
		libssh2_init(0);
	}

	SSHClient::~SSHClient() {
		if (session_) {
			auto* session = static_cast<LIBSSH2_SESSION*>(session_);
			libssh2_session_disconnect(session, "Normal shutdown");
			libssh2_session_free(session);
		}
		if (sock_ != -1) {
			close(sock_);
		}
		libssh2_exit();
	}

	bool SSHClient::connect() {
		// Parse host (format: user@hostname:port or user@hostname)
		std::string user, hostname;
		int port = 22;
		
		Logger::error("SSHClient::connect() called with host: " + host_);
		
		size_t at_pos = host_.find('@');
		if (at_pos == std::string::npos) {
			error_ = "Invalid host format. Expected: user@hostname[:port]";
			Logger::error("SSHClient::connect() failed: no @ in host");
			return false;
		}
		
		user = host_.substr(0, at_pos);
		std::string host_part = host_.substr(at_pos + 1);
		
		size_t colon_pos = host_part.find(':');
		if (colon_pos != std::string::npos) {
			hostname = host_part.substr(0, colon_pos);
			port = std::stoi(host_part.substr(colon_pos + 1));
		} else {
			hostname = host_part;
		}
		
		Logger::error("SSHClient::connect() parsed: user=" + user + " hostname=" + hostname + " port=" + std::to_string(port));
		
		// Resolve hostname
		struct hostent* host_info = gethostbyname(hostname.c_str());
		if (!host_info) {
			error_ = "Could not resolve hostname: " + hostname;
			Logger::error("SSHClient::connect() failed: gethostbyname() returned null");
			return false;
		}
		
		Logger::error("SSHClient::connect() resolved hostname successfully");
		
		// Create socket
		sock_ = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_ == -1) {
			error_ = "Failed to create socket: " + std::string(strerror(errno));
			Logger::error("SSHClient::connect() failed: socket() returned -1, errno=" + std::to_string(errno));
			return false;
		}
		
		Logger::error("SSHClient::connect() created socket: " + std::to_string(sock_));
		
		// Connect to server
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr = *reinterpret_cast<struct in_addr*>(host_info->h_addr);
		
		Logger::error("SSHClient::connect() attempting socket connect...");
		if (::connect(sock_, reinterpret_cast<struct sockaddr*>(&sin), sizeof(sin)) != 0) {
			error_ = "Failed to connect to " + hostname + ":" + std::to_string(port) + " - " + std::string(strerror(errno));
			Logger::error("SSHClient::connect() failed: connect() returned error, errno=" + std::to_string(errno) + " msg=" + std::string(strerror(errno)));
			close(sock_);
			sock_ = -1;
			return false;
		}
		
		Logger::error("SSHClient::connect() socket connected successfully");
		
		// Create SSH session
		session_ = libssh2_session_init();
		if (!session_) {
			error_ = "Failed to initialize SSH session";
			close(sock_);
			sock_ = -1;
			return false;
		}
		
		auto* session = static_cast<LIBSSH2_SESSION*>(session_);
		
		// Start SSH handshake
		if (libssh2_session_handshake(session, sock_) != 0) {
			char* err_msg;
			libssh2_session_last_error(session, &err_msg, nullptr, 0);
			error_ = std::string("SSH handshake failed: ") + err_msg;
			libssh2_session_free(session);
			session_ = nullptr;
			close(sock_);
			sock_ = -1;
			return false;
		}
		
		// Get home directory
		const char* home_dir = getenv("HOME");
		if (!home_dir) {
			struct passwd* pw = getpwuid(getuid());
			if (pw) {
				home_dir = pw->pw_dir;
			}
		}
		
		if (!home_dir) {
			error_ = "Could not determine home directory";
			libssh2_session_free(session);
			session_ = nullptr;
			close(sock_);
			sock_ = -1;
			return false;
		}
		
		// Try common SSH key locations
		std::vector<std::pair<std::string, std::string>> key_pairs = {
			{std::string(home_dir) + "/.ssh/id_ed25519", std::string(home_dir) + "/.ssh/id_ed25519.pub"},
			{std::string(home_dir) + "/.ssh/id_rsa", std::string(home_dir) + "/.ssh/id_rsa.pub"},
			{std::string(home_dir) + "/.ssh/id_ecdsa", std::string(home_dir) + "/.ssh/id_ecdsa.pub"},
			{std::string(home_dir) + "/.ssh/id_dsa", std::string(home_dir) + "/.ssh/id_dsa.pub"}
		};
		
		bool auth_success = false;
		std::string last_auth_error;
		
		for (const auto& key_pair : key_pairs) {
			const std::string& private_key = key_pair.first;
			const std::string& public_key = key_pair.second;
			
			// Check if private key exists
			struct stat st;
			if (stat(private_key.c_str(), &st) != 0) {
				continue;  // Key doesn't exist, try next
			}
			
			// Try authentication with this key
			int rc = libssh2_userauth_publickey_fromfile(
				session, 
				user.c_str(),
				public_key.c_str(),
				private_key.c_str(),
				nullptr  // No passphrase
			);
			
			if (rc == 0) {
				auth_success = true;
				break;
			} else {
				char* err_msg;
				libssh2_session_last_error(session, &err_msg, nullptr, 0);
				last_auth_error = err_msg;
			}
		}
		
		if (!auth_success) {
			error_ = "SSH authentication failed. Last error: " + last_auth_error;
			libssh2_session_free(session);
			session_ = nullptr;
			close(sock_);
			sock_ = -1;
			return false;
		}
		
		// Set non-blocking mode
		libssh2_session_set_blocking(session, 0);
		
		return true;
	}

	std::string SSHClient::execute(const std::string& command) {
		if (!session_) {
			error_ = "Not connected";
			return "";
		}
		
		auto* session = static_cast<LIBSSH2_SESSION*>(session_);
		LIBSSH2_CHANNEL* channel = nullptr;
		
		// Open channel with timeout
		auto start_time = std::chrono::steady_clock::now();
		const auto timeout = std::chrono::seconds(10);
		
		while ((channel = libssh2_channel_open_session(session)) == nullptr &&
			   libssh2_session_last_error(session, nullptr, nullptr, 0) == LIBSSH2_ERROR_EAGAIN) {
			if (std::chrono::steady_clock::now() - start_time > timeout) {
				error_ = "Timeout opening SSH channel";
				return "";
			}
			usleep(10000); // Sleep 10ms to avoid busy-waiting
		}
		
		if (!channel) {
			char* err_msg;
			libssh2_session_last_error(session, &err_msg, nullptr, 0);
			error_ = std::string("Failed to open channel: ") + err_msg;
			return "";
		}
		
		// Execute command with timeout
		start_time = std::chrono::steady_clock::now();
		while (libssh2_channel_exec(channel, command.c_str()) == LIBSSH2_ERROR_EAGAIN) {
			if (std::chrono::steady_clock::now() - start_time > timeout) {
				error_ = "Timeout executing command";
				libssh2_channel_free(channel);
				return "";
			}
			usleep(10000);
		}
		
		// Read output with timeout
		std::ostringstream output;
		char buffer[4096];
		int rc;
		start_time = std::chrono::steady_clock::now();
		
		while (true) {
			do {
				rc = libssh2_channel_read(channel, buffer, sizeof(buffer));
				if (rc > 0) {
					output.write(buffer, rc);
					start_time = std::chrono::steady_clock::now(); // Reset timeout on data received
				}
			} while (rc > 0);
			
			if (rc == LIBSSH2_ERROR_EAGAIN) {
				if (std::chrono::steady_clock::now() - start_time > timeout) {
					error_ = "Timeout reading command output";
					libssh2_channel_close(channel);
					libssh2_channel_free(channel);
					return "";
				}
				usleep(10000); // Sleep 10ms to avoid busy-waiting
				continue;
			} else {
				break;
			}
		}
		
		// Close channel with timeout
		start_time = std::chrono::steady_clock::now();
		while (libssh2_channel_close(channel) == LIBSSH2_ERROR_EAGAIN) {
			if (std::chrono::steady_clock::now() - start_time > timeout) {
				error_ = "Timeout closing SSH channel";
				break;
			}
			usleep(10000);
		}
		
		libssh2_channel_free(channel);
		
		return output.str();
	}

	bool SSHClient::is_connected() const {
		return session_ != nullptr && sock_ != -1;
	}

	std::string SSHClient::last_error() const {
		return error_;
	}

	//* Query implementation
	Query::Query(CommandExecutor& executor, const ServerConfig& config)
		: executor_(executor), config_(config) {
		// Cache excluded account IDs on construction
		cache_excluded_accounts();
	}

	std::string Query::mysql_exec(const std::string& query) {
	std::ostringstream cmd;
	cmd << "docker exec " << config_.container 
		<< " mysql -h" << config_.db_host
		<< " -u" << config_.db_user
		<< " -p" << config_.db_pass
		<< " -D" << config_.db_name
		<< " -sN -e \"" << query << "\" 2>/dev/null";  // Suppress MySQL warnings
	
	Logger::error("MYSQL DEBUG: Executing command: " + cmd.str());
	std::string result = executor_.execute(cmd.str());
	Logger::error("MYSQL DEBUG: Got result (length=" + std::to_string(result.length()) + "): '" + result.substr(0, 100) + "'");
	return result;
	}
	
	void Query::cache_excluded_accounts() {
		// Fetch account IDs for excluded usernames once and cache them
		std::string result = mysql_exec(
			"SELECT GROUP_CONCAT(id) FROM acore_auth.account "
			"WHERE username IN ('HAVOC','JOSHG','JOSHR','JON','CAITR','COLTON','KELSEYG','KYLAN','SETH','AHBOT');"
		);
		
		if (!result.empty() && result != "NULL") {
			excluded_account_ids_ = result;
			// Remove any trailing whitespace/newlines
			excluded_account_ids_.erase(excluded_account_ids_.find_last_not_of(" \n\r\t") + 1);
		} else {
			// If no accounts found, use impossible ID to avoid syntax errors
			excluded_account_ids_ = "-1";
		}
		
		Logger::debug("Cached excluded account IDs: " + excluded_account_ids_);
	}
	
	std::string Query::get_excluded_accounts_filter() {
		// Return optimized filter using cached account IDs
		return "account NOT IN (" + excluded_account_ids_ + ")";
	}

	BotStats Query::fetch_bot_stats() {
	BotStats stats;
	
	Logger::error("FETCH DEBUG: fetch_bot_stats() starting");
	
	// Measure query performance by timing a simple count query
	auto query_start = std::chrono::high_resolution_clock::now();
	
	// Optimized query - use cached excluded account IDs (no subquery needed)
	std::string result = mysql_exec(
		"SELECT COUNT(*) FROM characters "
		"WHERE online = 1 "
		"AND " + get_excluded_accounts_filter() + ";"
	);
	
	Logger::error("FETCH DEBUG: Bot count query returned: '" + result + "'");
	
	auto query_end = std::chrono::high_resolution_clock::now();
	auto query_duration = std::chrono::duration_cast<std::chrono::milliseconds>(query_end - query_start);
	
	// Store the total round-trip query time (SSH + Docker + MySQL)
	stats.update_time_avg = query_duration.count();
	
	if (!result.empty()) {
		stats.total = std::stoi(result);
		Logger::error("FETCH DEBUG: Parsed total=" + std::to_string(stats.total));
	} else {
		Logger::error("FETCH DEBUG: Empty result!");
	}
	
	// Get server uptime from container start time
	std::ostringstream cmd;
	cmd << "docker inspect " << config_.container << " --format='{{.State.StartedAt}}'";
	
	result = executor_.execute(cmd.str());
	if (!result.empty()) {
		// Parse ISO 8601 timestamp: 2025-12-11T16:27:03.505639176Z
		// Extract year, month, day, hour, minute, second
		int year, month, day, hour, minute, second;
		if (sscanf(result.c_str(), "%d-%d-%dT%d:%d:%d", 
				   &year, &month, &day, &hour, &minute, &second) == 6) {
			
			// Create start time
			struct tm start_tm = {};
			start_tm.tm_year = year - 1900;
			start_tm.tm_mon = month - 1;
			start_tm.tm_mday = day;
			start_tm.tm_hour = hour;
			start_tm.tm_min = minute;
			start_tm.tm_sec = second;
			
			time_t start_time = timegm(&start_tm);  // Use timegm for UTC
			time_t now = time(nullptr);
			
			// Calculate uptime in seconds
			double uptime_seconds = difftime(now, start_time);
			stats.uptime_hours = uptime_seconds / 3600.0;  // Convert to hours
		}
	}
	
	// Fetch real server performance metrics from "server info" command
	stats.perf = Query::fetch_server_performance();
	
	return stats;
	}
	
	::AzerothCore::ServerPerformance Query::fetch_server_performance() {
		// Check if we should fetch new data (1 second interval)
		auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();
		
		// If less than 1 second since last update, return cached data
		if (last_perf_update_time > 0 && 
			(now_ms - last_perf_update_time) < PERF_UPDATE_INTERVAL_MS &&
			last_known_perf.available) {
			Logger::debug("fetch_server_performance: Using cached data (age: " + 
				std::to_string(now_ms - last_perf_update_time) + "ms)");
			return last_known_perf;
		}
		
		::AzerothCore::ServerPerformance perf;
		
	// Execute "server info" command
	// Method 1: Try RA if credentials are configured
	// Method 2: Fallback to docker logs parsing
	std::ostringstream cmd;
	
	// Use expect script to automate docker attach interaction
	// This is more reliable than RA authentication
	cmd << "timeout 10 /usr/bin/expect -c '"
		<< "spawn docker attach " << config_.container << "; "
		<< "sleep 1; "
		<< "send \"\\r\"; "
		<< "expect \"AC>\"; "
		<< "send \"server info\\r\"; "
		<< "expect \"AC>\"; "
		<< "send \"\\x10\"; "
		<< "sleep 0.1; "
		<< "send \"\\x11\"; "
		<< "expect eof"
		<< "' 2>&1";
	
	Logger::debug("fetch_server_performance: Executing: " + cmd.str());
	std::string result = executor_.execute(cmd.str());
	Logger::debug("fetch_server_performance: Result length: " + std::to_string(result.length()));
	
	// Debug: write to file
	std::ofstream debug_file("/tmp/bottop_debug.txt", std::ios::app);
	debug_file << "=== fetch_server_performance ===" << std::endl;
	debug_file << "Command: " << cmd.str() << std::endl;
	debug_file << "Result length: " << result.length() << std::endl;
	debug_file << "Result: " << result << std::endl;
	
	if (result.empty()) {
		debug_file << "Empty result, returning" << std::endl;
		debug_file.close();
		Logger::debug("fetch_server_performance: Empty result from server info command");
		return perf;
	}
	
	// Strip ANSI escape codes
	std::regex ansi_regex("\033\\[[0-9;]*[mKH]|\\[\\?[0-9]+[lh]");
	result = std::regex_replace(result, ansi_regex, "");
	
	// Filter out bot logging lines
	std::istringstream pre_stream(result);
	std::ostringstream filtered;
	std::string pre_line;
	while (std::getline(pre_stream, pre_line)) {
		// Skip bot logging lines
		if (pre_line.find("[BotLevelBrackets]") != std::string::npos ||
			pre_line.find("AHBot") != std::string::npos ||
			pre_line.find("spawn docker") != std::string::npos ||
			pre_line.find("read escape sequence") != std::string::npos) {
			continue;
		}
		filtered << pre_line << "\n";
	}
	result = filtered.str();
	
	debug_file << "Filtered result: " << result << std::endl;
	debug_file.close();
		
		// Parse the output line by line
		// Example:
		// AzerothCore rev. ece1060fa05d+ 2025-12-12 19:37:01 +0000 (Testing-Playerbot branch) (Unix, RelWithDebInfo, Static)
		// Connected players: 1. Characters in world: 3063.
		// Connection peak: 1.
		// Server uptime: 9 hour(s) 35 minute(s) 54 second(s)
		// Update time diff: 41ms. Last 500 diffs summary:
		// |- Mean: 120ms
		// |- Median: 106ms
		// |- Percentiles (95, 99, max): 243ms, 278ms, 314ms
		
		std::istringstream stream(result);
	std::string line;
	
	while (std::getline(stream, line)) {
		// Strip "AC>" prefix if present
		if (line.find("AC>") == 0) {
			line = line.substr(3);
			// Trim leading whitespace
			line.erase(0, line.find_first_not_of(" \t"));
		}
		
		// Skip empty lines
		if (line.empty()) continue;
		
		// Parse "AzerothCore rev. ece1060fa05d+ 2025-12-12 19:37:01 +0000 (Testing-Playerbot branch) (Unix, RelWithDebInfo, Static)"
		if (line.find("AzerothCore rev.") != std::string::npos) {
				// Extract revision
				size_t rev_pos = line.find("rev. ");
				if (rev_pos != std::string::npos) {
					std::string rest = line.substr(rev_pos + 5);
					size_t space_pos = rest.find(' ');
					if (space_pos != std::string::npos) {
						perf.revision = rest.substr(0, space_pos);
					}
				}
				
				// Extract branch name from parentheses
				size_t branch_start = line.find("(");
				size_t branch_end = line.find(" branch)");
				if (branch_start != std::string::npos && branch_end != std::string::npos) {
					perf.branch = line.substr(branch_start + 1, branch_end - branch_start - 1);
				}
				
				// Extract build type (RelWithDebInfo, Debug, Release)
				size_t build_start = line.rfind("(");
				size_t build_end = line.rfind(")");
				if (build_start != std::string::npos && build_end != std::string::npos) {
					std::string build_info = line.substr(build_start + 1, build_end - build_start - 1);
					// Parse "Unix, RelWithDebInfo, Static"
					std::istringstream build_stream(build_info);
					std::string token;
					int count = 0;
					while (std::getline(build_stream, token, ',')) {
						token.erase(0, token.find_first_not_of(" \t"));
						token.erase(token.find_last_not_of(" \t") + 1);
						if (count == 1) {  // Second token is build type
							perf.build_type = token;
							break;
						}
						count++;
					}
				}
				
				// Extract build date (between rev and first paren)
				size_t date_start = line.find(" ", rev_pos + 15);  // After revision hash
				size_t date_end = line.find("(", date_start);
				if (date_start != std::string::npos && date_end != std::string::npos) {
					perf.build_date = line.substr(date_start + 1, date_end - date_start - 2);
					perf.build_date.erase(0, perf.build_date.find_first_not_of(" \t"));
					perf.build_date.erase(perf.build_date.find_last_not_of(" \t") + 1);
				}
			}
			// Parse "Connected players: 1. Characters in world: 3063."
			else if (line.find("Connected players:") != std::string::npos) {
				// Extract connected players
				size_t pos = line.find("Connected players:");
				if (pos != std::string::npos) {
					std::string rest = line.substr(pos + 18);
					size_t dot_pos = rest.find('.');
					if (dot_pos != std::string::npos) {
						std::string num_str = rest.substr(0, dot_pos);
						num_str.erase(0, num_str.find_first_not_of(" \t"));
						num_str.erase(num_str.find_last_not_of(" \t") + 1);
						perf.connected_players = std::stoi(num_str);
					}
				}
				
				// Extract characters in world
				size_t chars_pos = line.find("Characters in world:");
				if (chars_pos != std::string::npos) {
					std::string rest = line.substr(chars_pos + 20);
					size_t dot_pos = rest.find('.');
					if (dot_pos != std::string::npos) {
						std::string num_str = rest.substr(0, dot_pos);
						num_str.erase(0, num_str.find_first_not_of(" \t"));
						num_str.erase(num_str.find_last_not_of(" \t") + 1);
						perf.characters_in_world = std::stoi(num_str);
					}
				}
			}
			// Parse "Connection peak: 1."
			else if (line.find("Connection peak:") != std::string::npos) {
				size_t pos = line.find(":");
				if (pos != std::string::npos) {
					std::string rest = line.substr(pos + 1);
					size_t dot_pos = rest.find('.');
					if (dot_pos != std::string::npos) {
						std::string num_str = rest.substr(0, dot_pos);
						num_str.erase(0, num_str.find_first_not_of(" \t"));
						num_str.erase(num_str.find_last_not_of(" \t") + 1);
						perf.connection_peak = std::stoi(num_str);
					}
				}
			}
			// Parse "Server uptime: 9 hour(s) 35 minute(s) 54 second(s)"
			else if (line.find("Server uptime:") != std::string::npos) {
				size_t pos = line.find(":");
				if (pos != std::string::npos) {
					perf.uptime = line.substr(pos + 2);  // Skip ": "
					
					// Calculate total seconds
					long long hours = 0, minutes = 0, seconds = 0;
					std::istringstream uptime_stream(perf.uptime);
					std::string token;
					while (uptime_stream >> token) {
						if (std::isdigit(token[0])) {
							long long value = std::stoll(token);
							std::string next;
							uptime_stream >> next;
							if (next.find("hour") != std::string::npos) {
								hours = value;
							} else if (next.find("minute") != std::string::npos) {
								minutes = value;
							} else if (next.find("second") != std::string::npos) {
								seconds = value;
							}
						}
					}
					perf.uptime_seconds = hours * 3600 + minutes * 60 + seconds;
				}
			}
			// Parse "Update time diff: 41ms"
			else if (line.find("Update time diff:") != std::string::npos) {
				size_t pos = line.find(":");
				if (pos != std::string::npos) {
					std::string value_str = line.substr(pos + 1);
					// Extract number before "ms"
					size_t ms_pos = value_str.find("ms");
					if (ms_pos != std::string::npos) {
						value_str = value_str.substr(0, ms_pos);
						// Trim whitespace
						value_str.erase(0, value_str.find_first_not_of(" \t"));
						value_str.erase(value_str.find_last_not_of(" \t") + 1);
						perf.update_time_diff = std::stoll(value_str);
					}
				}
			}
			// Parse "|- Mean: 120ms"
			else if (line.find("Mean:") != std::string::npos) {
				size_t pos = line.find(":");
				if (pos != std::string::npos) {
					std::string value_str = line.substr(pos + 1);
					size_t ms_pos = value_str.find("ms");
					if (ms_pos != std::string::npos) {
						value_str = value_str.substr(0, ms_pos);
						value_str.erase(0, value_str.find_first_not_of(" \t"));
						value_str.erase(value_str.find_last_not_of(" \t") + 1);
						perf.mean = std::stoll(value_str);
					}
				}
			}
			// Parse "|- Median: 106ms"
			else if (line.find("Median:") != std::string::npos) {
				size_t pos = line.find(":");
				if (pos != std::string::npos) {
					std::string value_str = line.substr(pos + 1);
					size_t ms_pos = value_str.find("ms");
					if (ms_pos != std::string::npos) {
						value_str = value_str.substr(0, ms_pos);
						value_str.erase(0, value_str.find_first_not_of(" \t"));
						value_str.erase(value_str.find_last_not_of(" \t") + 1);
						perf.median = std::stoll(value_str);
					}
				}
			}
			// Parse "|- Percentiles (95, 99, max): 243ms, 278ms, 314ms"
			else if (line.find("Percentiles") != std::string::npos) {
				size_t colon_pos = line.find(":");
				if (colon_pos != std::string::npos) {
					std::string values_str = line.substr(colon_pos + 1);
					// Split by comma
					std::istringstream values_stream(values_str);
					std::string p95_str, p99_str, max_str;
					
					if (std::getline(values_stream, p95_str, ',') &&
						std::getline(values_stream, p99_str, ',') &&
						std::getline(values_stream, max_str, ',')) {
						
						// Extract numbers
						size_t ms_pos = p95_str.find("ms");
						if (ms_pos != std::string::npos) {
							p95_str = p95_str.substr(0, ms_pos);
							p95_str.erase(0, p95_str.find_first_not_of(" \t"));
							p95_str.erase(p95_str.find_last_not_of(" \t") + 1);
							perf.p95 = std::stoll(p95_str);
						}
						
						ms_pos = p99_str.find("ms");
						if (ms_pos != std::string::npos) {
							p99_str = p99_str.substr(0, ms_pos);
							p99_str.erase(0, p99_str.find_first_not_of(" \t"));
							p99_str.erase(p99_str.find_last_not_of(" \t") + 1);
							perf.p99 = std::stoll(p99_str);
						}
						
						ms_pos = max_str.find("ms");
						if (ms_pos != std::string::npos) {
							max_str = max_str.substr(0, ms_pos);
							max_str.erase(0, max_str.find_first_not_of(" \t"));
							max_str.erase(max_str.find_last_not_of(" \t") + 1);
							perf.max = std::stoll(max_str);
						}
					}
				}
			}
		}
		
		// Mark as available if we parsed at least one value
		perf.available = (perf.update_time_diff > 0 || perf.mean > 0 || perf.median > 0);
		
		if (perf.available) {
			Logger::debug("fetch_server_performance: diff=" + std::to_string(perf.update_time_diff) + 
						  "ms, mean=" + std::to_string(perf.mean) + 
						  "ms, median=" + std::to_string(perf.median) + 
						  "ms, p95=" + std::to_string(perf.p95) + 
						  "ms, p99=" + std::to_string(perf.p99) + 
						  "ms, max=" + std::to_string(perf.max) + "ms");
			
			// Cache this good data (mark as fresh)
			perf.is_cached = false;
			last_known_perf = perf;
			last_perf_update_time = now_ms;
		} else {
			// Failed to parse - use cached data if available
			if (last_known_perf.available) {
				Logger::debug("fetch_server_performance: Parse failed, using last known good data");
				perf = last_known_perf;
				perf.available = true;  // Mark as available since we have cached data
				perf.is_cached = true;  // Mark as cached
			}
		}
		
		return perf;
	}

	std::vector<Continent> Query::fetch_continents() {
		std::vector<Continent> continents;
		
		std::string query_str = 
			"SELECT continent, SUM(count) as total_count FROM ( "
			"  SELECT "
			"    CASE map "
			"      WHEN 0 THEN 'Eastern Kingdoms' "
			"      WHEN 1 THEN 'Kalimdor' "
			"      WHEN 530 THEN 'Outland' "
			"      WHEN 571 THEN 'Northrend' "
			"      WHEN 609 THEN 'Eastern Kingdoms' "
			"      WHEN 30 THEN 'Battlegrounds' "
			"      WHEN 489 THEN 'Battlegrounds' "
			"      WHEN 529 THEN 'Battlegrounds' "
			"      ELSE 'Instances' "
			"    END as continent, "
			"    COUNT(*) as count "
			"  FROM characters "
			"  WHERE online = 1 "
			"    AND " + get_excluded_accounts_filter() + " "
			"  GROUP BY map "
			") as map_counts "
			"GROUP BY continent "
			"ORDER BY total_count DESC;";
		
		std::string result = mysql_exec(query_str);
		if (result.empty()) return continents;
		
		std::istringstream stream(result);
		std::string line;
		int total = 0;
		
		while (std::getline(stream, line)) {
			size_t tab = line.find('\t');
			if (tab == std::string::npos) continue;
			
			Continent c;
			c.name = line.substr(0, tab);
			c.count = std::stoi(line.substr(tab + 1));
			total += c.count;
			continents.push_back(c);
		}
		
		// Calculate percentages
		for (auto& c : continents) {
			if (total > 0) {
				c.percent = (c.count * 100.0) / total;
			}
		}
		
		return continents;
	}
	
	std::vector<Faction> Query::fetch_factions() {
		std::vector<Faction> factions;
		
		std::string query_str = 
			"SELECT "
			"  CASE "
			"    WHEN c.race IN (1,3,4,7,11) THEN 'Alliance' "
			"    WHEN c.race IN (2,5,6,8,10) THEN 'Horde' "
			"    ELSE 'Neutral' "
			"  END as faction, "
			"  COUNT(*) as count "
			"FROM characters c "
			"WHERE c.online = 1 "
			"  AND " + get_excluded_accounts_filter() + " "
			"GROUP BY faction "
			"ORDER BY count DESC;";
		
		std::string result = mysql_exec(query_str);
		if (result.empty()) return factions;
		
		std::istringstream stream(result);
		std::string line;
		int total = 0;
		
		while (std::getline(stream, line)) {
			size_t tab = line.find('\t');
			if (tab == std::string::npos) continue;
			
			Faction f;
			f.name = line.substr(0, tab);
			f.count = std::stoi(line.substr(tab + 1));
			total += f.count;
			factions.push_back(f);
		}
		
		// Calculate percentages
		for (auto& f : factions) {
			if (total > 0) {
				f.percent = (f.count * 100.0) / total;
			}
		}
		
		return factions;
	}

	std::vector<Zone> Query::fetch_zones() {
		std::vector<Zone> zones;
		
		// Simplified query - just get zone IDs and stats from characters table
		// Zone names come from hardcoded ZONE_NAMES map
		std::string query_str = 
			"SELECT "
			"  zone as zone_id, "
			"  COUNT(*) as total, "
			"  MIN(level) as min_level, "
			"  MAX(level) as max_level "
			"FROM characters "
			"WHERE online = 1 "
			"  AND " + get_excluded_accounts_filter() + " "
			"GROUP BY zone "
			"HAVING total >= 1 "
			"ORDER BY total DESC;";
		
		std::string result = mysql_exec(query_str);
		
		if (result.empty()) {
			Logger::debug("fetch_zones: mysql_exec returned empty result");
			return zones;
		}
		
		Logger::debug("fetch_zones: Got result with " + std::to_string(result.size()) + " bytes");
		
		std::istringstream stream(result);
		std::string line;
		int line_count = 0;
		
		while (std::getline(stream, line)) {
			line_count++;
			std::istringstream line_stream(line);
			std::string zone_id_str, total_str, min_str, max_str;
			
			if (std::getline(line_stream, zone_id_str, '\t') &&
				std::getline(line_stream, total_str, '\t') &&
				std::getline(line_stream, min_str, '\t') &&
				std::getline(line_stream, max_str)) {
				
				Zone z;
				int zone_id = std::stoi(zone_id_str);
				z.zone_id = zone_id;  // Store zone ID for detail queries
				z.name = AzerothCore::get_zone_name(zone_id);  // Use hardcoded zone name map
				
				// Get continent and region metadata
				auto metadata = AzerothCore::get_zone_metadata(zone_id);
				z.continent = metadata.continent;
				z.region = metadata.region;
				
				z.total = std::stoi(total_str);
				
			// Store expected levels from metadata
			z.expected_min = metadata.min_level;
			z.expected_max = metadata.max_level;
			
			// Store actual bot levels from database
			z.actual_min = std::stoi(min_str);
			z.actual_max = std::stoi(max_str);
			
			// Calculate alignment heuristic based on actual min/max vs expected range
			// This is a fast approximation without needing per-zone queries
			z.alignment = 0.0;
			if (z.expected_min > 0 && z.expected_max > 0) {
				// If actual range is within expected range, assume perfect alignment
				if (z.actual_min >= z.expected_min && z.actual_max <= z.expected_max) {
					z.alignment = 100.0;
				} 
				// If ranges overlap, calculate based on overlap amount
				else {
					int overlap_min = std::max(z.actual_min, z.expected_min);
					int overlap_max = std::min(z.actual_max, z.expected_max);
					
					if (overlap_max >= overlap_min) {
						// There is some overlap
						int overlap_range = overlap_max - overlap_min + 1;
						int actual_range = z.actual_max - z.actual_min + 1;
						
						// Estimate: assume uniform distribution across actual range
						// Alignment = (overlap_range / actual_range) * 100
						z.alignment = (overlap_range * 100.0) / actual_range;
						
						// Cap at 100%
						if (z.alignment > 100.0) z.alignment = 100.0;
					} else {
						// No overlap at all
						z.alignment = 0.0;
					}
				}
			} else {
				// Unknown zones or zones without metadata
				z.alignment = 0.0;
			}
			
			zones.push_back(z);
				Logger::debug("fetch_zones: Parsed zone '" + z.name + "' (ID: " + zone_id_str + ") with " + std::to_string(z.total) + " bots");
			} else {
				Logger::debug("fetch_zones: Failed to parse line " + std::to_string(line_count) + ": '" + line + "'");
			}
		}
		
	Logger::debug("fetch_zones: Successfully parsed " + std::to_string(zones.size()) + " zones from " + std::to_string(line_count) + " lines");
	
	// Sort zones hierarchically: by continent, then region, then total descending
	std::sort(zones.begin(), zones.end(), [](const Zone& a, const Zone& b) {
		bool a_unknown = a.continent == "Unknown";
		bool b_unknown = b.continent == "Unknown";
		
		// Unknown zones go to the bottom
		if (a_unknown != b_unknown) {
			return !a_unknown;
		}
		
		// Sort by continent
		if (a.continent != b.continent) {
			// Order: Eastern Kingdoms, Kalimdor, Outland, Northrend, Unknown
			const std::vector<std::string> continent_order = {"Eastern Kingdoms", "Kalimdor", "Outland", "Northrend"};
			auto a_it = std::find(continent_order.begin(), continent_order.end(), a.continent);
			auto b_it = std::find(continent_order.begin(), continent_order.end(), b.continent);
			
			if (a_it != continent_order.end() && b_it != continent_order.end()) {
				return a_it < b_it;
			}
			return a.continent < b.continent;
		}
		
		// Same continent: sort by region
		if (a.region != b.region) {
			return a.region < b.region;
		}
		
		// Same continent and region: sort by total descending
		return a.total > b.total;
	});
	
	return zones;
	}

	std::vector<LevelBracket> Query::fetch_levels() {
		std::vector<LevelBracket> levels;
		
		// Build dynamic SQL CASE statement based on loaded bracket definitions
		std::string case_statement = "  CASE ";
		for (const auto& bracket : expected_values.bracket_definitions) {
			case_statement += "\n    WHEN level BETWEEN " + std::to_string(bracket.min_level) + 
			                  " AND " + std::to_string(bracket.max_level) + 
			                  " THEN '" + bracket.range + "' ";
		}
		case_statement += "\n    ELSE 'Other' ";
		case_statement += "\n  END as bracket";
		
		std::string query_str = 
			"SELECT " + case_statement + ", "
			"  COUNT(*) as count "
			"FROM characters "
			"WHERE online = 1 "
			"  AND " + get_excluded_accounts_filter() + " "
			"GROUP BY bracket "
			"ORDER BY MIN(level);";
		
		std::string result = mysql_exec(query_str);
		if (result.empty()) return levels;
		
		std::istringstream stream(result);
		std::string line;
		int total = 0;
		
		while (std::getline(stream, line)) {
			size_t tab = line.find('\t');
			if (tab == std::string::npos) continue;
			
			LevelBracket lb;
			lb.range = line.substr(0, tab);
			lb.count = std::stoi(line.substr(tab + 1));
			total += lb.count;
			levels.push_back(lb);
		}
		
		// Calculate percentages
		for (auto& lb : levels) {
			if (total > 0) {
				lb.percent = (lb.count * 100.0) / total;
			}
		}
		
		return levels;
	}
	
	std::vector<ZoneDetail> Query::fetch_zone_details(int zone_id) {
	std::vector<ZoneDetail> details;
	
	// Write debug info to file for easy checking
	std::ofstream debug_file("/tmp/bottop_zone_debug.txt", std::ios::app);
	debug_file << "\n=== fetch_zone_details called ===" << std::endl;
	debug_file << "zone_id: " << zone_id << std::endl;
	
	Logger::debug("fetch_zone_details: Fetching level breakdown for zone_id=" + std::to_string(zone_id));
	
	// Query level distribution with faction splits
	std::string query_str = 
		"SELECT "
		"  CASE "
		"    WHEN c.level BETWEEN 1 AND 9 THEN '1-9' "
		"    WHEN c.level BETWEEN 10 AND 19 THEN '10-19' "
		"    WHEN c.level BETWEEN 20 AND 29 THEN '20-29' "
		"    WHEN c.level BETWEEN 30 AND 39 THEN '30-39' "
		"    WHEN c.level BETWEEN 40 AND 49 THEN '40-49' "
		"    WHEN c.level BETWEEN 50 AND 59 THEN '50-59' "
		"    WHEN c.level BETWEEN 60 AND 69 THEN '60-69' "
		"    WHEN c.level BETWEEN 70 AND 79 THEN '70-79' "
		"    WHEN c.level = 80 THEN '80' "
		"    ELSE 'Other' "
		"  END as bracket, "
		"  COUNT(*) as total, "
		"  SUM(CASE WHEN c.race IN (1,3,4,7,11) THEN 1 ELSE 0 END) as alliance_count, "
		"  SUM(CASE WHEN c.race IN (2,5,6,8,10) THEN 1 ELSE 0 END) as horde_count "
		"FROM characters c "
		"WHERE c.online = 1 AND c.zone = " + std::to_string(zone_id) + " "
		"  AND " + get_excluded_accounts_filter() + " "
		"GROUP BY bracket "
		"ORDER BY MIN(c.level);";
	
	debug_file << "Query: " << query_str << std::endl;
	
	std::string result = mysql_exec(query_str);
	
	debug_file << "Result length: " << result.size() << " bytes" << std::endl;
	debug_file << "Result content: [" << result << "]" << std::endl;
	
	if (result.empty()) {
		Logger::debug("fetch_zone_details: Empty result for zone " + std::to_string(zone_id));
		debug_file << "Empty result - returning empty details" << std::endl;
		debug_file.close();
		return details;
	}
	
	std::istringstream stream(result);
	std::string line;
	int line_num = 0;
	
	while (std::getline(stream, line)) {
		line_num++;
		debug_file << "Line " << line_num << ": [" << line << "]" << std::endl;
		
		std::istringstream line_stream(line);
		std::string bracket, total_str, alliance_str, horde_str;
		
		if (!std::getline(line_stream, bracket, '\t')) {
			debug_file << "  Failed to read bracket" << std::endl;
			continue;
		}
		if (!std::getline(line_stream, total_str, '\t')) {
			debug_file << "  Failed to read total (bracket=" << bracket << ")" << std::endl;
			continue;
		}
		if (!std::getline(line_stream, alliance_str, '\t')) {
			debug_file << "  Failed to read alliance (bracket=" << bracket << ", total=" << total_str << ")" << std::endl;
			continue;
		}
		if (!std::getline(line_stream, horde_str, '\t')) {
			debug_file << "  Failed to read horde (bracket=" << bracket << ", total=" << total_str << ", alliance=" << alliance_str << ")" << std::endl;
			continue;
		}
		
		try {
			ZoneDetail d;
			int total = std::stoi(total_str);
			int alliance = std::stoi(alliance_str);
			int horde = std::stoi(horde_str);
			
			// Format: "Lvl 1-9: 45 bots (12A/33H)"
			d.label = "  Lvl " + bracket + ": " + std::to_string(total) + " bots (" +
			          std::to_string(alliance) + "A/" + std::to_string(horde) + "H)";
			d.count = total;
			d.percent = 0.0;
			
			debug_file << "  Parsed: " << d.label << std::endl;
			details.push_back(d);
		} catch (const std::exception& e) {
			debug_file << "  Parse exception: " << e.what() << std::endl;
			Logger::error("Failed to parse zone detail line: " + std::string(e.what()));
			continue;
		}
	}
	
	debug_file << "Total details parsed: " << details.size() << std::endl;
	debug_file.close();
	
	Logger::debug("fetch_zone_details: Returning " + std::to_string(details.size()) + " level brackets for zone " + std::to_string(zone_id));
	return details;
	}
	
	OllamaStats Query::fetch_ollama_stats() {
		OllamaStats ollama;
		
		// Search for all mod_ollama_* tables
		std::string search_query = 
			"SELECT table_name FROM information_schema.tables "
			"WHERE table_schema = '" + config_.db_name + "' "
			"AND table_name LIKE 'mod_ollama%' "
			"ORDER BY table_name;";
		
		std::string search_result = mysql_exec(search_query);
		Logger::debug("fetch_ollama_stats: Found mod_ollama tables: '" + search_result + "'");
		
		// Priority order: look for log/history tables first, then stats, then personality
		std::vector<std::string> table_priority = {
			"mod_ollama_chat_log",
			"mod_ollama_chat_history", 
			"mod_ollama_log",
			"mod_ollama_history",
			"mod_ollama_stats",
			"mod_ollama_chat_stats",
			"mod_ollama_chat_personality"  // Last resort - just shows characters with AI enabled
		};
		
		std::string found_table;
		std::istringstream stream(search_result);
		std::string line;
		
		// First pass: check priority tables
		while (std::getline(stream, line)) {
			if (!line.empty() && line != "table_name") {
				for (const auto& priority_table : table_priority) {
					if (line == priority_table) {
						found_table = line;
						Logger::debug("fetch_ollama_stats: Using priority table: '" + line + "'");
						break;
					}
				}
				if (!found_table.empty()) break;
			}
		}
		
		// Second pass: if no priority match, use first found table
		if (found_table.empty()) {
			stream.clear();
			stream.seekg(0);
			while (std::getline(stream, line)) {
				if (!line.empty() && line != "table_name") {
					found_table = line;
					Logger::debug("fetch_ollama_stats: Using first available table: '" + line + "'");
					break;
				}
			}
		}
		
		if (found_table.empty()) {
			Logger::debug("fetch_ollama_stats: No mod_ollama tables found, disabled");
			ollama.enabled = false;
			return ollama;
		}
		
		ollama.enabled = true;
		
		// Query stats based on table type
		std::string stats_query;
		
		if (found_table == "mod_ollama_chat_personality") {
		// Personality table - count unique characters with Ollama enabled
		stats_query = "SELECT COUNT(DISTINCT guid) FROM " + found_table + ";";
		Logger::debug("fetch_ollama_stats: Counting characters with Ollama personalities");
		
		std::string result = mysql_exec(stats_query);
		Logger::debug("fetch_ollama_stats: Query result: '" + result + "'");
		
		if (!result.empty()) {
			try {
				int total_count = std::stoi(result);
				// For personality table, just show character count as "rate"
				ollama.messages_per_hour = total_count;
				ollama.recent_messages = 0;  // No recent activity data
				ollama.failure_rate_60s = 0.0;
				Logger::debug("fetch_ollama_stats: " + std::to_string(total_count) + " characters with Ollama");
			} catch (const std::exception& e) {
				Logger::debug("fetch_ollama_stats: Error parsing count: " + std::string(e.what()));
			}
		}
		} else {
			// Log/history/stats table - query for three specific metrics:
			// 1. Rate: messages per hour (last 60 minutes)
			// 2. Recent: messages in last 60 seconds
			// 3. Failure %: failure rate for last 60 seconds
			
			// Check if table has a timestamp column
			std::string column_query = 
				"SELECT COLUMN_NAME FROM information_schema.COLUMNS "
				"WHERE TABLE_SCHEMA = '" + config_.db_name + "' "
				"AND TABLE_NAME = '" + found_table + "' "
				"AND (COLUMN_NAME LIKE '%time%' OR COLUMN_NAME LIKE '%date%') "
				"LIMIT 1;";
			
		std::string timestamp_col = mysql_exec(column_query);
		Logger::debug("fetch_ollama_stats: Timestamp column search result: '" + timestamp_col + "'");
		
		if (!timestamp_col.empty()) {
			// Remove newlines/whitespace
			timestamp_col.erase(timestamp_col.find_last_not_of(" \n\r\t") + 1);
			Logger::debug("fetch_ollama_stats: Using timestamp column: '" + timestamp_col + "'");
			
			// Query 1: Messages in last 60 minutes (for hourly rate)
			std::string hour_query = "SELECT COUNT(*) FROM " + found_table + 
				" WHERE " + timestamp_col + " >= NOW() - INTERVAL 60 MINUTE;";
			std::string hour_result = mysql_exec(hour_query);
			Logger::debug("fetch_ollama_stats: Hour query result: '" + hour_result + "'");
			
			// Query 2: Messages in last 60 seconds (recent activity)
			std::string recent_query = "SELECT COUNT(*) FROM " + found_table + 
				" WHERE " + timestamp_col + " >= NOW() - INTERVAL 60 SECOND;";
			std::string recent_result = mysql_exec(recent_query);
			Logger::debug("fetch_ollama_stats: Recent (60s) query result: '" + recent_result + "'");
				
				// Query 3: Check if table has a success/status column for failure rate
				std::string status_col_query = 
					"SELECT COLUMN_NAME FROM information_schema.COLUMNS "
					"WHERE TABLE_SCHEMA = '" + config_.db_name + "' "
					"AND TABLE_NAME = '" + found_table + "' "
					"AND (COLUMN_NAME LIKE '%success%' OR COLUMN_NAME LIKE '%status%' OR COLUMN_NAME LIKE '%error%' OR COLUMN_NAME LIKE '%fail%') "
					"LIMIT 1;";
				std::string status_col = mysql_exec(status_col_query);
				
				double failure_rate = 0.0;
				
				if (!status_col.empty()) {
					// Has status column - query actual failure rate for last 60 seconds
					status_col.erase(status_col.find_last_not_of(" \n\r\t") + 1);
					
					// Try to count failures (assuming status=0 or success=false or error=1)
					std::string failure_query = "SELECT COUNT(*) FROM " + found_table + 
						" WHERE " + timestamp_col + " >= NOW() - INTERVAL 60 SECOND "
						"AND (" + status_col + " = 0 OR " + status_col + " = false OR " + status_col + " IS NULL);";
					std::string failure_result = mysql_exec(failure_query);
					
					if (!failure_result.empty() && !recent_result.empty()) {
						try {
							int failures = std::stoi(failure_result);
							int recent_total = std::stoi(recent_result);
							if (recent_total > 0) {
								failure_rate = (failures * 100.0) / recent_total;
							}
						} catch (...) {
							// Assume 5% failure rate if we can't parse
							failure_rate = 5.0;
						}
					}
				} else {
					// No status column - assume 5% failure rate
					failure_rate = 5.0;
				}
				
				if (!hour_result.empty() && !recent_result.empty()) {
					try {
						int hour_count = std::stoi(hour_result);
						int recent_count = std::stoi(recent_result);
						
						ollama.messages_per_hour = hour_count;
						ollama.recent_messages = recent_count;
						ollama.failure_rate_60s = failure_rate;
						
						Logger::debug("fetch_ollama_stats: rate=" + std::to_string(hour_count) + 
							" msgs/hr, recent=" + std::to_string(recent_count) + 
							" msgs (60s), failure=" + std::to_string(failure_rate) + "%");
					} catch (const std::exception& e) {
						Logger::debug("fetch_ollama_stats: Error parsing counts: " + std::string(e.what()));
					}
				}
			} else {
				// No timestamp column - can't provide time-based metrics
				Logger::debug("fetch_ollama_stats: No timestamp column found");
				ollama.messages_per_hour = 0;
				ollama.recent_messages = 0;
				ollama.failure_rate_60s = 0.0;
			}
		}
		
		Logger::debug("fetch_ollama_stats: Returning enabled=" + std::to_string(ollama.enabled) + 
			" rate=" + std::to_string(ollama.messages_per_hour) + 
			" recent=" + std::to_string(ollama.recent_messages));
		return ollama;
	}

	std::pair<bool, double> Query::check_rebuild_status() {
		// Check if server is in rebuilding state and return progress
		// This checks for a rebuild marker file or log that indicates rebuild status
		// Returns: (is_rebuilding, progress_percentage)
		
		try {
			// Check for rebuild marker file existence
			// Format: /tmp/azerothcore_rebuild_progress.txt
			// Content: percentage as integer (0-100)
			std::string check_cmd = "docker exec " + config_.container + 
			                       " cat /tmp/azerothcore_rebuild_progress.txt 2>/dev/null || echo 'none'";
			
			std::string result = executor_.execute(check_cmd);
			
			// Trim whitespace
			result.erase(0, result.find_first_not_of(" \t\n\r"));
			result.erase(result.find_last_not_of(" \t\n\r") + 1);
			
			if (result == "none" || result.empty()) {
				// No rebuild in progress
				return {false, 0.0};
			}
			
			// Try to parse progress percentage
			try {
				double progress = std::stod(result);
				// Clamp to 0-100 range
				progress = std::max(0.0, std::min(100.0, progress));
				return {true, progress};
			} catch (...) {
				// Invalid format, assume not rebuilding
				return {false, 0.0};
			}
			
		} catch (const std::exception& e) {
			Logger::debug("check_rebuild_status error: " + std::string(e.what()));
			return {false, 0.0};
		}
	}

	std::vector<ContainerStatus> Query::fetch_container_statuses() {
		// Fetch status of all AzerothCore-related containers
		// Returns list of containers with their current state
		
		std::vector<ContainerStatus> containers;
		
		try {
			// Use docker ps -a with format to get all AzerothCore containers
			// Format: name|state|status
			// Example output:
			//   testing-ac-worldserver|running|Up 2 hours
			//   testing-ac-authserver|running|Up 2 hours
			//   testing-ac-database|running|Up 2 hours
			
			std::string cmd = "docker ps -a --filter 'name=ac-' --format '{{.Names}}|{{.State}}|{{.Status}}'";
			std::string result = executor_.execute(cmd);
			
			if (result.empty()) {
				Logger::debug("fetch_container_statuses: No containers found");
				return containers;
			}
			
			// Parse result - one container per line
			std::istringstream stream(result);
			std::string line;
			
			while (std::getline(stream, line)) {
				// Trim whitespace
				line.erase(0, line.find_first_not_of(" \t\n\r"));
				line.erase(line.find_last_not_of(" \t\n\r") + 1);
				
				if (line.empty()) continue;
				
				// Parse: name|state|status
				ContainerStatus container;
				
				size_t pos1 = line.find('|');
				if (pos1 == std::string::npos) continue;
				
				size_t pos2 = line.find('|', pos1 + 1);
				if (pos2 == std::string::npos) continue;
				
				container.name = line.substr(0, pos1);
				container.state = line.substr(pos1 + 1, pos2 - pos1 - 1);
				container.status = line.substr(pos2 + 1);
				container.is_running = (container.state == "running");
				
				// Extract short name (e.g., "testing-ac-worldserver" -> "worldserver")
				// Look for "ac-" and take everything after it
				size_t ac_pos = container.name.find("ac-");
				if (ac_pos != std::string::npos) {
					container.short_name = container.name.substr(ac_pos + 3);
				} else {
					// Fallback: use full name
					container.short_name = container.name;
				}
				
				// Show ALL containers during troubleshooting (OFFLINE/RESTARTING/REBUILDING)
				// This includes service containers AND any init/helper containers that may affect restart/rebuild
				containers.push_back(container);
			}
			
			Logger::debug("fetch_container_statuses: Found " + std::to_string(containers.size()) + " containers");
			
		} catch (const std::exception& e) {
			Logger::debug("fetch_container_statuses error: " + std::string(e.what()));
		}
		
		return containers;
	}

	ServerData Query::fetch_all() {
		ServerData data;
		
		Logger::error("FETCH_ALL DEBUG: Starting fetch_all()");
		
		// Set server URL from config
		data.server_url = config.ssh_host;
		
		// Get current timestamp
		auto now = std::time(nullptr);
		std::ostringstream ts;
		ts << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
		data.timestamp = ts.str();
		
		try {
			Logger::error("FETCH_ALL DEBUG: About to call fetch_bot_stats()");
			data.stats = fetch_bot_stats();
			Logger::error("FETCH_ALL DEBUG: fetch_bot_stats() returned, total=" + std::to_string(data.stats.total));
			
			Logger::error("FETCH_ALL DEBUG: About to call fetch_continents()");
			data.continents = fetch_continents();
			Logger::error("FETCH_ALL DEBUG: fetch_continents() returned");
			
			Logger::error("FETCH_ALL DEBUG: About to call fetch_factions()");
			data.factions = fetch_factions();
			Logger::error("FETCH_ALL DEBUG: fetch_factions() returned");
			
			Logger::error("FETCH_ALL DEBUG: About to call fetch_zones()");
			data.zones = fetch_zones();
			Logger::error("FETCH_ALL DEBUG: fetch_zones() returned, count=" + std::to_string(data.zones.size()));
			
			Logger::error("FETCH_ALL DEBUG: About to call fetch_levels()");
			data.levels = fetch_levels();
			Logger::error("FETCH_ALL DEBUG: fetch_levels() returned");
			
			Logger::error("FETCH_ALL DEBUG: About to call fetch_ollama_stats()");
			data.ollama = fetch_ollama_stats();
			Logger::error("FETCH_ALL DEBUG: fetch_ollama_stats() returned");
		} catch (const std::exception& e) {
			Logger::error("FETCH_ALL DEBUG: Exception caught: " + std::string(e.what()));
			data.error = e.what();
		}
		
		Logger::error("FETCH_ALL DEBUG: Returning from fetch_all()");
		return data;
	}

	//* Module functions
	void init() {
		// File-based debug logging since Logger might not be ready
		std::ofstream debug_log("/tmp/bottop_init_debug.log", std::ios::app);
		debug_log << "\n=== INIT CALLED ===" << std::endl;
		debug_log << "enabled=" << enabled << std::endl;
		
		std::cerr << "[BOTTOP DEBUG] init() called, enabled=" << enabled << std::endl;
		if (!enabled) {
			debug_log << "Returning early: not enabled" << std::endl;
			std::cerr << "[BOTTOP DEBUG] init() returning early, not enabled" << std::endl;
			return;
		}
		
		try {
			// Auto-detect if we should use local Docker
			if (config.ssh_host.empty() || config.ssh_host == "localhost" || config.ssh_host == "127.0.0.1") {
				// Force local mode if ssh_host is empty or localhost
				config.use_local = true;
				debug_log << "SSH host is empty/localhost, forcing local mode" << std::endl;
			} else if (!config.use_local) {
				// Check if Docker is available locally with AzerothCore containers
				FILE* pipe = popen("docker ps --filter 'name=ac-' --format '{{.Names}}' 2>/dev/null | head -1", "r");
				if (pipe) {
					char buffer[256];
					std::string result;
					if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
						result = buffer;
						// Trim whitespace
						result.erase(0, result.find_first_not_of(" \t\r\n"));
						result.erase(result.find_last_not_of(" \t\r\n") + 1);
					}
					pclose(pipe);
					
					if (!result.empty()) {
						config.use_local = true;
						debug_log << "Found local AzerothCore containers, enabling local mode" << std::endl;
						std::cerr << "[BOTTOP DEBUG] Auto-detected local AzerothCore containers: " << result << std::endl;
						Logger::error("Auto-detected local AzerothCore container, using local Docker mode");
					}
				}
			}
			
			// Create appropriate executor
			if (config.use_local) {
				debug_log << "Using local Docker mode" << std::endl;
				std::cerr << "[BOTTOP DEBUG] Using local Docker executor" << std::endl;
				Logger::error("Initializing local Docker connection");
				executor = std::make_unique<LocalExecutor>();
				current_data.server_url = "localhost (Docker)";
			} else {
				debug_log << "Using SSH mode to " << config.ssh_host << std::endl;
				std::cerr << "[BOTTOP DEBUG] Using SSH executor to " << config.ssh_host << std::endl;
				Logger::error("Initializing SSH connection to " + config.ssh_host);
				
				auto ssh = std::make_unique<SSHClient>(config.ssh_host);
				if (!ssh->connect()) {
					debug_log << "SSH connection failed: " << ssh->last_error() << std::endl;
					std::cerr << "[BOTTOP DEBUG] SSH connection failed: " << ssh->last_error() << std::endl;
					current_data.error = "Failed to connect: " + ssh->last_error();
					return;
				}
				executor = std::move(ssh);
				current_data.server_url = config.ssh_host;
			}
			
			debug_log << "Executor created successfully, creating Query object" << std::endl;
			std::cerr << "[BOTTOP DEBUG] Executor ready, creating Query object" << std::endl;
			query = std::make_unique<Query>(*executor, config);
			active = true;
			
			debug_log << "About to call load_expected_values()" << std::endl;
			std::cerr << "[BOTTOP DEBUG] About to call load_expected_values()" << std::endl;
			// Load expected values from config file
			load_expected_values();
			debug_log << "load_expected_values() returned, brackets loaded: " << expected_values.bracket_definitions.size() << std::endl;
			std::cerr << "[BOTTOP DEBUG] load_expected_values() returned" << std::endl;
		} catch (const std::exception& e) {
			debug_log << "Exception in init(): " << e.what() << std::endl;
			std::cerr << "[BOTTOP DEBUG] Exception in init(): " << e.what() << std::endl;
			current_data.error = std::string("Init error: ") + e.what();
		}
		
		debug_log.close();
	}

	bool check_server_online() {
		if (!executor || !executor->is_connected()) return false;
		
		try {
			// Check if container is running
			std::string cmd = "docker ps --filter name=" + config.container + " --format '{{.Status}}'";
			std::string status = executor->execute(cmd);
			
			// If output contains "Up", container is running
			return status.find("Up") != std::string::npos;
		} catch (...) {
			return false;
		}
	}
	
	void reset_stats() {
		Logger::info("Resetting all stats and clearing display data");
		
		// Clear all data
		current_data = ServerData();
		current_data.server_url = config.use_local ? "localhost (Docker)" : config.ssh_host;
		current_data.status = ServerStatus::OFFLINE;
		current_data.error = "Server disconnected or restarted";
		
		// Clear historical data
		load_history.clear();
		
		// Clear cached performance data
		last_known_perf = ServerPerformance();
		last_perf_update_time = 0;
		
		Logger::info("Stats reset complete");
	}
	
	void collect() {
		if (!enabled || !active || !query) return;
		
		Logger::error("COLLECT DEBUG: collect() called at " + std::to_string(std::time(nullptr)));
		
		// Get current time for config refresh check
		auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();
		
		try {
			// First check if server is online
			bool server_online = check_server_online();
			
			if (!server_online) {
				// Detect transition from ONLINE to OFFLINE/RESTARTING
				if (previous_status == ServerStatus::ONLINE) {
					Logger::info("Server disconnected or went offline - resetting stats");
					reset_stats();
				}
				
				current_data.status = ServerStatus::OFFLINE;
				current_data.consecutive_failures++;
				
				// After 3 failures, assume restarting
				if (current_data.consecutive_failures >= 3) {
					current_data.status = ServerStatus::RESTARTING;
				}
				
				current_data.error = "Server container is not running";
				
				// Fetch container statuses to show detailed state
				current_data.containers = query->fetch_container_statuses();
				
				// Clear performance data when server is not online
				current_data.stats.perf.available = false;
				current_data.stats.perf.is_cached = false;
				
				// Update previous status
				previous_status = current_data.status;
				
				return;
			}
			
		// Server is online, check if rebuilding
		Logger::error("COLLECT DEBUG: Server online, checking rebuild status");
		auto [is_rebuilding, rebuild_progress] = query->check_rebuild_status();
		
		if (is_rebuilding) {
			// Detect transition from ONLINE to REBUILDING
			if (previous_status == ServerStatus::ONLINE) {
				Logger::info("Server started rebuilding - resetting stats");
				reset_stats();
			}
			
			current_data.status = ServerStatus::REBUILDING;
			current_data.rebuild_progress = rebuild_progress;
			current_data.error = "Server is rebuilding databases";
			
			// Fetch container statuses during rebuild
			current_data.containers = query->fetch_container_statuses();
			
			// Clear performance data during rebuild
			current_data.stats.perf.available = false;
			current_data.stats.perf.is_cached = false;
			
			// Update previous status
			previous_status = current_data.status;
			
			Logger::error("COLLECT DEBUG: Server is rebuilding, progress=" + std::to_string(rebuild_progress) + "%");
			return;
		}
		
		// Detect transition from OFFLINE/RESTARTING/REBUILDING to ONLINE
		if (previous_status != ServerStatus::ONLINE) {
			Logger::info("Server came back online after being offline/restarting/rebuilding - resetting stats");
			reset_stats();
			// Force config refresh immediately after server restart
			last_config_refresh_time = 0;
		}
		
	// Periodic config refresh (every 90 seconds when server is online)
	if (last_config_refresh_time == 0 || 
	    (now_ms - last_config_refresh_time) >= CONFIG_REFRESH_INTERVAL_MS) {
		Logger::info("Performing periodic config refresh (90s interval)");
		load_expected_values();
		last_config_refresh_time = now_ms;
		
		// Trigger bottop config reload via SIGUSR2 signal
		kill(getpid(), SIGUSR2);
	}
		
		// Server is online and not rebuilding, try to fetch data
		Logger::error("COLLECT DEBUG: Server online, calling fetch_all()");
		ServerData new_data = query->fetch_all();
		
		// Fetch container statuses for ONLINE state too
		new_data.containers = query->fetch_container_statuses();

		
		// DEBUG: Log what we got back
		Logger::error("COLLECT DEBUG: fetch_all() returned, total=" + std::to_string(new_data.stats.total) + 
		              " zones=" + std::to_string(new_data.zones.size()) + " containers=" + std::to_string(new_data.containers.size()) +
		              " error=" + new_data.error);
		
		// Success - reset failure counter and rebuild progress
		current_data = new_data;
		current_data.status = ServerStatus::ONLINE;
		current_data.rebuild_progress = 0.0;
		current_data.consecutive_failures = 0;
		
		// Update previous status
		previous_status = ServerStatus::ONLINE;
			
		// DEBUG: Verify assignment
		std::ofstream collect_log("/tmp/bottop_collect.txt", std::ios::app);
		collect_log << "After assignment: ollama.enabled=" << current_data.ollama.enabled 
		            << " rate=" << current_data.ollama.messages_per_hour 
		            << " recent=" << current_data.ollama.recent_messages << std::endl;
		collect_log.close();
			
		//* Update history for graphs (keep last 300 samples)
		// Track mean server update time (average of last 500 cycles)
		if (current_data.stats.perf.available) {
			load_history.push_back(current_data.stats.perf.mean);
		} else {
			load_history.push_back((long long)current_data.stats.update_time_avg);
		}
			if (load_history.size() > 300) {
				load_history.pop_front();
			}
			
		} catch (const std::exception& e) {
			current_data.error = std::string("Collect error: ") + e.what();
			current_data.status = ServerStatus::ERROR;
			current_data.consecutive_failures++;
			
			// Clear performance data on error
			current_data.stats.perf.available = false;
			current_data.stats.perf.is_cached = false;
		}
	}

	void load_expected_values() {
		// File-based debug logging
		std::ofstream debug_log("/tmp/bottop_init_debug.log", std::ios::app);
		debug_log << "\n=== LOAD_EXPECTED_VALUES CALLED ===" << std::endl;
		
		std::cerr << "[BOTTOP DEBUG] load_expected_values: === STARTING BRACKET CONFIG LOAD ===" << std::endl;
		Logger::error("load_expected_values: === STARTING BRACKET CONFIG LOAD ===");
		
		// Set default brackets (8 brackets for WotLK)
		expected_values.bracket_definitions = {
			{1, 9}, {10, 19}, {20, 29}, {30, 39},
			{40, 49}, {50, 59}, {60, 60}, {61, 69},
			{70, 70}, {71, 79}, {80, 80}
		};
		
		// Set default percentages
		// Set default percentages
		expected_values.level_distribution = {
			{"1-9", 0, 5.0},
			{"10-19", 0, 6.0},
			{"20-29", 0, 10.0},
			{"30-39", 0, 9.0},
			{"40-49", 0, 8.0},
			{"50-59", 0, 6.0},
			{"60", 0, 12.0},
			{"61-69", 0, 7.0},
			{"70", 0, 10.0},
			{"71-79", 0, 16.0},
			{"80", 0, 11.0}
		};
		
		// Set default continent percentages
		expected_values.continent_distribution = {
			{"Eastern Kingdoms", 0, 40.0},
			{"Kalimdor", 0, 40.0},
			{"Outland", 0, 15.0},
			{"Northrend", 0, 5.0}
		};
		
	expected_values.loaded = true;
	
	debug_log << "Set 8 default brackets" << std::endl;
	Logger::error("load_expected_values: Set 8 default brackets");
	
	// Check if SSH is available at all
	if (!executor || !executor->is_connected()) {
		debug_log << "No SSH connection available - ssh=" << (executor ? "yes" : "no") 
		          << ", connected=" << ((executor && executor->is_connected()) ? "yes" : "no") << std::endl;
		debug_log.close();
		Logger::error("load_expected_values: No SSH connection available, using defaults");
		return;
	}
	
	// Auto-discover container name if not manually configured
	if (config.container.empty()) {
		debug_log << "Container not configured, attempting auto-discovery..." << std::endl;
		Logger::error("load_expected_values: Container not configured, attempting auto-discovery");
		
		// Try multiple patterns to find the worldserver container
		std::vector<std::string> container_patterns = {
			"docker ps --filter 'name=ac-worldserver' --format '{{.Names}}' | head -1",
			"docker ps --filter 'name=worldserver' --format '{{.Names}}' | grep -i ac | head -1",
			"docker ps --format '{{.Names}}' | grep -i worldserver | head -1",
			"docker ps --format '{{.Names}}' | grep 'ac-' | head -1"
		};
		
		for (const auto& cmd : container_patterns) {
			try {
				debug_log << "Trying pattern: " << cmd << std::endl;
				std::string result = executor->execute(cmd);
				
				// Trim whitespace and newlines
				result.erase(0, result.find_first_not_of(" \t\r\n"));
				result.erase(result.find_last_not_of(" \t\r\n") + 1);
				
				if (!result.empty() && result.find("Error") == std::string::npos) {
					config.container = result;
					debug_log << "Auto-discovered container: " << config.container << std::endl;
					Logger::error("load_expected_values: Auto-discovered container: " + config.container);
					break;
				}
			} catch (const std::exception& e) {
				debug_log << "Pattern failed: " << e.what() << std::endl;
				continue;
			}
		}
		
		if (config.container.empty()) {
			debug_log << "Failed to auto-discover container, using defaults" << std::endl;
			debug_log.close();
			Logger::error("load_expected_values: Failed to auto-discover container, using defaults");
			return;
		}
	} else {
		debug_log << "Using manually configured container: " << config.container << std::endl;
		Logger::error("load_expected_values: Using manually configured container: " + config.container);
	}
	
	// Auto-discover config path if not manually configured
	if (config.config_path.empty()) {
		debug_log << "Config path not configured, attempting auto-discovery..." << std::endl;
		Logger::error("load_expected_values: Config path not configured, attempting auto-discovery");
		
		// Try common paths in order of likelihood
		std::vector<std::string> config_paths = {
			"/azerothcore/env/dist/etc/worldserver.conf",
			"/etc/worldserver.conf",
			"/opt/azerothcore/etc/worldserver.conf",
			"/azerothcore/etc/worldserver.conf"
		};
		
		for (const auto& path : config_paths) {
			try {
				std::string test_cmd = "docker exec " + config.container + " test -f " + path + " && echo found";
				debug_log << "Testing path: " << path << std::endl;
				std::string result = executor->execute(test_cmd);
				
				if (result.find("found") != std::string::npos) {
					config.config_path = path;
					debug_log << "Auto-discovered config path: " << config.config_path << std::endl;
					Logger::error("load_expected_values: Auto-discovered config path: " + config.config_path);
					break;
				}
			} catch (const std::exception& e) {
				debug_log << "Path test failed: " << e.what() << std::endl;
				continue;
			}
		}
		
		if (config.config_path.empty()) {
			debug_log << "Failed to auto-discover config path, using defaults" << std::endl;
			debug_log.close();
			Logger::error("load_expected_values: Failed to auto-discover config path, using defaults");
			return;
		}
	} else {
		debug_log << "Using manually configured config path: " << config.config_path << std::endl;
		Logger::error("load_expected_values: Using manually configured config path: " + config.config_path);
	}
		
		debug_log << "Starting bracket config load from container: " << config.container << std::endl;
		Logger::error("load_expected_values: Starting bracket config load from container: " + config.container);
		
		try {
			debug_log << "About to read main config file from: " << config.config_path << std::endl;
			Logger::error("load_expected_values: About to read main config file");
			
			// Read main config file from container
			std::string cmd = "docker exec " + config.container + " cat " + config.config_path;
			debug_log << "Executing command: " << cmd << std::endl;
			std::string config_content = executor->execute(cmd);
			
			debug_log << "Got config content, length=" << config_content.length() << std::endl;
			Logger::error("load_expected_values: Got config content, length=" + std::to_string(config_content.length()));
			
			if (config_content.empty()) {
				debug_log << "Config file empty, using defaults" << std::endl;
				debug_log.close();
				Logger::error("load_expected_values: Config file empty, using defaults");
				return;
			}
			
			// Parse config file line by line
			std::istringstream stream(config_content);
			std::string line;
			
			while (std::getline(stream, line)) {
				// Skip comments and empty lines
				if (line.empty() || line[0] == '#') continue;
				
				// Look for PlayerBot configuration
				if (line.find("AiPlayerbot.MinRandomBots") != std::string::npos) {
					size_t eq_pos = line.find('=');
					if (eq_pos != std::string::npos) {
						std::string value = line.substr(eq_pos + 1);
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t\r\n") + 1);
						expected_values.bot_min = std::stoi(value);
					}
				}
				else if (line.find("AiPlayerbot.MaxRandomBots") != std::string::npos) {
					size_t eq_pos = line.find('=');
					if (eq_pos != std::string::npos) {
						std::string value = line.substr(eq_pos + 1);
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t\r\n") + 1);
						expected_values.bot_max = std::stoi(value);
					}
				}
			}
			
			// Read level bracket configuration file from container
			std::string bracket_config_path = "/azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf";
			std::string bracket_cmd = "docker exec " + config.container + " cat " + bracket_config_path;
			debug_log << "About to execute bracket config command: " << bracket_cmd << std::endl;
			std::string bracket_content = executor->execute(bracket_cmd);
			
			debug_log << "Got bracket content, length=" << bracket_content.length() << std::endl;
			
			if (bracket_content.empty()) {
				debug_log << "Bracket config empty, using defaults" << std::endl;
				debug_log.close();
				Logger::error("load_expected_values: Bracket config empty, using defaults");
				return;
			}
			
			Logger::error("load_expected_values: Got bracket config content, length=" + std::to_string(bracket_content.length()));
			
			// Parse bracket definitions and percentages
			// Format: BotLevelBrackets.Alliance.RangeX.Lower/Upper/Pct
			std::map<int, int> bracket_min_levels;  // range_num -> min_level
			std::map<int, int> bracket_max_levels;  // range_num -> max_level
			std::map<int, double> bracket_percentages;  // range_num -> percentage
			
			std::istringstream bracket_stream(bracket_content);
			std::string bracket_line;
			
			while (std::getline(bracket_stream, bracket_line)) {
				// Skip comments and empty lines
				if (bracket_line.empty() || bracket_line[0] == '#') continue;
				
				// Look for BotLevelBrackets.Alliance.RangeX.Lower
				if (bracket_line.find("BotLevelBrackets.Alliance.Range") != std::string::npos &&
				    bracket_line.find(".Lower") != std::string::npos) {
					size_t eq_pos = bracket_line.find('=');
					if (eq_pos != std::string::npos) {
						std::string value = bracket_line.substr(eq_pos + 1);
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t\r\n") + 1);
						
						size_t start = bracket_line.find("Range") + 5;
						size_t end = bracket_line.find(".Lower");
						if (start < end) {
							int range_num = std::stoi(bracket_line.substr(start, end - start));
							bracket_min_levels[range_num] = std::stoi(value);
						}
					}
				}
				// Look for BotLevelBrackets.Alliance.RangeX.Upper
				else if (bracket_line.find("BotLevelBrackets.Alliance.Range") != std::string::npos &&
				         bracket_line.find(".Upper") != std::string::npos) {
					size_t eq_pos = bracket_line.find('=');
					if (eq_pos != std::string::npos) {
						std::string value = bracket_line.substr(eq_pos + 1);
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t\r\n") + 1);
						
						size_t start = bracket_line.find("Range") + 5;
						size_t end = bracket_line.find(".Upper");
						if (start < end) {
							int range_num = std::stoi(bracket_line.substr(start, end - start));
							bracket_max_levels[range_num] = std::stoi(value);
						}
					}
				}
				// Look for BotLevelBrackets.Alliance.RangeX.Pct
				else if (bracket_line.find("BotLevelBrackets.Alliance.Range") != std::string::npos &&
				         bracket_line.find(".Pct") != std::string::npos) {
					size_t eq_pos = bracket_line.find('=');
					if (eq_pos != std::string::npos) {
						std::string value = bracket_line.substr(eq_pos + 1);
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t\r\n") + 1);
						
						size_t start = bracket_line.find("Range") + 5;
						size_t end = bracket_line.find(".Pct");
						if (start < end) {
							int range_num = std::stoi(bracket_line.substr(start, end - start));
							bracket_percentages[range_num] = std::stod(value);
						}
					}
				}
			}
			
			// Parse continent percentages
			// Format: BotContinentPct.EasternKingdoms = 40.0
			std::map<std::string, double> continent_percentages;
			
			std::istringstream continent_stream(bracket_content);
			std::string continent_line;
			
			while (std::getline(continent_stream, continent_line)) {
				// Skip comments and empty lines
				if (continent_line.empty() || continent_line[0] == '#') continue;
				
				// Look for BotContinentPct.ContinentName
				if (continent_line.find("BotContinentPct.") != std::string::npos) {
					size_t eq_pos = continent_line.find('=');
					if (eq_pos != std::string::npos) {
						std::string value = continent_line.substr(eq_pos + 1);
						value.erase(0, value.find_first_not_of(" \t"));
						value.erase(value.find_last_not_of(" \t\r\n") + 1);
						
						size_t start = continent_line.find("BotContinentPct.") + 16;
						size_t end = eq_pos;
						if (start < end) {
							std::string continent_key = continent_line.substr(start, end - start);
							continent_key.erase(0, continent_key.find_first_not_of(" \t"));
							continent_key.erase(continent_key.find_last_not_of(" \t") + 1);
							
							// Map config keys to continent display names
							std::string continent_name;
							if (continent_key == "EasternKingdoms") continent_name = "Eastern Kingdoms";
							else if (continent_key == "Kalimdor") continent_name = "Kalimdor";
							else if (continent_key == "Outland") continent_name = "Outland";
							else if (continent_key == "Northrend") continent_name = "Northrend";
							else continue;  // Unknown continent
							
							continent_percentages[continent_name] = std::stod(value);
						}
					}
				}
			}
			
			// Update continent distribution if found in config
			if (!continent_percentages.empty()) {
				expected_values.continent_distribution.clear();
				for (const auto& pair : continent_percentages) {
					expected_values.continent_distribution.push_back({
						pair.first,  // name
						0,           // count (will be filled during collection)
						pair.second  // percent
					});
				}
				Logger::error("load_expected_values: Loaded " + std::to_string(continent_percentages.size()) + " continent percentages from config");
			}
			
			// Build bracket definitions from parsed values
			if (!bracket_min_levels.empty() && !bracket_max_levels.empty()) {
				debug_log << "Building brackets from parsed data, found " << bracket_min_levels.size() << " min levels" << std::endl;
				expected_values.bracket_definitions.clear();
				expected_values.level_distribution.clear();
				
				// Find the maximum bracket number
				int max_bracket = 0;
				for (const auto& pair : bracket_min_levels) {
					max_bracket = std::max(max_bracket, pair.first);
				}
				
				debug_log << "Max bracket number: " << max_bracket << std::endl;
				
				// Build brackets in order
				for (int i = 1; i <= max_bracket; i++) {
					if (bracket_min_levels.count(i) && bracket_max_levels.count(i)) {
						int min_level = bracket_min_levels[i];
						int max_level = bracket_max_levels[i];
						double percent = bracket_percentages.count(i) ? bracket_percentages[i] : 0.0;
						
						debug_log << "  Bracket " << i << ": " << min_level << "-" << max_level << " (" << percent << "%)" << std::endl;
						
						expected_values.bracket_definitions.push_back({min_level, max_level});
						
						// Format range consistently with BracketDefinition constructor
						std::string range_str;
						if (min_level == max_level) {
							range_str = std::to_string(min_level);  // "60"
						} else {
							range_str = std::to_string(min_level) + "-" + std::to_string(max_level);  // "1-9"
						}
						
						expected_values.level_distribution.push_back({
							range_str,
							0,
							percent
						});
					}
				}
				
				debug_log << "Final bracket count: " << expected_values.bracket_definitions.size() << std::endl;
				debug_log.close();
				Logger::error("load_expected_values: Loaded " + std::to_string(expected_values.bracket_definitions.size()) + " brackets from remote config");
			} else {
				debug_log << "No brackets parsed (min_levels=" << bracket_min_levels.size() << ", max_levels=" << bracket_max_levels.size() << ")" << std::endl;
				debug_log.close();
				Logger::error("load_expected_values: No brackets parsed from config, using defaults");
			}
			
		} catch (const std::exception& e) {
			debug_log << "Exception in load_expected_values: " << e.what() << std::endl;
			debug_log.close();
			Logger::error("load_expected_values: Failed to parse remote config: " + std::string(e.what()));
			// Keep defaults
		}
		
		debug_log.close();
	}
	
	void cleanup() {
		query.reset();
		executor.reset();
		active = false;
	}

	std::string get_box() {
		// This will be implemented to return formatted box content for btop display
		// For now, return a placeholder
		std::ostringstream out;
		
		if (!enabled) {
			out << "AzerothCore Monitor: Disabled";
			return out.str();
		}
		
		if (!active) {
			out << "AzerothCore Monitor: Not connected";
			if (!current_data.error.empty()) {
				out << "\nError: " << current_data.error;
			}
			return out.str();
		}
		
		out << "AzerothCore Bot Monitor\n";
		out << "======================\n";
		out << "Total Bots: " << current_data.stats.total << "\n";
		out << "Uptime: " << std::fixed << std::setprecision(1) 
			<< current_data.stats.uptime_hours << " hours\n";
		out << "Update Time: " << std::fixed << std::setprecision(1) 
			<< current_data.stats.update_time_avg << " ms\n\n";
		
		if (!current_data.continents.empty()) {
			out << "Continents:\n";
			for (const auto& c : current_data.continents) {
				out << "  " << c.name << ": " << c.count 
					<< " (" << std::fixed << std::setprecision(1) << c.percent << "%)\n";
			}
		}
		
		if (!current_data.error.empty()) {
			out << "\nError: " << current_data.error;
		}
		
		return out.str();
	}
}
