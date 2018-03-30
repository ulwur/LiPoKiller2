# LiPoKiller2

LiPoKiller2 is a device for discharging and analyzing hobby LIPO-Bateries. 

It Comprises of a PCB with circutits to measure cell balance and Mosfets for switching a discharging load (ie 12v halogen lightbulbs)

The Schematic and PCB is made with Altium CircuitMaker and is public in the repository

https://workspace.circuitmaker.com/Projects/Details/ulf-karlsson-3/LiPoKiller2

The code executes on a Adafrutit Metro Mini ardunio-like board and its purpose is to

* Measure the cell voltages and discharging current
* Control the loads (light bulbs)
* Stop discharging at storage voltage (3.8 volt per cell)
* Print all voltages and current to serial console for monitoring

Another project LiKiLink is software for a Wemos D1 Mini board that listens to the serial port 
and makes the console accessable via Telnet and posts the mesuremnt data to a MQTT server for 
graphing and display.

See: https://github.com/ulwur/LiKILink

