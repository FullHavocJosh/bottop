# RA (Remote Administrator) Setup Complete

**Date:** December 13, 2025, 19:22 EST  
**Status:** ✅ **READY FOR TESTING - Need RA Credentials**

---

## Summary

Bottop has been fully configured to use the **RA (Remote Administrator)** protocol to fetch WorldServer performance data from the `server info` command. The code is complete and built successfully - you just need to populate your RA credentials.

---

## What Was Done

### 1. ✅ Added RA Credentials to Environment Variables

**File:** `~/.zshrc_envvars`

Added these two lines (currently empty - you need to fill them in):

```bash
export BOTTOP_AC_RA_USERNAME=''
export BOTTOP_AC_RA_PASSWORD=''
```

### 2. ✅ Updated Configuration System

**Files Modified:**

- `src/btop_config.cpp` - Added config descriptions and environment variable reading
- `src/btop_azerothcore.hpp` - Added `ra_username` and `ra_password` to ServerConfig
- `src/btop.cpp` - Load RA credentials from config

**New Config Options:**

```
azerothcore_ra_username = ""
azerothcore_ra_password = ""
```

### 3. ✅ Implemented RA Protocol

**File:** `src/btop_azerothcore.cpp` (lines 386-400)

The fetch_server_performance function now:

1. Checks if RA credentials are configured
2. If yes: Uses telnet to connect to RA port 3443
3. Authenticates with username/password
4. Sends "server info" command
5. Parses the output

**Command Used:**

```bash
timeout 5 bash -c '{
  sleep 0.3; echo "USERNAME";
  sleep 0.3; echo "PASSWORD";
  sleep 0.3; echo "server info";
  sleep 1;
} | telnet localhost 3443 2>&1' | grep -A 50 "Connected players"
```

### 4. ✅ Built Successfully

```
Binary: /home/havoc/bottop/build/bottop
Size: 2.3 MB
Built: December 13, 2025, 19:22:32 EST
Warnings: 1 (unrelated to our code)
Errors: 0
```

---

## How to Complete Setup

### Step 1: Fill in RA Credentials

Edit `~/.zshrc_envvars` and add your GM account credentials:

```bash
export BOTTOP_AC_RA_USERNAME='HAVOC'
export BOTTOP_AC_RA_PASSWORD='your_actual_password_here'
```

**Requirements:**

- Account must have **GM level 3 or higher**
- Account must exist in `acore_auth.account` table
- Password is your actual game account password

### Step 2: Reload Environment Variables

```bash
source ~/.zshrc_envvars
```

Or open a new terminal (zsh will auto-load it).

### Step 3: Run Bottop

```bash
/home/havoc/bottop/build/bottop
```

---

## What You Should See

If RA credentials are correctly configured, you'll see:

```
┌─ AzerothCore Server ─────────────────────────┐
│ Status: ONLINE                                │
│                                               │
│ WorldServer Performance                       │
│   Rev: ece1060fa05d+ (Testing-Playerbot)     │
│        [RelWithDebInfo]                       │
│   Players: 0  Characters: 2842  Peak: 0      │
│   Uptime: 4 minute(s) 56 second(s)           │
│   Current: 174ms     ← Color-coded            │
│   Mean: 96ms  Median: 77ms                   │
│   P95: 186ms  P99: 198ms  Max: 227ms         │
│                                               │
│   [Real-time graph showing update times]      │
└───────────────────────────────────────────────┘
```

---

## Troubleshooting

### If You See "Database Query Time" Instead

This means RA authentication is not working. Check:

1. **Credentials are set:**

    ```bash
    echo $BOTTOP_AC_RA_USERNAME
    echo $BOTTOP_AC_RA_PASSWORD
    ```

2. **RA port is accessible:**

    ```bash
    ssh root@testing-azerothcore.rollet.family "timeout 2 nc -zv localhost 3443"
    ```

    Should show: "Connection to localhost 3443 port [tcp/*] succeeded!"

3. **Test RA manually:**

    ```bash
    ssh root@testing-azerothcore.rollet.family 'timeout 5 bash -c "{
      sleep 0.3; echo HAVOC;
      sleep 0.3; echo YOUR_PASSWORD;
      sleep 0.3; echo server info;
      sleep 1;
    } | telnet localhost 3443 2>&1"'
    ```

    You should see:

    ```
    Authentication Required
    Username: Password:
    AzerothCore rev. ece1060fa05d+ ...
    Connected players: 0. Characters in world: 2842.
    ...
    ```

