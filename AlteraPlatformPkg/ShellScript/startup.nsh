@echo -off
echo Running startup.nsh - The Auto-Start UEFI Shell Script
@echo -off
echo Running startup.nsh - The Auto-Start UEFI Shell Script
@echo -on
@ifconfig -s eth0 static 192.168.0.5 255.255.255.0 192.168.0.1
ifconfig -s eth0 dhcp
stall 5000000
ifconfig -l eth0
@echo .
fs0:
ls
help tftp

