#!/bin/bash

# Dynamic Bot Relocation Script - V9
# FIXED: Proper level ranges matching playerbots.conf ZoneBracket definitions
# FIXED: Verified coordinates from actual bot positions
# FIXED: Non-overlapping level assignments

echo "Dynamic bot relocation V9: Verified coordinates and proper level ranges..."

SQL_FILE="/tmp/generated_relocate.sql"
cat >"$SQL_FILE" <<'EOF'
USE acore_characters;

-- Set all bots offline before relocation
UPDATE characters 
SET online = 0 
WHERE account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

START TRANSACTION;

-- ============================================================================
-- LEVELS 1-12: STARTER ZONES (matches ZoneBracket 1,12,14,85,141,215,3430,3524)
-- ============================================================================

-- Alliance Starter Zones (Levels 1-12)
UPDATE characters 
SET position_x = CASE race
    WHEN 1 THEN -8949.95    -- Human -> Elwynn Forest (zone 12)
    WHEN 3 THEN -6240.32    -- Dwarf -> Dun Morogh (zone 1)
    WHEN 4 THEN 10311.30    -- Night Elf -> Teldrassil (zone 141)
    WHEN 7 THEN -6240.32    -- Gnome -> Dun Morogh (zone 1)
    WHEN 11 THEN -3961.67   -- Draenei -> Azuremyst Isle (zone 3524)
END,
position_y = CASE race
    WHEN 1 THEN -132.49
    WHEN 3 THEN 331.03
    WHEN 4 THEN 832.46
    WHEN 7 THEN 331.03
    WHEN 11 THEN -13931.20
END,
position_z = CASE race
    WHEN 1 THEN 83.53
    WHEN 3 THEN 382.76
    WHEN 4 THEN 1326.41
    WHEN 7 THEN 382.76
    WHEN 11 THEN 100.62
END,
map = CASE race
    WHEN 1 THEN 0
    WHEN 3 THEN 0
    WHEN 4 THEN 1
    WHEN 7 THEN 0
    WHEN 11 THEN 530
END,
zone = CASE race
    WHEN 1 THEN 12
    WHEN 3 THEN 1
    WHEN 4 THEN 141
    WHEN 7 THEN 1
    WHEN 11 THEN 3524
END,
orientation = 0
WHERE level BETWEEN 1 AND 12
  AND race IN (1, 3, 4, 7, 11)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- Horde Starter Zones (Levels 1-12)
UPDATE characters 
SET position_x = CASE race
    WHEN 2 THEN -618.52      -- Orc -> Durotar (zone 14)
    WHEN 5 THEN 1681.00      -- Undead -> Tirisfal Glades (zone 85)
    WHEN 6 THEN -2917.58     -- Tauren -> Mulgore (zone 215)
    WHEN 8 THEN -618.52      -- Troll -> Durotar (zone 14)
    WHEN 10 THEN 10349.60    -- Blood Elf -> Eversong Woods (zone 3430)
END,
position_y = CASE race
    WHEN 2 THEN -4251.70
    WHEN 5 THEN 1667.75
    WHEN 6 THEN -257.98
    WHEN 8 THEN -4251.70
    WHEN 10 THEN -6357.29
END,
position_z = CASE race
    WHEN 2 THEN 38.72
    WHEN 5 THEN 121.04
    WHEN 6 THEN 52.97
    WHEN 8 THEN 38.72
    WHEN 10 THEN 33.13
END,
map = CASE race
    WHEN 2 THEN 1
    WHEN 5 THEN 0
    WHEN 6 THEN 1
    WHEN 8 THEN 1
    WHEN 10 THEN 530
END,
zone = CASE race
    WHEN 2 THEN 14
    WHEN 5 THEN 85
    WHEN 6 THEN 215
    WHEN 8 THEN 14
    WHEN 10 THEN 3430
END,
orientation = 0
WHERE level BETWEEN 1 AND 12
  AND race IN (2, 5, 6, 8, 10)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 13-20: SECONDARY ZONES
-- ============================================================================

-- Alliance: Westfall (zone 40, bracket 10-21) - using 13-20 to avoid overlap
UPDATE characters 
SET position_x = -10628.69, position_y = 1055.96, position_z = 36.24,
    map = 0, zone = 40, orientation = 0
WHERE level BETWEEN 13 AND 20
  AND race IN (1, 3, 4, 7, 11)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- Horde: Barrens (zone 17, bracket 10-25) - using 13-20 to avoid overlap
UPDATE characters 
SET position_x = -450.95, position_y = -2643.28, position_z = 96.23,
    map = 1, zone = 17, orientation = 0
WHERE level BETWEEN 13 AND 20
  AND race IN (2, 5, 6, 8, 10)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 21-30: MID-LEVEL ZONES
-- ============================================================================

