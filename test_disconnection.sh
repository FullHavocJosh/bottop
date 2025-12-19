#!/bin/bash
# Test script for disconnection detection and config refresh

set -e

echo "=== Bottop Disconnection & Config Refresh Test ==="
echo ""
echo "This script will test the new features:"
echo "  1. Stats reset on disconnection"
echo "  2. Stats reset on server restart"
echo "  3. Periodic 90s config refresh"
echo ""
echo "Prerequisites:"
echo "  - bottop must be running in another terminal"
echo "  - AzerothCore containers must be running initially"
echo ""

CONTAINER="testing-ac-worldserver"

# Check if container exists
if ! docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER}$"; then
	echo "ERROR: Container '${CONTAINER}' not found!"
	echo "Please update CONTAINER variable in this script."
	exit 1
fi

echo "Using container: ${CONTAINER}"
echo ""

# Function to wait for user confirmation
wait_for_user() {
	echo ""
	read -p "Press Enter to continue to next test..."
	echo ""
}

# Test 1: Server stop
echo "=== Test 1: Server Stop ==="
echo "This will stop the worldserver container."
echo "Expected: bottop should detect disconnection and reset stats"
echo ""
read -p "Ready to stop server? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
	echo "Stopping ${CONTAINER}..."
	docker stop ${CONTAINER}
	echo ""
	echo "✓ Container stopped"
	echo ""
	echo "Check bottop - you should see:"
	echo "  - Status: OFFLINE or RESTARTING"
	echo "  - All stats cleared (zeros)"
	echo "  - Container status details shown"
	echo ""
	echo "Log message: 'Server disconnected or went offline - resetting stats'"
	wait_for_user
fi

# Test 2: Server start
echo "=== Test 2: Server Start ==="
echo "This will start the worldserver container."
echo "Expected: bottop should detect recovery and reset stats + refresh config"
echo ""
read -p "Ready to start server? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
	echo "Starting ${CONTAINER}..."
	docker start ${CONTAINER}
	echo ""
	echo "✓ Container started"
	echo ""
	echo "Check bottop - you should see:"
	echo "  - Stats reset again when ONLINE"
	echo "  - Config refreshed immediately"
	echo "  - Data starts populating"
	echo ""
	echo "Log messages:"
	echo "  - 'Server came back online after being offline/restarting/rebuilding - resetting stats'"
	echo "  - 'Performing periodic config refresh (90s interval)'"
	wait_for_user
fi

# Test 3: Config change
echo "=== Test 3: Config Change (90s refresh) ==="
echo "This will show how to test periodic config refresh."
echo ""
echo "To test config refresh:"
echo "  1. Edit the bracket config in the container:"
echo "     docker exec ${CONTAINER} vi /azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf"
echo ""
echo "  2. Change a level bracket percentage (e.g., Range1.Pct)"
echo ""
echo "  3. Save and wait up to 90 seconds"
echo ""
echo "  4. Check bottop - brackets should update automatically"
echo ""
echo "Expected behavior:"
echo "  - Config is reloaded every 90 seconds"
echo "  - Log message: 'Performing periodic config refresh (90s interval)'"
echo "  - Level bracket distributions update to match new config"
echo ""

wait_for_user

# Test 4: Server restart
echo "=== Test 4: Server Restart ==="
echo "This will restart the worldserver container."
echo "Expected: bottop should detect restart and reset stats twice"
echo ""
read -p "Ready to restart server? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
	echo "Restarting ${CONTAINER}..."
	docker restart ${CONTAINER}
	echo ""
	echo "✓ Container restarting"
	echo ""
	echo "Check bottop - you should see:"
	echo "  - Status: ONLINE → OFFLINE/RESTARTING → ONLINE"
	echo "  - Stats reset at OFFLINE transition"
	echo "  - Stats reset again when ONLINE"
	echo "  - Config refreshed when ONLINE"
	echo ""
	echo "Log messages:"
	echo "  - 'Server disconnected or went offline - resetting stats'"
	echo "  - 'Server came back online after being offline/restarting/rebuilding - resetting stats'"
	echo "  - 'Performing periodic config refresh (90s interval)'"
	wait_for_user
fi

echo ""
echo "=== Testing Complete ==="
echo ""
echo "Summary of implemented features:"
echo "  ✓ Stats reset on disconnection (ONLINE → OFFLINE)"
echo "  ✓ Stats reset on restart detection (ONLINE → RESTARTING)"
echo "  ✓ Stats reset on recovery (OFFLINE/RESTARTING → ONLINE)"
echo "  ✓ Periodic config refresh every 90 seconds"
echo "  ✓ Immediate config refresh after server restart"
echo ""
echo "To view logs:"
echo "  tail -f ~/.config/btop/error.log | grep -E 'reset|refresh|config'"
echo ""
