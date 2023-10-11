# WLED-RPM-Meter
A WLED usermod used to display RPM to LED strip using Wemos D1 Mini board and ELM327 bluetooth OBD-II adapter.

## How It Works
This usermod uses ELM327 bluetooth OBD-II adapter to read RPM value from vehicle's ECU and display it to LED strip as percentage value. The LED strip must be set to "Percent" effect mode in order to function correctly.

## Usermod Configurations
* `Max RPM` -> Set max RPM used to calculate the RPM percentage
* `Refresh Rate` -> Set how many time per second to get new RPM data to vehicle's ECU.
