# WorldServer Performance - Socat Fix

## The Issue

The executable `testing-ac-worldserver` doesn't exist inside the container, so we need to use the **socat** method to connect to the worldserver socket.

## The Fix

Changed the command from:

```bash
docker exec -i testing-ac-worldserver testing-ac-worldserver 'server info'
```

To:

```bash
docker exec -i testing-ac-worldserver /bin/bash -c 'echo "server info" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'
```

## Test It

**1. Test the command manually first:**

```bash
ssh root@testing-azerothcore.rollet.family \
  "docker exec -i testing-ac-worldserver /bin/bash -c 'echo \"server info\" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'"
```

You should see output like:

```
AC> server info
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

**2. If that works, run bottop:**

```bash
./build/bottop
```

**3. Check the debug log again:**

```bash
cat /tmp/bottop_debug.txt
```

You should now see the server info data instead of the error.

## Expected Display

If everything works, you should see in bottop:

```
WorldServer Performance
  Rev: ece1060fa05d+ (Testing-Playerbot) [RelWithDebInfo]
  Players: 0  Characters: 2842  Peak: 0
  Uptime: 4 minute(s) 56 second(s)
  Current: 174ms  ← Color-coded
  Mean: 96ms  Median: 77ms
  P95: 186ms  P99: 198ms  Max: 227ms

  [Real-time graph]
```

## If It Still Doesn't Work

If the socat command doesn't work, check:

1. **Is socat installed in the container?**

    ```bash
    ssh root@testing-azerothcore.rollet.family \
      "docker exec -i testing-ac-worldserver which socat"
    ```

2. **Does the socket exist?**

    ```bash
    ssh root@testing-azerothcore.rollet.family \
      "docker exec -i testing-ac-worldserver ls -la /azerothcore/data/worldserver.sock"
    ```

3. **Try alternative socket paths:**
    - `/azerothcore/data/worldserver.sock`
    - `/opt/azerothcore/data/worldserver.sock`
    - `/var/run/worldserver.sock`
    - `/tmp/worldserver.sock`

Share the results and we can adjust the path if needed!
