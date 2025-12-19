#!/bin/bash
# Unified Bot Relocation Script
# Uses zone brackets from playerbots.conf to ensure consistency
# Works for both startup relocation and manual relocation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${CONFIG_FILE:-/root/testing-azerothcore-wotlk/env/dist/etc/modules/playerbots.conf}"
PARSER_SCRIPT="${SCRIPT_DIR}/zone_bracket_parser.sh"

echo "=========================================="
echo "Unified Bot Relocation Script"
echo "=========================================="
echo "Config: $CONFIG_FILE"
echo ""

# Source the zone bracket parser
if [ ! -f "$PARSER_SCRIPT" ]; then
	echo "Error: Zone bracket parser not found: $PARSER_SCRIPT"
	exit 1
fi

source "$PARSER_SCRIPT" "$CONFIG_FILE"

# Verify coverage before proceeding
echo "Verifying zone bracket coverage..."
uncovered_levels=()
for level in {1..80}; do
	if ! is_level_covered "$level"; then
		uncovered_levels+=("$level")
	fi
done

if [ ${#uncovered_levels[@]} -gt 0 ]; then
	echo "⚠️  WARNING: The following levels are not covered by zone brackets:"
	echo "   ${uncovered_levels[*]}"
	echo ""
	echo "These bots will not be relocated. Please update playerbots.conf to add coverage."
	echo ""
fi

# Function to get zone name for logging
get_zone_name() {
	local zone_id=$1
	case $zone_id in
	1) echo "Dun Morogh" ;;
	12) echo "Elwynn Forest" ;;
	14) echo "Durotar" ;;
	85) echo "Tirisfal Glades" ;;
	141) echo "Teldrassil" ;;
	215) echo "Mulgore" ;;
	3430) echo "Eversong Woods" ;;
	3524) echo "Azuremyst Isle" ;;
	17) echo "Barrens" ;;
	40) echo "Westfall" ;;
	10) echo "Duskwood" ;;
	331) echo "Ashenvale" ;;
	45) echo "Arathi Highlands" ;;
	33) echo "Stranglethorn Vale" ;;
	440) echo "Tanaris" ;;
	490) echo "Un'Goro Crater" ;;
	618) echo "Winterspring" ;;
	3703) echo "Shattrath City" ;;
	4395) echo "Dalaran" ;;
	*) echo "Zone $zone_id" ;;
	esac
}

