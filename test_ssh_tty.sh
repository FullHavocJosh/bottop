#!/bin/bash
cd /root/bottop
echo 'Current config update_ms:'
grep update_ms ~/.config/bottop/bottop.conf

echo ''
echo 'Starting bottop with TTY for 3 seconds (press q to quit early)...'
echo 'This will test if SSH detection changes update_ms from 30000 to 5000'
sleep 2

# Use script command to provide a pseudo-TTY
script -q -c 'timeout 3 ./build/bottop' /dev/null 2>&1 | head -20 &
sleep 4

echo ''
echo 'After running, config update_ms is:'
grep update_ms ~/.config/bottop/bottop.conf
