# MillerLab Pneumatic System
## An open-source, Arduino-based Pneumatic system developed at Rice University


#####[Miller Lab: Physiologic Systems Engineering and Advanced Materials Laboratory](http://millerlab.rice.edu)
Developed by Jordan Miller, Bagrat Grigoryan, and Paul Greenfield
Documentation by Nick Calafat

## Overview
The MillerLab Pneumatic system is an open-source pneumatic fluid control system for conducting microfluidic experiments, utilizing an [Industruino](http://www.industruino.com/) board and a [Processing](http://www.processing.org/)-based graphical user interface **(GUI)** for controlling the system.

Our typical configuration operates on a 0-30 psi range, but system components can be easily customized. We've also provided a guide to making firmware changes for further customization. Enjoy!

![photo of pneumatics setup](PicsVids/OpenSourcePneumaticSystem.jpg)

##Build Documentation
This git repository includes:
- Current firmware for Industruino IND.I/O kit
- Processing GUI to control the Industruino from a computer
- Wiring diagram for electronics configuration
- Bill of Materials for all the parts we are using


##Required Hardware

[Arduino](http://www.arduino.cc/), [Industruino](https://industruino.com), and [Processing](http://processing.org/) are all cross-platform hardware and software systems, making them super awesome! **As such, this setup works well with Linux, MacOS, and Windows computer systems.**

To use this equipment you will need a tethered computer and display for live control of the pneumatic system. This could be a computer laptop, or even a very simple and low cost single board computer such as a [Raspberry Pi](https://www.raspberrypi.org/). Setting up and configuring a Raspberry Pi is outside the scope of this README, but if you're interested, we direct you to the [Rice Bioe 421/521 Microcontroller Applications](https://github.com/jmil/Bioe421_521-MicrocontrollerApplications) class; all the labs are available online and Lab 01 gets you going configuring a Raspberry Pi and getting it on the Internet and installing Arduino software.

##Installation Instructions
###Uploading Firmware to Industruino
1. Download the latest version of Arduino from [https://www.arduino.cc/en/Main/Software](https://www.arduino.cc/en/Main/Software)
2. Connect Industruino to computer via USB connection
3. Open PneumaticFirmware in Arduino
4. On the Arduino taskbar, navigate to `Tools`->`Board` and select `Arduino Leonardo`. Next, navigate to `Tools`->`Port` and select the COM port assigned to Industruino. The correct COM port can be determined by locating `Arduino Leonardo` under the `Ports` dropdown in Device Manager
5. Finally, upload the `PneumaticFirmware.ino` to Industruino using the upload button in Arduino Software

###Processing and Libraries Installation
1. Download the latest version of Processing from https://processing.org/download/
2. Connect Industruino to computer via USB connection
3. Open ProcessingGUI in Processing and run the program. If all libraries are installed, the GUI will open and you can proceed to the next section. 
If a library is missing, one of the following errors may occur:
![image of error in Processing console](PicsVids/ProcessingLibraryError.PNG)
4. Libraries can be installed by accessing `Sketch`->`Import Library`->`Add Library` on the Processing Taskbar. Find the missing libraries and press install. Libraries will automatically install to the Processing sketchbook

###GUI Overview
The Processing GUI (depicted below) allows the user to specify settings for a pneumatic program. 
The three buttons in the top left corner of the GUI change the behavior of the pneumatic program. Specifically, the system can be programmed to provide a constant PWM, a pulsing PWM between two specified values, or a ramping PWM between two specified values. These behaviors can also be selected by pressing keys 1, 2, or 3 (corresponding to constant, pulse, and ramp, respectively).

The large sliders on the GUI allow the user to specify the starting PWM (for constant, pulse and ramp), the ending PWM (for pulse and ramp), and the ms time interval (for pulse and ramp).

Beneath the sliders, numeric indicators display current program settings and system outputs.

![image of Processing GUI](PicsVids/ProcessingGUI.PNG)


###Starting a Pneumatic Program
1. Connect Industruino to computer via USB
1. Open `ProcessingGUI` in Processing
1. Run the ProcessingGUI in Processing. If a port error occurs, you can follow our troubleshooting guide (see below)
1. Specify the desired pneumatic settings and press Enter to run the program, or run your custom program (see Configuration for instructions on creating a custom program)
1. Press 0 to stop the program


###Use `CheckSerialPort.pde` to Determine Port Number

**NOTE: As detailed above, This equipment is compatible with Linux, Mac, and Windows systems because Arduino, Industruino, and Processing are compatible with those systems. The below details are for Windows, other platform-specific instructions may be provided at a later time.**

####On Windows:

1. Connect Industruino to the desired USB port on your computer
1. Open Device Manager and navigate to the `Ports(COM & LPT)` dropdown. Find `Arduino Leonardo` and take note of its assigned COM number
   ![photo of device manager](PicsVids/DeviceManager_PortNo.PNG)
1. Next, open `CheckSerialPort.pde` in Processing and run the program. Active COM ports will be listed, along with their indices in the ports list. Locate the COM port that Industruino is connected to (determined in step 2), and take note of its index
   ![photo of CheckSerialPort script](PicsVids/CheckSerialPort.PNG)
1. Open ProcessingGUI and navigate to the following line:
		String portName = Serial.list()[0]; //change the 0 to a 1 or 2 etc. to match your port
Change the bracketed number to the index noted in step 3. These steps can be repeated to troubleshoot any port connectivity issues, as the COM number assigned to Industruino will depend on USB port used and other connected devices




##More Configuration Options and Examples
###I want to:
####Change the ramp step size: edit Industruino firmware
Ramp step size controls the integral interval for PWM steps at each time interval.
For example, in the GUI configuration shown above we have a 1000ms interval set, and the default ramp step size is 10. This means there will be a 10 PWM change every 1000ms. Edit the following firmware line to change the PWM step size used in ramp mode:
	
    int stepSize = 10; // in mA_raw, How many steps should we take? From 0-4095

  
####Customize the Processing interface
The resolution on the PWM slider is autoscaled based on the range of PWM availble and the size of the GUI window. Thus, if you want greater resolution for a certain project, you can restrict the PWM slider to the range you're working in. For instance, changing min_PWM to 1000 and max_PWM to 2000 would increase the precision of the PWM slider scale in the Processing GUI. The interval line sets the default ms time interval.

	int min_PWM = 0;
	int max_PWM = 3995;
	int interval = 1000;
    
    
####Create a custom pneumatic program
The following code specifies a pneumatic program that ramps from 1300 to 2100 PWM at a 0ms interval (continuous ramp).
This code can be modified to create your own program. It can also be copied to create additional programs (just change the initiating key and the desired parameters).

    case('5'):			//specifies the numeric key that initiates the program (pressing 5 sets the following parameters)

      							//Using minim library to simply play the wav file:
      song.play(); 				// play the tone.wav file in the current directory
      song.rewind(); 			// "rewind" it so it can be played again
      min.setValue(1300);		//specifies the starting PWM
      max.setValue(2100);		//specifies the ending PWM
      interval = 0;				//specifies the time interval (in ms)
      modeSelected.activate(2);	// specifies the pneumatic behavior (0 = constant, 1 = pulse, 2 = ramp)
      break;


##Acknowledgements
- Thanks to [Arduino](https://www.arduino.cc) and [Processing](https://processing.org) for the open-source software that enabled us to build our pneumatic system
- Thanks to [Industruino](https://industruino.com) for providing the industrial board for controlling our system
![MillerLab logo](PicsVids/MillerLab_logo.jpg)