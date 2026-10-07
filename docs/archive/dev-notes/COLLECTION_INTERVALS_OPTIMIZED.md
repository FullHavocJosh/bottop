# Collection Intervals Optimized

## Changes Made

### Performance Data Collection

**Changed:** 1 second → 5 seconds
**File:** `src/btop_azerothcore.cpp:58`

```cpp
const uint64_t PERF_UPDATE_INTERVAL_MS = 5000;  // Update every 5 seconds
```

### General Bot Data Collection

**Changed:** 10 seconds → 30 seconds
**File:** `src/btop_config.cpp:291`

```cpp
{"update_ms", 30000},
```

## Rationale

### Why 5 seconds for performance?

- Docker attach commands take 1-2 seconds to complete
- Reduces overhead from frequent docker attach operations
- Still responsive enough for monitoring server health
- Balances data freshness with system load

### Why 30 seconds for general data?

- Bot distribution, faction data, zone data changes slowly
- Reduces SSH connection overhead
- Reduces database query load on AzerothCore
- Still provides timely updates for monitoring purposes

## Impact

### Before:

- Performance fetch: Every 1 second
- General data fetch: Every 10 seconds
- Total operations per minute: 60 + 6 = 66 operations

### After:

- Performance fetch: Every 5 seconds
- General data fetch: Every 30 seconds
- Total operations per minute: 12 + 2 = 14 operations

**Reduction:** ~79% fewer fetch operations per minute

## Benefits

1. **Lower Network Overhead:** Fewer SSH connections
2. **Lower Docker Overhead:** Fewer docker attach operations (most expensive)
3. **Lower Database Load:** Fewer SQL queries against AzerothCore
4. **Better Stability:** Less chance of connection issues
5. **Still Responsive:** Data updates frequently enough for monitoring

## Build Status

✅ Successfully compiled with new intervals

- Binary: `/home/havoc/bottop/build/bottop`
- Build time: Dec 14, 2025, 15:53 EST
