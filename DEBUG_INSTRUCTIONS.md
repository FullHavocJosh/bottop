# Debug Instructions - WorldServer Performance

## Run Bottop with Debug Logging

I've added debug logging to see what's happening with the server info command.

### Step 1: Run Bottop

```bash
./build/bottop
```

Let it run for a few seconds, then quit (press 'q').

### Step 2: Check Debug Log

```bash
cat /tmp/bottop_debug.txt
```

This will show:

- The exact command being executed
- The length of the result
- The actual result returned

### What to Look For

The debug output should show something like:

```
=== fetch_server_performance ===
Command: docker exec -i testing-ac-worldserver testing-ac-worldserver 'server info'
Result length: 423
Result: AC> server info
AC> AzerothCore rev. ece1060fa05d+ 2025-12-12 19:37:01 +0000 (Testing-Playerbot branch) (Unix, RelWithDebInfo, Static)
Connected players: 0. Characters in world: 2842.
Connection peak: 0.
Server uptime: 4 minute(s) 56 second(s)
Update time diff: 174ms. Last 500 diffs summary:
|- Mean: 96ms
|- Median: 77ms
|- Percentiles (95, 99, max): 186ms, 198ms, 227ms
AC>
```

### Possible Issues

1. **Result length: 0** - Command is not returning anything
    - Check if worldserver is running
    - Test the command manually

2. **Command looks wrong** - Wrong container name or command
    - Check your config at `~/.config/bottop/bottop.conf`

3. **Result has errors** - SSH or docker errors in output
    - May need to adjust the command

### Test the Command Manually

Try running this directly:

```bash
ssh root@testing-azerothcore.rollet.family \
  "docker exec -i testing-ac-worldserver testing-ac-worldserver 'server info'"
```

This should output the server info. If it doesn't work, we need to adjust the command.

### Common Alternative Commands

If the above doesn't work, try these alternatives:

1. **Using socat and socket:**

    ```bash
    docker exec -i testing-ac-worldserver /bin/bash -c \
      'echo "server info" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'
    ```

2. **Using worldserver-console:**

    ```bash
    docker exec -i testing-ac-worldserver worldserver-console 'server info'
    ```

3. **Using ac-console:**
    ```bash
    docker exec -i testing-ac-worldserver ac-console 'server info'
    ```

Share the output of `/tmp/bottop_debug.txt` and I can help diagnose the issue!
