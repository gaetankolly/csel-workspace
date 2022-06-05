#!/bin/sh
# 2. example
echo function_graph > /sys/kernel/debug/tracing/current_tracer
echo *i2c* >  /sys/kernel/debug/tracing/set_ftrace_filter
echo 1 >  /sys/kernel/debug/tracing/tracing_on
i2cdetect -y 0
echo 0 >  /sys/kernel/debug/tracing/tracing_on
trace-cmd show
echo nop > /sys/kernel/debug/tracing/current_tracer

