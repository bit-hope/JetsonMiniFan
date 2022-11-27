#! /bin/bash

existCnt=$(systemctl list-unit-files | grep "jetson-mini-fan" | wc -l)
if [ ${existCnt} -gt 0 ]; then
    systemctl disable jetson-mini-fan
    systemctl stop    jetson-mini-fan
    rm -rf "/etc/systemd/system/jetson-mini-fan.service"
    systemctl daemon-reload
fi

rm -rf "/usr/bin/jetson_mini_fan"
