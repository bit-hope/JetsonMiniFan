#! /bin/bash

. ./uninstall.sh

cp "./jetson_mini_fan" "/usr/bin/jetson_mini_fan"
cp "./jetson-mini-fan.service" "/etc/systemd/system/jetson-mini-fan.service"
systemctl daemon-reload
systemctl enable jetson-mini-fan
systemctl start  jetson-mini-fan
