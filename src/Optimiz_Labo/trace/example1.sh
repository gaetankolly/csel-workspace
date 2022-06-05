#!/bin/sh
# 1. example
echo 1 > /sys/kernel/debug/tracing/events/sched/sched_switch/enable
./tracing  &
pidof tracing > /sys/kernel/debug/tracing/set_event_pid
echo 1 > /sys/kernel/debug/tracing/tracing_on ; sleep 2 ; echo 0 > /sys/kernel/debug/tracing/tracing_on
trace-cmd show
