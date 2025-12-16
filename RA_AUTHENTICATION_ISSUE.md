# RA Authentication Issue

**Status:** ❌ Authentication Failing  
**Error:** "Authentication failed" from RA port 3443

---

## The Problem

The RA (Remote Administrator) console is rejecting the password. Based on testing:

```bash
$ ssh root@testing-azerothcore.rollet.family 'timeout 5 bash -c "{
  sleep 0.3; echo HAVOC;
  sleep 0.3; echo ch405P1x3l;
  sleep 0.3; echo \"server info\";
  sleep 1;
} | telnet localhost 3443 2>&1"'

Output:
Trying ::1...
Connected to localhost.
Escape character is '^]'.
Authentication Required
Username: Password: Authentication failed
Connection closed by foreign host.
```

---

## Why It's Failing

AzerothCore uses **SRP6 authentication** for account passwords, but RA might require:

1. A separate RA password (not the account password)
2. The password to be set via in-game command
3. A different password format/hash

---

## Solution 1: Set RA Password via Console

You mentioned you can use `docker attach testing-ac-worldserver` successfully. Try this:

### Step 1: Attach to WorldServer Console

```bash
docker attach testing-ac-worldserver
```

### Step 2: Set RA Password

Once you see the `AC>` prompt, run:

```
.account set password HAVOC ch405P1x3l ch405P1x3l
```

This sets the account password explicitly.

### Step 3: Test RA

Detach from console (Ctrl+P, Ctrl+Q), then test:

```bash
timeout 5 bash -c '{
  sleep 0.3; echo HAVOC;
  sleep 0.3; echo ch405P1x3l;
  sleep 0.3; echo "server info";
  sleep 1;
} | telnet localhost 3443'
```

If you see "Authentication successful" followed by server info output, it works!

---

## Solution 2: Create Dedicated RA Account

Create a new account specifically for RA access:

### Via Docker Attach Console:

```bash
docker attach testing-ac-worldserver
```

Then at the `AC>` prompt:

```
.account create BOTTOP_RA bottop123
.account set gmlevel BOTTOP_RA 3 -1
```

Then update your `~/.zshrc_envvars`:

```bash
export BOTTOP_AC_RA_USERNAME='BOTTOP_RA'
export BOTTOP_AC_RA_PASSWORD='bottop123'
```

---

## Solution 3: Use Direct Console Access Instead

Since `docker attach` works for you interactively, we could change bottop to use a different method entirely.

### Option A: Parse Docker Logs

WorldServer might log server stats periodically. We could parse `docker logs`.

### Option B: Use Database + InfluxDB

If you have InfluxDB configured, we could query metrics from there instead.

### Option C: Create a Console Wrapper Script

Create a script that uses `expect` or Python's `pexpect` to interact with `docker attach` programmatically.

---

## Immediate Next Steps

1. **Test current password** via console to confirm it works
2. **Set RA password** explicitly using `.account set password`
3. **Verify RA authentication** with manual telnet test
4. **Remove grep filter** (already done) to see full output
5. **Run bottop** and check `/tmp/bottop_debug.txt` for full RA response

---

## Updated Code

I've removed the `grep` filter so you can see the full RA output including authentication errors.

**New build:** `/home/havoc/bottop/build/bottop` (built at 19:25)

Run:

```bash
rm /tmp/bottop_debug.txt
./build/bottop  # Let it run for 5-10 seconds
cat /tmp/bottop_debug.txt
```

You should now see:

```
Command: timeout 5 bash -c '{ sleep 0.3; echo "HAVOC"; sleep 0.3; echo "ch405P1x3l"; sleep 0.3; echo "server info"; sleep 1; } | telnet localhost 3443 2>&1'
Result length: XXX
Result:
Trying ::1...
Connected to localhost.
Escape character is '^]'.
Authentication Required
Username: Password: Authentication failed
Connection closed by foreign host.
```

This confirms the authentication is the issue, not the parsing.

---

## Alternative: Direct Method You Use

**Question:** When you run `docker attach testing-ac-worldserver`, what exactly do you do to get the server info?

1. Do you just see the `AC>` prompt immediately?
2. Do you type `server info` and press Enter?
3. Does it show the output right there?

If yes to all three, then we can implement a solution that:

1. Creates a named pipe
2. Sends commands to it
3. Reads the output

Would you like me to implement that instead?

---

## Summary

**Current Issue:** RA password authentication failing  
**Likely Cause:** Password not set for RA or wrong format  
**Next Action:** Set password via console and test manually  
**Alternative:** Implement docker attach method you use interactively

Let me know which solution you'd like to pursue!
