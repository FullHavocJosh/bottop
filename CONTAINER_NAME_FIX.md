# Container Name Configuration Issue - FIXED

## Date: December 15, 2025

## Problem

After fixing the SSH timeout bug, bottop was still showing no data. All database queries were returning empty results (length=0).

## Root Cause

The logs showed bottop was executing queries with the **wrong container name**:

```bash
docker exec testing-ac-worldserver mysql ...
```

But the actual container is named:

```bash
26a5e2cc361e_testing-ac-worldserver
```

### Why This Happened

The environment variable `BOTTOP_AC_CONTAINER` was set to `testing-ac-worldserver`, which **overrides** the config file setting.

In `src/btop_config.cpp:686-687`:

```cpp
if (const char* env_val = std::getenv("BOTTOP_AC_CONTAINER")) {
    strings["azerothcore_container"] = env_val;
}
```

### Verification

Testing with wrong container name:

```bash
$ ssh root@testing-azerothcore.rollet.family "docker exec testing-ac-worldserver mysql ..."
Error response from daemon: No such container: testing-ac-worldserver
```

Testing with correct container name:

```bash
$ ssh root@testing-azerothcore.rollet.family "docker exec 26a5e2cc361e_testing-ac-worldserver mysql ..."
4112  # <-- Success! Returns bot count
```

## The Fix

Updated the environment variable to the correct container name:

```bash
export BOTTOP_AC_CONTAINER=26a5e2cc361e_testing-ac-worldserver
```

Also added to `~/.bashrc` for persistence:

```bash
echo "export BOTTOP_AC_CONTAINER=26a5e2cc361e_testing-ac-worldserver" >> ~/.bashrc
```

## Configuration Priority

Bottop reads configuration in this order (highest priority first):

1. **Environment Variables** (e.g., `BOTTOP_AC_CONTAINER`) ← This was overriding everything!
2. **Config File** (`~/.config/bottop/bottop.conf`)
3. **Default Values**

## Environment Variables Used by Bottop

From `src/btop_config.cpp:673-694`:

- `BOTTOP_AC_SSH_HOST` - SSH host for remote server
- `BOTTOP_AC_DB_USER` - Database username (default: root)
- `BOTTOP_AC_DB_PASS` - Database password
- `BOTTOP_AC_DB_HOST` - Database host (default: testing-ac-database)
- `BOTTOP_AC_DB_NAME` - Database name (default: acore_characters)
- `BOTTOP_AC_CONTAINER` - Docker container name ⚠️ **THIS WAS THE PROBLEM**
- `BOTTOP_AC_RA_USERNAME` - Remote Access username (optional)
- `BOTTOP_AC_RA_PASSWORD` - Remote Access password (optional)

## Current Environment Variables

```bash
$ env | grep BOTTOP_AC
BOTTOP_AC_CONTAINER=26a5e2cc361e_testing-ac-worldserver  ✅ FIXED
BOTTOP_AC_DB_PASS=password
```

## Expected Behavior After Fix

After restarting bottop, the logs should now show:

```
2025/12/15 (XX:XX:XX) | ERROR: MYSQL DEBUG: Executing SSH command: docker exec 26a5e2cc361e_testing-ac-worldserver mysql ...
2025/12/15 (XX:XX:XX) | ERROR: MYSQL DEBUG: Got result (length=4): '4112'
2025/12/15 (XX:XX:XX) | ERROR: FETCH DEBUG: Bot count query returned: '4112'
2025/12/15 (XX:XX:XX) | ERROR: FETCH DEBUG: Parsed total=4112
2025/12/15 (XX:XX:XX) | ERROR: COLLECT DEBUG: fetch_all() returned, total=4112 zones=XX error=
```

And bottop should display:

- **Total bots online**: ~4100+
- **Zone distribution** with actual counts
- **Faction breakdown** (Alliance/Horde)
- **Level distribution**
- **All other AzerothCore data**

## Testing Instructions

1. Verify environment variable is set:

    ```bash
    echo $BOTTOP_AC_CONTAINER
    # Should output: 26a5e2cc361e_testing-ac-worldserver
    ```

2. Run bottop:

    ```bash
    ~/bottop/bin/bottop
    ```

3. Wait 5 seconds for first data collection

4. Verify data appears on screen

5. Check logs to confirm correct container name:
    ```bash
    tail -50 ~/.local/state/bottop.log | grep "docker exec"
    ```
    Should show `docker exec 26a5e2cc361e_testing-ac-worldserver`

## Alternative: Remove Environment Variable

If you prefer to use the config file instead of environment variables:

```bash
unset BOTTOP_AC_CONTAINER
# Remove from .bashrc
sed -i '/BOTTOP_AC_CONTAINER/d' ~/.bashrc
```

Then bottop will use the value from `~/.config/bottop/bottop.conf`:

```
azerothcore_container = "26a5e2cc361e_testing-ac-worldserver"
```

## Lesson Learned

Always check for **environment variable overrides** when config file settings don't seem to apply. Environment variables have **higher priority** than config files in bottop.

---

**Status**: FIXED ✅  
**Next step**: Restart bottop to see data
