# Database Query Time Metric

## What Does "Database Query Time" Measure?

The **Database Query Time** metric shows the total round-trip time to execute a MySQL query, including:

- SSH network latency to the remote server
- Docker exec overhead
- MySQL connection establishment
- Query execution time
- Data return

This is **NOT** the WorldServer's actual tick/update time - it's a measure of how fast bottop can query your database.

## Typical Values

- **< 100ms** (Green) - Excellent - Fast network and database
- **100-200ms** (Yellow) - Acceptable - Moderate latency
- **≥ 200ms** (Red) - Slow - High network latency or slow database

## Why Can't We Get Real WorldServer Performance?

Without InfluxDB, AzerothCore doesn't expose WorldServer update times. The options are:

### Option 1: Use InfluxDB (Recommended)

Configure AzerothCore to send metrics to InfluxDB for real server performance data.

### Option 2: Current Approach (Default)

Measure database query time as a proxy for overall system responsiveness.

### Option 3: Parse WorldServer Logs

Extract performance data from worldserver logs (complex, not real-time).

## Query Being Timed

```sql
SELECT COUNT(*) FROM characters
WHERE online = 1
AND account NOT IN (
    SELECT id FROM acore_auth.account
    WHERE username IN ('HAVOC','JOSHG','JOSHR','JON','CAITR','COLTON','KELSEYG','KYLAN','SETH','AHBOT')
);
```

This query counts online bots (excluding admin accounts).

## Improving Query Performance

If your query times are high:

1. **Add index on characters table**:

    ```sql
    CREATE INDEX idx_online ON characters(online);
    CREATE INDEX idx_account ON characters(account);
    ```

2. **Cache excluded accounts**: The subquery runs every time

3. **Reduce network latency**: Run bottop closer to the database

4. **Enable MySQL query cache** (if not already enabled)

## Color Coding

- **Green** (< 100ms) - Excellent
- **Yellow** (100-200ms) - Acceptable
- **Red** (≥ 200ms) - Poor

Graph scale is fixed at 0-300ms.