-- Alliance: Duskwood (zone 10, bracket 19-33) - using 21-30
UPDATE characters 
SET position_x = -10515.92, position_y = -1281.91, position_z = 38.38,
    map = 0, zone = 10, orientation = 0
WHERE level BETWEEN 21 AND 30
  AND race IN (1, 3, 4, 7, 11)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- Horde: Ashenvale (zone 331, bracket 18-33) - using 21-30
UPDATE characters 
SET position_x = 2744.80, position_y = -1373.86, position_z = 107.83,
    map = 1, zone = 331, orientation = 0
WHERE level BETWEEN 21 AND 30
  AND race IN (2, 5, 6, 8, 10)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 31-40: HIGHER MID-LEVEL ZONES
-- ============================================================================

-- Alliance: Arathi Highlands (zone 45, bracket 30-42) - using 31-40
UPDATE characters 
SET position_x = -1513.99, position_y = -2627.51, position_z = 41.34,
    map = 0, zone = 45, orientation = 0
WHERE level BETWEEN 31 AND 40
  AND race IN (1, 3, 4, 7, 11)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- Horde: Stranglethorn Vale (zone 33, bracket 32-47) - using 31-40
UPDATE characters 
SET position_x = -11921.70, position_y = -59.54, position_z = 39.73,
    map = 0, zone = 33, orientation = 0
WHERE level BETWEEN 31 AND 40
  AND race IN (2, 5, 6, 8, 10)
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 41-50: HIGH-LEVEL CLASSIC ZONES
-- ============================================================================

-- Both factions: Tanaris (zone 47, bracket 42-51) - using 41-50
UPDATE characters 
SET position_x = -6823.28, position_y = 770.62, position_z = 57.88,
    map = 1, zone = 47, orientation = 0
WHERE level BETWEEN 41 AND 50
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 51-60: END-GAME CLASSIC ZONES
-- ============================================================================

-- Both factions: Un'Goro Crater (zone 490, bracket 49-58) and Winterspring (zone 618, bracket 54-61)
-- Split 51-55 to Un'Goro, 56-60 to Winterspring
UPDATE characters 
SET position_x = -6291.55, position_y = -1158.62, position_z = -258.17,
    map = 1, zone = 490, orientation = 0
WHERE level BETWEEN 51 AND 55
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

UPDATE characters 
SET position_x = 6726.54, position_y = -4645.06, position_z = 720.80,
    map = 1, zone = 618, orientation = 0
WHERE level BETWEEN 56 AND 60
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 61-70: BURNING CRUSADE ZONES
-- ============================================================================

-- Both factions: Shattrath City (zone 3703, bracket 61-70)
-- This is the TBC hub city, safe for all TBC level bots
UPDATE characters 
SET position_x = -1838.16, position_y = 5301.79, position_z = -12.43,
    map = 530, zone = 3703, orientation = 0
WHERE level BETWEEN 61 AND 70
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- ============================================================================
-- LEVELS 71-80: WRATH OF THE LICH KING ZONES
-- ============================================================================

-- Both factions: Dalaran (zone 4395, bracket 71-80)
-- This is the WotLK hub city, safe for all WotLK level bots
UPDATE characters 
SET position_x = 5804.15, position_y = 624.77, position_z = 647.77,
    map = 571, zone = 4395, orientation = 0
WHERE level BETWEEN 71 AND 80
  AND account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

COMMIT;

-- ============================================================================
-- VERIFICATION QUERY
-- ============================================================================

SELECT 'Bot Relocation V9 Complete' as status, 
       NOW() as timestamp,
       COUNT(*) as total_bots_relocated
FROM characters 
WHERE account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%');

-- Show distribution by level bracket
SELECT 
    CASE 
        WHEN level BETWEEN 1 AND 12 THEN '1-12 (Starters)'
        WHEN level BETWEEN 13 AND 20 THEN '13-20 (Secondary)'
        WHEN level BETWEEN 21 AND 30 THEN '21-30 (Mid)'
        WHEN level BETWEEN 31 AND 40 THEN '31-40 (Higher Mid)'
        WHEN level BETWEEN 41 AND 50 THEN '41-50 (High Classic)'
        WHEN level BETWEEN 51 AND 60 THEN '51-60 (Endgame Classic)'
        WHEN level BETWEEN 61 AND 70 THEN '61-70 (TBC)'
        WHEN level BETWEEN 71 AND 80 THEN '71-80 (WotLK)'
        ELSE 'Other'
    END as level_bracket,
    COUNT(*) as bot_count
FROM characters 
WHERE account IN (SELECT id FROM acore_auth.account WHERE username LIKE 'RNDBOT%')
GROUP BY level_bracket
ORDER BY MIN(level);

EOF

echo "Executing SQL..."
mysql -h testing-ac-database -uroot -p$MYSQL_ROOT_PASSWORD <"$SQL_FILE" 2>&1 | grep -v "Using a password"

rm -f "$SQL_FILE"
echo "Bot relocation V9 complete!"
