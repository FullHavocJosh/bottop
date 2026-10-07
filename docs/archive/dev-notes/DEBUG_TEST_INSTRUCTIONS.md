# Debug Logging Test Instructions

## What Was Added

I've added comprehensive file-based debug logging to `/tmp/bottop_init_debug.log` to diagnose why the adaptive bracket loading isn't working.

The debug log will track:

1. When `init()` is called and whether AzerothCore support is enabled
2. SSH connection status
3. When `load_expected_values()` is called
4. Config file reading results
5. Bracket config file reading results
6. How many brackets were parsed
7. Details of each bracket (min, max, percentage)
8. Final bracket count loaded

## How to Test

### Step 1: Clear Old Debug Log

```bash
rm -f /tmp/bottop_init_debug.log
```

### Step 2: Run Bottop

Run bottop in a proper terminal (it requires a TTY):

```bash
./bin/bottop
```

### Step 3: Exit Bottop

Press `q` to quit bottop after a few seconds (or after you see the AzerothCore pane).

### Step 4: Check Debug Log

```bash
cat /tmp/bottop_init_debug.log
```

## What to Look For

### If Init is Running Correctly

You should see output like:

```
=== INIT CALLED ===
enabled=1
Proceeding with SSH connection
SSH connected successfully
About to call load_expected_values()

=== LOAD_EXPECTED_VALUES CALLED ===
Set 8 default brackets
Starting bracket config load from container: testing-ac-worldserver
About to read main config file from: /opt/azerothcore/etc/worldserver.conf
Got config content, length=12345
About to execute bracket config command: docker exec testing-ac-worldserver cat /azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf
Got bracket content, length=5678
Building brackets from parsed data, found 11 min levels
Max bracket number: 11
  Bracket 1: 1-9 (5%)
  Bracket 2: 10-19 (6%)
  Bracket 3: 20-29 (10%)
  ...
  Bracket 11: 80-80 (11%)
Final bracket count: 11
load_expected_values() returned, brackets loaded: 11
```

### Common Failure Scenarios

**Scenario 1: Init Not Running**

- Log file doesn't exist or is empty
- Means: `init()` never called OR logging failed to initialize
- **Action**: Check if AzerothCore support is enabled in config

**Scenario 2: Not Enabled**

```
=== INIT CALLED ===
enabled=0
Returning early: not enabled
```

- **Action**: Check `~/.config/bottop/bottop.conf` for `azerothcore_enable=True`

**Scenario 3: SSH Connection Failed**

```
SSH connection failed: Connection refused
```

- **Action**: Verify SSH credentials and host accessibility

**Scenario 4: No Remote Config Available**

```
No remote config available - ssh=yes, connected=yes, config_path=
```

- Means: `config.config_path` is empty
- **Action**: Check config file for `azerothcore_config_path` setting

**Scenario 5: Bracket Config Empty**

```
Got bracket content, length=0
Bracket config empty, using defaults
```

- Means: The docker exec command failed or config file doesn't exist
- **Action**: Manually test: `ssh root@testing-azerothcore.rollet.family "docker exec testing-ac-worldserver cat /azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf"`

**Scenario 6: No Brackets Parsed**

```
No brackets parsed (min_levels=0, max_levels=0)
```

- Means: Config file was read but parsing failed
- **Action**: Check config file format - should have `BotLevelBrackets.Alliance.Range1.Lower` etc.

## Next Steps Based on Results

After running the test and checking the log, report back:

1. Does the log file exist?
2. What does it contain (paste the full contents)?
3. At what point does the process fail (if at all)?

This will tell us exactly where the bracket loading is breaking down.
