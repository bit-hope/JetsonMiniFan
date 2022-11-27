# JetsonMiniFan

a mini fan controller for jetson.

一个用于控制 jetson PWM 风扇自动调速的小工具，支持添加到系统服务并以开机自启。

已对风扇在升速和降速逻辑间频繁切换，导致风扇出现啸叫噪音的情形，做了简单优化，效果改善比较明显。

已在 jetson nano 上测试通过，并长期运行。
