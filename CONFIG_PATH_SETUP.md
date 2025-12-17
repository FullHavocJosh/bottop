# Config Path Setup for Remote Server Configuration

**Date**: December 16, 2024

## Overview

Bottop can pull expected values (level brackets, continent percentages) from your AzerothCore server's configuration files. This document explains how to set it up.

## How It Works

Bottop reads config files **from inside the Docker container** using SSH + docker exec:

```bash
# On remote server via SSH
docker exec <container_name> cat <config_path>
```

## Required Configuration

### Your Remote Server Structure

```
root@testing-azerothcore.rollet.family:
  testing-azerothcore-wotlk/
    env/
      dist/
        etc/
          worldserver.conf           ← Main config
          modules/
            mod_player_bot_level_brackets.conf  ← Bracket config
```

### Inside Docker Container

The paths **inside your container** should be:
```
/azerothcore/env/dist/etc/worldserver.conf
/azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf
```

## Bottop Configuration

Edit `~/.config/bottop/bottop.conf`:

```ini
# SSH connection
azerothcore_ssh_host = "root@testing-azerothcore.rollet.family"

# Docker container name (on remote server)
azerothcore_container = "testing-ac-worldserver"

# Path INSIDE the container to worldserver.conf
azerothcore_config_path = "/azerothcore/env/dist/etc/worldserver.conf"
```

**Important**: The `azerothcore_config_path` is the path **inside the container**, not the host filesystem path.

## What Gets Read From Configs

### From `worldserver.conf`

```ini
AiPlayerbot.MinRandomBots = 100
AiPlayerbot.MaxRandomBots = 500
```

Used for:
- Bot count validation
- Expected bot range

### From `mod_player_bot_level_brackets.conf`

#### Level Brackets
```ini
BotLevelBrackets.Alliance.Range1.Lower = 1
BotLevelBrackets.Alliance.Range1.Upper = 9
BotLevelBrackets.Alliance.Range1.Pct = 10.0

BotLevelBrackets.Alliance.Range2.Lower = 10
BotLevelBrackets.Alliance.Range2.Upper = 19
BotLevelBrackets.Alliance.Range2.Pct = 9.0

# ... etc
```

Used for:
- Level bracket definitions (1-9, 10-19, 60, 70, 80, etc.)
- Expected percentage per bracket
- Color coding (green/yellow/red based on deviation)

#### Continent Percentages
```ini
BotContinentPct.EasternKingdoms = 25.0
BotContinentPct.Kalimdor = 25.0
BotContinentPct.Outland = 25.0
BotContinentPct.Northrend = 25.0
```

Used for:
- Expected continent distribution
- Color coding (green/yellow/red based on deviation)

## Verifying Config Paths

### Test SSH Access
```bash
ssh root@testing-azerothcore.rollet.family "docker ps"
```

### Test Container Name
```bash
ssh root@testing-azerothcore.rollet.family "docker ps --filter 'name=ac-' --format '{{.Names}}'"
```

Expected output might be:
```
testing-ac-worldserver
testing-ac-authserver
testing-ac-mysql
```

### Test Config File Access
```bash
# Test worldserver.conf
ssh root@testing-azerothcore.rollet.family \
  "docker exec testing-ac-worldserver cat /azerothcore/env/dist/etc/worldserver.conf | head -20"

# Test bracket config
ssh root@testing-azerothcore.rollet.family \
  "docker exec testing-ac-worldserver cat /azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf | head -20"
```

If these commands fail, the paths might be different inside your container.

### Find Actual Paths Inside Container
```bash
# List files in container
ssh root@testing-azerothcore.rollet.family \
  "docker exec testing-ac-worldserver find / -name worldserver.conf 2>/dev/null"

ssh root@testing-azerothcore.rollet.family \
  "docker exec testing-ac-worldserver find / -name mod_player_bot_level_brackets.conf 2>/dev/null"
```

## Default Values (When Config Not Loaded)

If `azerothcore_config_path` is empty or files can't be read, bottop uses these defaults:

### Level Brackets (11 brackets)
```
1-9     = 10%
10-19   = 9%
20-29   = 9%
30-39   = 9%
40-49   = 9%
50-59   = 9%
60      = 10%
61-69   = 9%
70      = 8%
71-79   = 9%
80      = 18%
```

### Continents
```
No default percentages - will show grey (no health indication)
```

## Color Coding System

Once config is loaded, percentages are color-coded based on deviation:

- **Green**: ±0-1% from expected (on target)
- **Yellow**: ±2-3% from expected (warning)
- **Red**: >3% from expected (critical)

### Example

If config says `60 = 10%` and actual is:
- `10%` → Green (0% deviation)
- `11%` → Green (1% deviation)
- `12%` → Yellow (2% deviation)
- `14%` → Red (4% deviation)

## Troubleshooting

### Config Not Loading

Check bottop debug logs:
```bash
tail -f /tmp/bottop_expected_values_debug.log
tail -f /tmp/bottop_init_debug.log
```

### Common Issues

1. **Wrong container name**
   - Check: `docker ps --filter 'name=ac-'`
   - Fix: Set correct `azerothcore_container` name

2. **Wrong path inside container**
   - Check: `docker exec <container> ls /azerothcore/env/dist/etc/`
   - Fix: Use actual path from inside container

3. **SSH authentication fails**
   - Check: `ssh root@testing-azerothcore.rollet.family "echo test"`
   - Fix: Set up SSH keys or password authentication

4. **Docker permission denied**
   - Check: `ssh user@host "docker ps"`
   - Fix: Add SSH user to docker group or use root

## Testing Configuration

After setting up config, restart bottop and check:

1. **Level brackets display correctly**
   - Should show your custom brackets (not defaults)
   - Percentages should be colored based on your expected values

2. **Continent percentages colored**
   - Should show green/yellow/red based on your config
   - Grey means config wasn't loaded

3. **Check logs**
   ```bash
   # Look for successful config load
   grep "load_expected_values" /tmp/bottop_init_debug.log
   grep "Loaded.*brackets" /tmp/bottop_expected_values_debug.log
   ```

## Environment Variables Alternative

Instead of editing config file, you can use environment variables:

```bash
# In ~/.zshrc or ~/.bashrc
export BOTTOP_AC_SSH_HOST="root@testing-azerothcore.rollet.family"
export BOTTOP_AC_CONTAINER="testing-ac-worldserver"
export BOTTOP_AC_CONFIG_PATH="/azerothcore/env/dist/etc/worldserver.conf"
```

Then leave the config file values empty.

## Summary

✅ **Config Path**: Path **inside** Docker container, not host filesystem  
✅ **Bracket Config**: Hardcoded to `/azerothcore/env/dist/etc/modules/mod_player_bot_level_brackets.conf`  
✅ **Access Method**: SSH → docker exec → cat config file  
✅ **Fallback**: Uses defaults if config can't be loaded  
✅ **Color Coding**: Requires config to show health colors (green/yellow/red)

Set your config path to match your container's internal structure, not the host directory layout.
