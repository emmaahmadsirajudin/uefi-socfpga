@echo -off
echo Running startup.nsh - The Auto-Start UEFI Shell Script
fs1:
@echo -on
@ifconfig -s eth0 static 192.168.0.5 255.255.255.0 192.168.0.1
ifconfig -s eth0 dhcp
stall 5000000
ifconfig -l eth0
@echo .

