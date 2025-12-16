-- AzerothCore Database Optimization for Bottop
-- This script adds indexes to improve query performance
-- Uses CREATE INDEX ... (ignores errors if index already exists)

USE acore_characters;

-- ============================================================================
-- CHARACTERS TABLE INDEXES
-- ============================================================================

-- Index for online character lookups (most common filter)
-- Covers: WHERE online = 1
CREATE INDEX idx_characters_online ON characters (online);

-- Index for zone-based queries
-- Covers: WHERE online = 1 AND zone = ?
CREATE INDEX idx_characters_online_zone ON characters (online, zone);

-- Index for map-based queries (continent lookups)
-- Covers: WHERE online = 1 AND map = ?
CREATE INDEX idx_characters_online_map ON characters (online, map);

-- Index for level-based queries
-- Covers: WHERE online = 1 AND level BETWEEN ? AND ?
CREATE INDEX idx_characters_online_level ON characters (online, level);

-- Index for race-based queries (faction lookups)
-- Covers: WHERE online = 1 AND race IN (...)
CREATE INDEX idx_characters_online_race ON characters (online, race);

-- Composite index for account filtering
-- Covers: WHERE online = 1 AND account NOT IN (...)
CREATE INDEX idx_characters_online_account ON characters (online, account);

-- Composite index for zone detail queries
-- Covers: WHERE online = 1 AND zone = ? (with level, race in SELECT)
CREATE INDEX idx_characters_zone_details ON characters (online, zone, level, race);

-- ============================================================================
-- ACCOUNT TABLE INDEXES
-- ============================================================================

USE acore_auth;

-- Index for username lookups (excluded accounts)
-- Covers: SELECT id FROM account WHERE username IN (...)
CREATE INDEX idx_account_username ON account (username);

-- ============================================================================
-- STATISTICS UPDATE
-- ============================================================================

USE acore_characters;
ANALYZE TABLE characters;

USE acore_auth;
ANALYZE TABLE account;

-- ============================================================================
-- VERIFICATION
-- ============================================================================

-- Show all indexes on characters table
SELECT 
    TABLE_NAME,
    INDEX_NAME,
    COLUMN_NAME,
    SEQ_IN_INDEX,
    CARDINALITY
FROM information_schema.STATISTICS
WHERE TABLE_SCHEMA = 'acore_characters' 
  AND TABLE_NAME = 'characters'
ORDER BY INDEX_NAME, SEQ_IN_INDEX;

-- Show all indexes on account table
SELECT 
    TABLE_NAME,
    INDEX_NAME,
    COLUMN_NAME,
    SEQ_IN_INDEX,
    CARDINALITY
FROM information_schema.STATISTICS
WHERE TABLE_SCHEMA = 'acore_auth' 
  AND TABLE_NAME = 'account'
ORDER BY INDEX_NAME, SEQ_IN_INDEX;
