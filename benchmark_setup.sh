#!/usr/bin/env sh

set -eux

sudo cpupower frequency-set --governor performance
echo off | sudo tee /sys/devices/system/cpu/smt/control
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
echo 0 | sudo tee /proc/sys/kernel/nmi_watchdog
echo performance | sudo tee /sys/module/pcie_aspm/parameters/policy

# A good example of /etc/default/grub to isolate all cores except 0,8, on a system with 8 cores (16 with HT / SMT) (find your CPU topology with lstopo):
# GRUB_CMDLINE_LINUX_DEFAULT="quiet splash processor.max_cstate=1 intel_idle.max_cstate=0 pcie_aspm=off isolcpus=3-4,11-12 nohz_full=3-4,11-12 rcu_nocbs=3-4,11-12 irqaffinity=0-2,5-10,13-15 intel_pstate=disabled"