# Function to get appropriate zone for race and level
get_zone_for_race_and_level() {
	local race=$1
	local level=$2

	# Get all zones for this level
	local zones=($(get_zones_for_level "$level"))

	if [ ${#zones[@]} -eq 0 ]; then
		echo ""
		return 1
	fi

	# Define race-specific starter zones
	local -A race_starter_zones=(
		[1]=12    # Human -> Elwynn Forest
		[3]=1     # Dwarf -> Dun Morogh
		[4]=141   # Night Elf -> Teldrassil
		[7]=1     # Gnome -> Dun Morogh
		[11]=3524 # Draenei -> Azuremyst Isle
		[2]=14    # Orc -> Durotar
		[5]=85    # Undead -> Tirisfal Glades
		[6]=215   # Tauren -> Mulgore
		[8]=14    # Troll -> Durotar
		[10]=3430 # Blood Elf -> Eversong Woods
	)

	# Define faction-preferred zones
	local -A alliance_zones=(
		[40]=1   # Westfall
		[10]=1   # Duskwood
		[45]=1   # Arathi Highlands
		[38]=1   # Loch Modan
		[148]=1  # Darkshore
		[3525]=1 # Bloodmyst Isle
	)

	local -A horde_zones=(
		[17]=1   # Barrens
		[331]=1  # Ashenvale
		[33]=1   # Stranglethorn Vale
		[130]=1  # Silverpine Forest
		[3433]=1 # Ghostlands
	)

	# Check if race's starter zone is in the list
	local starter_zone=${race_starter_zones[$race]}
	if [ -n "$starter_zone" ]; then
		for zone in "${zones[@]}"; do
			if [ "$zone" = "$starter_zone" ]; then
				echo "$zone"
				return 0
			fi
		done
	fi

	# Check faction-preferred zones
	local is_alliance=0
	case $race in
	1 | 3 | 4 | 7 | 11) is_alliance=1 ;;
	esac

	if [ $is_alliance -eq 1 ]; then
		for zone in "${zones[@]}"; do
			if [ -n "${alliance_zones[$zone]}" ]; then
				echo "$zone"
				return 0
			fi
		done
	else
		for zone in "${zones[@]}"; do
			if [ -n "${horde_zones[$zone]}" ]; then
				echo "$zone"
				return 0
			fi
		done
	fi

	# Return first available zone as fallback
	echo "${zones[0]}"
	return 0
}

# Get zone coordinates from database
get_zone_coords() {
	local zone_id=$1

	# Query for innkeeper/flightmaster in this zone
	mysql -h testing-ac-database -uroot -p"$MYSQL_ROOT_PASSWORD" -sN acore_world 2>/dev/null <<EOF
SELECT 
    c.map,
    ROUND(c.position_x, 2),
    ROUND(c.position_y, 2),
    ROUND(c.position_z, 2),
    ROUND(c.orientation, 2)
FROM creature c
INNER JOIN creature_template t ON c.id1 = t.entry
INNER JOIN creature_template_addon a ON c.guid = a.guid
WHERE t.npcflag & 73728  -- Innkeeper or Flight Master
AND a.auras NOT LIKE '%16380%'  -- Not invisible
LIMIT 1;
EOF
}

echo "Generating relocation SQL..."
SQL_FILE="/tmp/unified_bot_relocate_$(date +%s).sql"

cat >"$SQL_FILE" <<'SQLHEADER'
USE acore_characters;

-- Set all bots offline before relocation
UPDATE characters 
SET online = 0 
WHERE guid IN (SELECT DISTINCT bot FROM acore_playerbots.playerbots_random_bots WHERE bot > 0);

START TRANSACTION;

SQLHEADER

# Generate SQL for each level
echo "Building relocation queries for levels 1-80..."

for level in {1..80}; do
	if ! is_level_covered "$level"; then
		echo "-- Level $level: No coverage, skipping" >>"$SQL_FILE"
		continue
	fi

	echo "" >>"$SQL_FILE"
	echo "-- ============================================================================" >>"$SQL_FILE"
	echo "-- LEVEL $level" >>"$SQL_FILE"
	echo "-- ============================================================================" >>"$SQL_FILE"

	# Process each race
	for race in {1..11}; do
		[ $race -eq 9 ] && continue # Skip invalid race ID

		zone_id=$(get_zone_for_race_and_level "$race" "$level")

		if [ -z "$zone_id" ]; then
			echo "-- Race $race, Level $level: No suitable zone found" >>"$SQL_FILE"
			continue
		fi

		zone_name=$(get_zone_name "$zone_id")
		echo "-- Race $race, Level $level -> Zone $zone_id ($zone_name)" >>"$SQL_FILE"

		# Get coordinates for this zone (you'll need to implement proper zone->coords mapping)
		# For now, we'll use placeholder logic
		# In production, this should query the database for appropriate locations

	done
done

cat >>"$SQL_FILE" <<'SQLFOOTER'

COMMIT;

-- ============================================================================
-- VERIFICATION QUERY
-- ============================================================================

SELECT 'Unified Bot Relocation Complete' as status, 
       NOW() as timestamp,
       COUNT(*) as total_bots_relocated
FROM characters 
WHERE guid IN (SELECT DISTINCT bot FROM acore_playerbots.playerbots_random_bots WHERE bot > 0);

-- Show distribution by level bracket
SELECT 
    CASE 
        WHEN level BETWEEN 1 AND 9 THEN '1-9'
        WHEN level BETWEEN 10 AND 19 THEN '10-19'
        WHEN level BETWEEN 20 AND 29 THEN '20-29'
        WHEN level BETWEEN 30 AND 39 THEN '30-39'
        WHEN level BETWEEN 40 AND 49 THEN '40-49'
        WHEN level BETWEEN 50 AND 59 THEN '50-59'
        WHEN level = 60 THEN '60'
        WHEN level BETWEEN 61 AND 69 THEN '61-69'
        WHEN level = 70 THEN '70'
        WHEN level BETWEEN 71 AND 79 THEN '71-79'
        WHEN level = 80 THEN '80'
        ELSE 'Other'
    END as level_bracket,
    COUNT(*) as bot_count,
    GROUP_CONCAT(DISTINCT zone ORDER BY zone) as zones_used
FROM characters 
WHERE guid IN (SELECT DISTINCT bot FROM acore_playerbots.playerbots_random_bots WHERE bot > 0)
GROUP BY level_bracket
ORDER BY MIN(level);

SQLFOOTER

echo ""
echo "SQL generated: $SQL_FILE"
echo ""
echo "Note: This is a template. The actual relocate script needs zone coordinate mapping."
echo "See relocate_bots_with_zones.sh for the complete implementation."
echo ""
echo "To complete this implementation, we need to:"
echo "1. Map zones to specific coordinates (can query creature table for innkeepers)"
echo "2. Handle map IDs for each zone"
echo "3. Add proper race/faction filtering for zone selection"
