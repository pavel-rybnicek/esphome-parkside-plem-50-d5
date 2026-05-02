# esphome-parkside-plem-50-d5
Parkside PLEM 50 D5 laser range finder UART Integration for ESPHome

## Description
Parkside PLEM 50 D5 is a cheap (€10-€20) laser finder. I find this one very useful for monitoring water levels in water tanks and drainages.

The device consist of laser sensor module and main PCB + display module. Laser module is connected to main board with 4-conductor cable - UART interface. Pins are 
desctibed on the PCB. Baud rate is 115200.
For ESPHome integration we need the laser sensor module only.

This project is not finished - I've damaged the electronics during the development. However, the communication protocol is pretty simple - it's
implemented in the cpp module - see "update" and "process_measurement" methods.

## Usage
[See here for how to use external components](https://esphome.io/components/external_components.html).
See the example configuration file.

