# Disconnection Detection and Periodic Config Refresh

## Summary

Implemented two key features to improve bottop's resilience and accuracy:

1. **Automatic stats reset on disconnection/restart detection**
2. **Periodic 90-second config refresh for brackets and alignments**

## Changes Made

### 1. Disconnection/Restart Detection (`src/btop_azerothcore.cpp`)

Added tracking of server status transitions to detect when the server:

- Goes from ONLINE to OFFLINE
- Goes from ONLINE to RESTARTING
- Goes from ONLINE to REBUILDING
- Comes back ONLINE after being offline/restarting/rebuilding

**Key additions:**

- `previous_status` variable to track the last known server state
- State transition detection in `collect()` function
- Automatic call to `reset_stats()` when disconnection is detected

### 2. Stats Reset Function (`src/btop_azerothcore.hpp` + `.cpp`)

Implemented `reset_stats()` function that:

- Clears all `ServerData` (bot stats, zones, continents, factions, levels, etc.)
- Clears historical graph data (`load_history`)
- Resets cached performance metrics
- Sets status to OFFLINE with appropriate error message
- Logs the reset action

**Function signature:**

```cpp
void reset_stats();
```

### 3. Periodic Config Refresh (`src/btop_azerothcore.cpp`)

Added automatic config refresh every 90 seconds when server is online:

- Tracks time of last config refresh using `last_config_refresh_time`
- Refreshes config every 90 seconds (`CONFIG_REFRESH_INTERVAL_MS = 90000`)
- Forces immediate config refresh when server comes back online after restart
- Calls `load_expected_values()` to reload:
    - Level bracket definitions
    - Expected level distributions
    - Expected continent distributions
    - Bot min/max values

**Key additions:**

```cpp
uint64_t last_config_refresh_time = 0;
const uint64_t CONFIG_REFRESH_INTERVAL_MS = 90000;  // 90 seconds
```

## Behavior

### Disconnection Scenarios

**Scenario 1: Server container stops**

1. `check_server_online()` returns false
2. Status changes: ONLINE → OFFLINE
3. `reset_stats()` is called
4. Display clears and shows "Server container is not running"
5. Container statuses are fetched to show details

**Scenario 2: Server restarts**

1. Status changes: ONLINE → RESTARTING (after 3 consecutive failures)
2. `reset_stats()` is called
3. When server comes back online:
    - Status changes: RESTARTING → ONLINE
    - `reset_stats()` is called again
    - Config is immediately refreshed (`load_expected_values()`)

**Scenario 3: Server rebuilds databases**

1. `check_rebuild_status()` detects rebuild marker
2. Status changes: ONLINE → REBUILDING
3. `reset_stats()` is called
4. Rebuild progress is shown
5. When rebuild completes:
    - Status changes: REBUILDING → ONLINE
    - `reset_stats()` is called
    - Config is immediately refreshed

### Config Refresh

**When it happens:**

- Every 90 seconds during normal operation (server online)
- Immediately after server restart/recovery
- When bottop first starts up

**What gets refreshed:**

- `AiPlayerbot.MinRandomBots` and `AiPlayerbot.MaxRandomBots`
- Level bracket definitions from `mod_player_bot_level_brackets.conf`
- Expected level distribution percentages
- Expected continent distribution percentages
- Container name and config path (if auto-discovery is enabled)

**Why 90 seconds?**

- Frequent enough to catch config changes quickly
- Infrequent enough to avoid performance impact
- Allows server admins to tune bot distribution without restarting bottop

## Files Modified

1. `src/btop_azerothcore.hpp` - Added `reset_stats()` declaration
2. `src/btop_azerothcore.cpp` - Implemented all features:
    - Added `previous_status` tracking
    - Added `last_config_refresh_time` tracking
    - Implemented `reset_stats()` function
    - Modified `collect()` to detect transitions and refresh config

## Testing Recommendations

### Test 1: Server Stop/Start

```bash
# Stop the worldserver container
docker stop testing-ac-worldserver

# Observe bottop - should reset stats and show OFFLINE
# Start the container
docker start testing-ac-worldserver

# Observe bottop - should reset stats again and reload config
```

### Test 2: Config Changes

```bash
# Edit the level bracket config on the server
docker exec testing-ac-worldserver vi /azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf

# Wait up to 90 seconds
# Observe bottop - brackets should update automatically
```

### Test 3: Server Restart via Docker Compose

```bash
# Restart all AzerothCore containers
docker-compose restart

# Observe bottop through the restart cycle:
# ONLINE → RESTARTING → ONLINE (with stats reset at each transition)
```

## Log Messages

Key log messages to watch for:

- `"Server disconnected or went offline - resetting stats"`
- `"Server started rebuilding - resetting stats"`
- `"Server came back online after being offline/restarting/rebuilding - resetting stats"`
- `"Performing periodic config refresh (90s interval)"`
- `"Resetting all stats and clearing display data"`
- `"Stats reset complete"`

## Benefits

1. **Cleaner UI**: Old stale data is automatically cleared when server disconnects
2. **Accurate brackets**: Config changes are picked up automatically every 90s
3. **Better recovery**: Stats are reset when server comes back online
4. **No manual intervention**: Everything happens automatically
5. **Detailed visibility**: Container statuses shown during downtime help diagnose issues

## Future Enhancements

Potential improvements for future versions:

- Make config refresh interval configurable (currently hardcoded to 90s)
- Add visual indicator showing when last config refresh occurred
- Add manual "refresh config now" hotkey
- Detect config file changes via file watching instead of polling
