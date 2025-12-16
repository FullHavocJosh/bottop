# Config Path Fix Applied

## Problem Found

The debug log revealed that the config file read was returning 0 bytes because:

- **Config path in bottop.conf**: `/opt/azerothcore/etc/worldserver.conf`
- **Actual path in container**: `/azerothcore/env/dist/etc/worldserver.conf`
- The code was trying to `cat` the file directly on the SSH host (where it doesn't exist) instead of inside the container

## Fixes Applied

### 1. Updated Code (btop_azerothcore.cpp:1561)

Changed from:

```cpp
std::string cmd = "cat " + config.config_path;
```

To:

```cpp
std::string cmd = "docker exec " + config.container + " cat " + config.config_path;
```

Now it reads the config from inside the container, just like the bracket config.

### 2. Updated Config File (~/.config/bottop/bottop.conf)

Changed:

```
azerothcore_config_path = "/opt/azerothcore/etc/worldserver.conf"
```

To:

```
azerothcore_config_path = "/azerothcore/env/dist/etc/worldserver.conf"
```

This is the actual path inside the container.

### 3. Rebuilt Binary

Successfully recompiled in 50s.

## Test Again

Please run the same test:

```bash
rm -f /tmp/bottop_init_debug.log
./bin/bottop
# Wait a few seconds, press 'q' to quit
cat /tmp/bottop_init_debug.log
```

**Expected Result**: You should now see:

- `Got config content, length=` with a **large number** (not 0)
- `Got bracket content, length=` with a number
- `Building brackets from parsed data, found 11 min levels`
- List of all 11 brackets
- `Final bracket count: 11`

If this works, the UI should now display all 11 brackets including the single-level ones (60, 70, 80)!
