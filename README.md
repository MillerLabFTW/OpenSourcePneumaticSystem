# MillerLab Pneumatic System

##### An open-source Arduino-based Pneumatic system developed at Rice University
##### Miller Lab: Physiologic Systems Engineering and Advanced Materials
##### [millerlab.rice.edu] (http://millerlab.rice.edu)

###Overview

 Coming soon...


![photo of pneumatics setup](PicsVids/OpenSourcePneumaticSystem.jpg)

###Build Documentation
This git repository includes:
- Current firmware for Industruino IND.I/O kit
- Processing GUI to control the Industruino from a computer
- Wiring diagram for electronics configuration
- Bill of Materials for all parts we are using

###Installation Instructions
#####Uploading Firmware to Industruino
1. Download the latest version of Arduino from https://www.arduino.cc/en/Main/Software
2. Connect Industruino to computer via USB connection
3. Open PneumaticFirmware in Arduino
4. Under "Tools," select Board: "Arduino Leonardo". In "Tools," select the port the Industruino is connected to.
This can be determined using Device Manager and the port connectivity tool we've provided (see instructions below)
5. Finally, upload the firmware to Industruino using the upload button in Arduino

#####Processing Installation
1. Download the latest version of Processing from https://processing.org/download/
2. Connect Industruino to computer via USB connection
3. Open "ProcessingGUI" in Processing
4. Click the run button on Processing, then install ControlP5, minim, and sound libraries if prompted
5. ... (more thorough instructions coming soon)

Getting started:
1. Open ProcessingGUI in Processing
2. Connect Industruino to computer via USB
3. Run the ProcessingGUI in Processing. If a port error occurs, you can follow our troubleshooting guide (see below)
4. Specify the desired pneumatic settings and press Enter to run the program, or run your custom program (see Configuration for instructions on creating a custom program)
5. Press 0 to stop the program

###Configuration
####I want to:
######Change the ramp step size

######Reconfigure the Processing interface

######Create a custom pneumatic program


###Acknowledgements

![MillerLab logo](MillerLab_logo.jpg)
