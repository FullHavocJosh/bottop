#!/bin/bash

echo '=== SSH Detection Test ==='
echo ''
echo '1. Environment Check:'
echo '   SSH_CONNECTION: 192.168.144.196 56874 192.168.144.163 22'
echo '   SSH_CLIENT: 192.168.144.115 50118 22'
echo ''

echo '2. Current Config:'
grep update_ms ~/.config/bottop/bottop.conf 2>/dev/null || echo '   No config found'
echo ''

echo '3. Starting bottop for 3 seconds...'
cd /root/bottop
timeout 3 ./build/bottop 2>&1 &
BOTTOP_PID=$!
sleep 3
kill $BOTTOP_PID 2>/dev/null

echo ''
echo '4. Updated Config:'
grep update_ms ~/.config/bottop/bottop.conf 2>/dev/null || echo '   No config found'
echo ''

UPDATED_VALUE=$(grep 'update_ms' ~/.config/bottop/bottop.conf | grep -o '[0-9]*' | head -1)
if [ "$UPDATED_VALUE" = "5000" ]; then
    echo '✅ SUCCESS: SSH detection worked! update_ms changed to 5000ms'
elif [ "$UPDATED_VALUE" = "30000" ]; then
    echo '❌ FAILED: update_ms still at 30000ms (SSH detection did not trigger)'
else
    echo '⚠️  UNKNOWN: update_ms = '$UPDATED_VALUE'ms'
fi
