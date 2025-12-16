# Test Without Error Suppression

I've rebuilt bottop to **not suppress errors** (removed `2>/dev/null`).

## Run It Now

```bash
rm /tmp/bottop_debug.txt
./build/bottop
```

Let it run for a few seconds, then quit.

## Check Debug Output

```bash
cat /tmp/bottop_debug.txt
```

Now the "Result:" line should show the actual error message instead of being empty.

Common errors we might see:

1. **socat: not found** - Need to install socat in the container
2. **No such file or directory** - Wrong socket path
3. **Permission denied** - Socket permissions issue
4. **Connection refused** - WorldServer not running or socket not listening

Share the output and we can fix the specific issue!