4. **Check debug log:**
    ```bash
    rm /tmp/bottop_debug.txt
    /home/havoc/bottop/build/bottop  # Run for a few seconds then Ctrl+C
    cat /tmp/bottop_debug.txt
    ```

### If Authentication Fails

The error will show "Authentication failed" in telnet output. This means:

- Wrong username or password
- Account doesn't have GM level 3
- Account is banned/locked

**Verify your account:**

```bash
ssh root@testing-azerothcore.rollet.family \
  "docker exec -i testing-ac-database mysql -uroot -ppassword acore_auth -e \
  'SELECT a.id, a.username, aa.gmlevel
   FROM account a
   INNER JOIN account_access aa ON a.id = aa.id
   WHERE a.username = \"HAVOC\";' 2>&1 | grep -v Warning"
```

Should show:

```
id      username    gmlevel
1052    HAVOC       3
```

---

## Environment Variables Reference

All BOTTOP environment variables in `~/.zshrc_envvars`:

```bash
# SSH and Database
export BOTTOP_AC_SSH_HOST='root@testing-azerothcore.rollet.family'
export BOTTOP_AC_DB_HOST='testing-ac-database'
export BOTTOP_AC_DB_USER='root'
export BOTTOP_AC_DB_PASS='password'
export BOTTOP_AC_DB_NAME='acore_characters'
export BOTTOP_AC_CONTAINER='testing-ac-worldserver'

# RA Console Access (YOU NEED TO FILL THESE IN)
export BOTTOP_AC_RA_USERNAME=''
export BOTTOP_AC_RA_PASSWORD=''
```

---

## Alternative: Use Config File Instead

If you prefer not to use environment variables, you can add to `~/.config/bottop/bottop.conf`:

```ini
azerothcore_ra_username = "HAVOC"
azerothcore_ra_password = "your_password"
```

**Note:** Environment variables take priority over config file values.

---

## How RA Protocol Works

1. **Connect** to RA port (default 3443)
2. **Server prompts:** "Authentication Required\nUsername: "
3. **Client sends:** username + newline
4. **Server prompts:** "Password: "
5. **Client sends:** password + newline
6. **Server responds:** Authentication success/failure
7. **If successful:** Client can send commands
8. **Client sends:** "server info" + newline
9. **Server responds:** Full server info output
10. **Client disconnects**

Our implementation uses bash + telnet to handle this automatically.

---

## Files Modified

| File                       | Changes                                                     |
| -------------------------- | ----------------------------------------------------------- |
| `~/.zshrc_envvars`         | Added RA_USERNAME and RA_PASSWORD exports                   |
| `src/btop_config.cpp`      | Added config descriptions, defaults, env reading, help text |
| `src/btop_azerothcore.hpp` | Added ra_username and ra_password to ServerConfig           |
| `src/btop_azerothcore.cpp` | Implemented RA protocol in fetch_server_performance()       |
| `src/btop.cpp`             | Load RA credentials from config                             |

---

## Next Steps

1. **Fill in your RA credentials** in `~/.zshrc_envvars`
2. **Reload environment:** `source ~/.zshrc_envvars`
3. **Run bottop:** `/home/havoc/bottop/build/bottop`
4. **Verify WorldServer Performance display** shows all 12 metrics

---

## Security Notes

- RA password is stored in plaintext in `~/.zshrc_envvars`
- This file should have restricted permissions: `chmod 600 ~/.zshrc_envvars`
- Never commit this file to git
- Consider using a dedicated RA-only account instead of your main GM account
- RA connections are not encrypted (plaintext over network)
- Bottop connects via SSH first, so traffic to remote server is encrypted

---

## SOAP Alternative (Not Implemented)

We enabled SOAP on the server (port 7878) but didn't implement it because:

- SRP6 password authentication is complex
- RA is simpler (plain text protocol)
- RA was already enabled and working

If you want SOAP instead, you'd need to:

1. Implement SRP6 authentication in C++
2. Build XML requests
3. Parse XML responses

RA is much simpler for our use case.

---

**Status:** ✅ Code complete, build successful, ready for your RA credentials!  
**Binary:** `/home/havoc/bottop/build/bottop`  
**Action Required:** Fill in `BOTTOP_AC_RA_USERNAME` and `BOTTOP_AC_RA_PASSWORD` in `~/.zshrc_envvars`
