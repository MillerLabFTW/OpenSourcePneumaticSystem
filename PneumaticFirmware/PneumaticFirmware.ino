/* Indio Library, by Rustem Iskuzhin for Industruino - wwww.industruino.com
This library enables communication with the Industruino IND.I/O Baseboard.
The IND.I/O Baseboard has 8CH of 24V digital I/O (6.5-32V supported), 4CH 0-10V/4-20mA ADC, 2CH 0-10V/4-20mA DAC, RS485 transceiver.


WARNING!: PLEASE BE VERY CAREFUL WHEN CONNECTING PERIPHERALS TO YOUR IND.I/O AND CONFIGURING THE DIRECTION OF THE I/O.
          THE FAILURE MODE OF A 24V SHORTED CONNECTION IS MUCH MORE SPECTACULAR THAN WHEN WORKING AT 5V. 


Please find the calibration data array inside the Indio.cpp library file, together with an explanation on how to perform the calibration. 
Library is preloaded with calibration data but characterisctics are board specific thus reading with standard cal. data might be off.
*/

#include <Indio.h>
#include <Wire.h>
#include <UC1701.h>
#include <U8glib.h>


// The dimensions of the LCD (in pixels)...
static const byte LCD_WIDTH = 128;
static const byte LCD_HEIGHT = 64;

static UC1701 lcd;

const int backlightPin = 13; // PWM output pin that the LED backlight is attached to


float analogInput1, analogInput2, analogInput3, analogInput4; //variables to hold your sensor data


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

String ch1string = "CH1 Set: ";
String ch2string = "CH2 Set: ";
String v_string = " mA";
String psi = " psi";
String estimated = " expected";
String percentage = "%";
String mon_string = " " + v_string + " monitor signal";
String output = " output";
String blank = "          ";


long ch1_PWM, ch2_PWM;
float ch1_accuracy, ch2_accuracy;

String ch1_accuracy_string, ch2_accuracy_string;

// RECEIVING COMMUNICATION FROM PROCESSING
// FORMAT IS: mode,minPWM,maxPWM,interval
String receivedString[4];


//Membrane panel button defines

int buttonUpState = 0; //status of "Up" button input
int buttonEnterState = 0; //status of "Enter" button input
int buttonDownState = 0; //status of "Down" button input

int prevBtnUp = 0; //previous state of "Up" button
int prevBtnEnt = 0; //previous state of "Enter" button
int prevBtnDown = 0; //previous state of "Down" button

int lastBtnUp = 0; //time since last "Up" pressed event
int lastBtnEnt = 0; //time since last "Enter" pressed event
int lastBtnDown = 0; //time since last "Down" pressed event

int enterPressed = 0; //status of "Enter" button after debounce filtering : 1 = pressed 0 = unpressed

int transEntInt = 250; //debounce treshold for "Enter" button
int transInt = 100; //debounce for other buttons
unsigned long lastAdminActionTime = 0; //keeps track of last button activity


// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A5;  // Analog input pin that the button panel is attached to

int ButtonsAnalogValue = 0;        // value read from mebrane panel buttons.

int defaultInterval = 1000;
int interval = defaultInterval; // interval in milliseconds at which to ramp voltage

int stepSize = 10; // in mA_raw, How many steps should we take? From 0-4095
int startAt = 0; // min is zero, 830 is ~ 4 mA
int counter = startAt;
int countUntil = 1000; // Up to 4095; 3660 is 20 mA according to measured values
//static variables are only set the first time through the loop


//Set the values for your E2P here
//For example, we ordered Omega.com IP610-030, which takes 4-20 mA to translate 0-30 psi
//For example, we ordered Omega.com IP710-X17, which takes 4-20 mA to translate 1-17 psi
// USE 610 OR 710 HERE:
int which_EtoP = 610;
float psi_low, psi_high;
float expected_mA;
float expected_psi;

/* These E2Ps are very linear for PWM -> mA or PWM -> mV outputs. Therefore, they will follow y = m*x + b equation
// So use measured data to approximate the mA we would expect
// However, the values are different for each output on each particular Industruino
// So, expected mA for a given PWM input can be calculated as:
expected_mA = psi_1m * PWM + psi_1b
where PWM == ch1_PWM or ch2_PWM PWM value (0-4095 which gives ~0=20 mA or 0=10 V)

And... once we have mA, we can then calculate expected psi using map because mA -> psi conversion should be perfectly linear
expected_psi = mapfloat(expected_mA, 4.000, 20.000, psi_low, psi_high)

*/

float psi_1m = 0.0056;
float psi_1b = -0.6637;
float psi_2m = 0.0056;
float psi_2b = -0.7037; 



/*  ARDUINO AND PROCESSING COMMUNICATION:
    USING INDUSTRUINO, we can't just access the pin values directly.
    we need to use the Indio library, which can only be accessed by Arduino directly.
    So, we will send strings back and forth between Processing and Arduino
    sent string and received string should be CSV format
    CONVENTION is to have all calculations done on Arduino, and have Processing just blindly
    display what Arduino says is correct. This has added benefit of Arduino can work independently
    from computer (e.g. with Industruino, keypad, and buttons), and future equipment can still use
    the same GUI! Keep it all modular.
    
    DESIGN NOTE: 610 ranges from 0-30 psi but has low precision. 710 ranges from 2-17 but with high precision.
    Both have internal vent ports which dump excess pressure to the room (e.g. when changing PWM from 3650 to 0,
    they dump excess air into the room). And, most dedicated on/off solenoids we could buy can't function at very low psi.
    SO... We COULD use wall air @ 100 psi, fed into manual regulator stepped down to 20 psi. This 20 psi gets fed into the 710
    which can step from 2-17 psi high precison. Then this is connected downstream to the 610 which we can oscillate 3650-0 PWM to turn ON/OFF.

    For now, let's hope we don't need that level of complexity. Just use 610 to start with since it goes to near zero pressure (NOTE THAT EVEN
    0 psi on 610 will still deliver some air, unfortunately. we'd really need a low flow solenoid to compensate for this and get complete "OFF").
*/


//    Arduino_string sent to GUI: output channel used (1or2),int which_EtoP,int psi_low,int psi_high, PWM (0-4095),interval(ms);mA(expected),psi(expected)
String Arduino_string = "1;610,0,30;0;1000;20.00000;30.00000";
//    GUI_string sent to Arduino: output channel used (1or2), which_EtoP, requested_PWM_low and requested_PWM_high,pulse_interval_on,pulse_interval_off
String GUI_string = "1;610;2200;3650;1000";
//    Example: User wants to use 610 E2P and set it to 5 psi. Arduino will have to translate this into proper PWM


/* DIFFERENT MODES:
 *  Mode 0: keep everything constant
 *  Mode 1: oscillate
 *  Mode 2: will receive minPWM and maxPWM from processing. Use it to 
*/
int mode = 0;


void setup()
{

  // PCD8544-compatible displays may have a different resolution...
  lcd.begin();

  pinMode(backlightPin, OUTPUT); //set backlight pin to output 
  analogWrite(backlightPin, 0); //convert backlight intesity from a value of 0-5 to a value of 0-255 for PWM.  
  
  // SET ADC RESOLUTION (analog to digital conversion resolution in sps == samples per second)
  // NOTE:: THE CONCOMITANT SPS VALUE HERE FOR EACH RESOLUTION SETTING PUTS SIGNIFICANT DELAYS ON TIME TO GET THROUGH THE MAIN loop()
  Indio.setADCResolution(12); // Set the ADC resolution. Choices are 12bit@240SPS, 14bit@60SPS, 16bit@15SPS and 18bit@3.75SPS.
  // jmil measured:
  //
  // at 12-bit resolution: two inputs can be read in 12 ms
  // at 14-bit resolution: two inputs can be read in 36 ms
  // at 16-bit resolution: two inputs can be read in 136 ms  
  // at 18-bit resolution: two inputs can be read in 535 ms

  Indio.analogWriteMode(1, mA_raw); // Set Analog-Out CH1 to 10V mode (0-10V).
  Indio.analogWriteMode(2, mA_raw); // Set Analog-Out CH2 to 10V mode (0-10V).

  Indio.analogReadMode(1, mA); // Set Analog-In CH1 to 10V mode (0-10V).
  Indio.analogReadMode(2, mA); // Set Analog-In CH2 to 10V mode (0-10V).
  Indio.analogReadMode(3, mA); // Set Analog-In CH1 to 10V mode (0-10V).
  Indio.analogReadMode(4, mA); // Set Analog-In CH2 to 10V mode (0-10V).
  // RECALL: For our ProportionAir 0-10 psi electronic pressure regulator, 0-10V ==> 0-10 psi. So report as psi reading
  
  // Write a piece of text on the first line of the LCD...
  lcd.setCursor(0, 0);
  lcd.print("MillerLab FTW");


  // SET VALUES ON STARTUP IMMEDIATELY BEFORE CONNECTION SO THAT WE DON'T HAVE HIGH PRESSURE BY DEFAULT  
  ch1_PWM = 1*counter;
  ch2_PWM = 1*counter;
    
  Indio.analogWrite(1, ch1_PWM, true); //Set CH2 to 2.67V ("true" will write value to EEPROM of DAC, restoring it after power cycling).
  Indio.analogWrite(2, ch2_PWM, true); //Set CH2 to 2.67V ("true" will write value to EEPROM of DAC, restoring it after power cycling).
    



  //Initialize serial and wait for port to open
  Serial.begin(9600); //enables Serial port for debugging messages  
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  establishContact();  // send a byte to establish contact until receiver responds

 }


 

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println(Arduino_string);   // send an initial string
    //delay(300);
  }
}


void loop()
{

  // PUTTING THIS SWITCH IN THE MAIN LOOP MEANS THAT THE RANGE CAN BE TWEAKED ON THE FLY BY THE GUI USER
  switch (which_EtoP) {
    case 710:
        psi_low = 1;
        psi_high = 17;
        break;
    case 610:
        psi_low = 0;
        psi_high = 30;
        break;
  }
  


  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB, such as Leonardo boards
  //}

  // RECEIVE STRING FROM PROCESSING COMMUNICATION
    // get incoming bytes and concatenate into receivedString:
    receivedString[0] = "0"; // reset the received string so that it's ready for the next one
    receivedString[1] = "0"; // reset the received string so that it's ready for the next one
    receivedString[2] = "0"; // reset the received string so that it's ready for the next one
    receivedString[3] = "0"; // reset the received string so that it's ready for the next one
//    if (Serial.available()){
//      receivedString = Serial.readStringUntil('\n');
//      Serial.println(receivedString);
//      ch1_PWM = long(receivedString);
//    }

    // We want to pass multiple variables in, parse them out into separate elements
    // of receivedString as an array
    int whichVariable = 0;
    while (Serial.available() > 0) {
      int inChar = Serial.read();
      if (isDigit(inChar)) {
          // convert the incoming byte to a char
          // and add it to the string:
         receivedString[whichVariable] += (char)inChar;
      } else if (inChar == ';') {
        whichVariable++;
      // if you get a newline, print the string, then the string's value:
      } else if (inChar == '\n') {
        //Serial.print("Value:");
        // GET THE MODE WE SHOULD USE BASED ON THE INPUT STRING
        mode = receivedString[0].toInt();

        Serial.print("String received by Arduino: ");
        Serial.println(receivedString[0] + ";" + receivedString[1] + ";" + receivedString[2] + ";" + receivedString[3]);
        // clear the string for new input:
        //inString = "";

        switch (mode) {
          case 0: // default, keep everything constant
            ch1_PWM = receivedString[1].toInt();            
            ch2_PWM = receivedString[1].toInt();
            //stepSize = ch1_PWM; // in mA_raw, How many steps should we take? From 0-4095
            //countUntil = stepSize; // Up to 4095; 3660 is 20 mA according to measured values
            interval = receivedString[3].toInt();          
           break;
          case 1: // oscillate
            ch1_PWM = receivedString[1].toInt();
            ch2_PWM = receivedString[1].toInt();
            counter = ch1_PWM;
            startAt = counter;
            countUntil = receivedString[2].toInt();
            stepSize = countUntil - startAt; // in mA_raw, How many steps should we take? From 0-4095
            //countUntil = stepSize; // Up to 4095; 3660 is 20 mA according to measured values
            interval = receivedString[3].toInt();          
           break;
          case 2:
            ch1_PWM = receivedString[1].toInt();
            ch2_PWM = receivedString[1].toInt();
            counter = ch1_PWM;
            startAt = ch1_PWM;
            countUntil = receivedString[2].toInt(); // Up to 4095; 3660 is 20 mA according to measured values
            // Might need to count down!
            if (countUntil < startAt) {
              stepSize = -10; // in mA_raw, How many steps should we take? From 0-4095
              //Serial.println("COUNTING DOWN with " + String(stepSize));
            } else {
              stepSize = 10; // in mA_raw, How many steps should we take? From 0-4095              
            }
            interval = receivedString[3].toInt();          
            break;
        }
      }
    }


  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();
  
  ReadButtons(); //check buttons


  if (currentMillis - previousMillis >= interval) {
    
    
    // Now reset the previous timestamp to the current timestamp so we are ready to loop again
    previousMillis = currentMillis;

    if ((mode == 1) || (mode == 2)) {
      ch1_PWM = 1*counter;
      ch2_PWM = 1*counter;
    }

    unsigned long awrite_start = millis();
    
    Indio.analogWrite(1, ch1_PWM, true); //Set CH2 to 2.67V ("true" will write value to EEPROM of DAC, restoring it after power cycling).
    Indio.analogWrite(2, ch2_PWM, true); //Set CH2 to 2.67V ("true" will write value to EEPROM of DAC, restoring it after power cycling).
    
    // ALLOW THE SET VOLTAGE TO EQUILIBRATE ON ANALOG OUTPUT 1 AND 2
    //delay(1000);

    unsigned long awrite_stop = millis();

    //Serial.println("Analog DAC Voltage set in:");
    //Serial.print(awrite_stop - awrite_start);
    //Serial.println(" ms");

    
    unsigned long aread_start = millis();

    //Serial.println(counter);
    counter = counter + stepSize;
    //Serial.println(counter);
    
    if ((counter > countUntil) && (counter > startAt)) {
      counter = startAt;
    } else if ((counter < countUntil) && (counter < startAt)) {
      counter = startAt;   
    }

  }

     // FOLLOW Arduino_string format
     // Arduino_string sent to GUI: output channel used (1or2),int which_EtoP,int psi_low,int psi_high, PWM (0-4095),interval(ms);mA(expected),psi(expected)
     // String Arduino_string = "1;610,0,30;3650;1000;20.00000;30.00000";
     //expected_mA = psi_1m * ch1_PWM + psi_1b;
     //expected_psi = mapfloat(expected_mA, 4.000, 20.000, psi_low, psi_high);
     expected_mA = analogInput1;
     expected_psi = mapfloat(analogInput1, 3.806, 19.803, psi_low, psi_high);
     Arduino_string = "1;"
     + String(which_EtoP)
     + ","
     + String(psi_low)
     + ","
     + String(psi_high)
     + ";"
     + String(ch1_PWM)
     + ";"
     + String(interval)
     + ";"
     + String(expected_mA, 5)
     + ";"
     + String(expected_psi, 5);
     
     Serial.print(Arduino_string);
     Serial.print('\n');
     delay(50);

    //for (int i=0; i < 1; i++) {
      //Serial.print(ch1_PWM);
      //Serial.print(",");
      analogInput1=Indio.analogRead(1); //Read Analog-In CH1 (output depending on selected mode)
      //Serial.print(analogInput1, 5);
      //Serial.print(",");
      //ch1_accuracy = (analogInput1) / (counter) * 10;
      //Serial.print(ch1_accuracy,5);
      // Serial.print(percentage);
     //Serial.print(ch1_accuracy_string);
      //Serial.print(",");
      
      //analogInput2=Indio.analogRead(2); //Read Analog-In CH1 (output depending on selected mode)
      //Serial.println(analogInput2, 5);
      //Serial.print(",");
      //ch2_accuracy = (analogInput2) / (counter) * 10;
      //Serial.print(ch2_accuracy,5);
      //Serial.println(percentage);
      //Serial.println(ch2_accuracy_string);
      //Serial.print(",");
      //analogInput3=Indio.analogRead(3); //Read Analog-In CH1 (output depending on selected mode)
      //Serial.println(analogInput3, 5);
      //analogInput4=Indio.analogRead(4); //Read Analog-In CH1 (output depending on selected mode)
      //Serial.println(analogInput4, 5);
   // }
    //Serial.println("");

    unsigned long aread_stop = millis();
    
    //Serial.println("Analog ADC Voltage read in:");
    //Serial.print(aread_stop - aread_start);
    //Serial.println(" ms");


    lcd.setCursor(0,1);
    String sec = " ms interval";
    lcd.print(interval + sec + blank);

    lcd.setCursor(0, 2);
    lcd.print(ch1string);
    lcd.print(int(ch1_PWM));
    lcd.print(blank);
    lcd.setCursor(0, 3);
    lcd.print(analogInput1, 5);
    lcd.print(v_string + output + blank);
    lcd.setCursor(0, 4);
    expected_psi = mapfloat(analogInput1, 3.806, 19.803, psi_low, psi_high);
    lcd.print(expected_psi, 5);
    lcd.print(psi + estimated + blank);

//    lcd.setCursor(0, 5);
//    lcd.print(ch2string);
//    lcd.print(int(ch2_PWM));
//    lcd.print(blank);
//    lcd.setCursor(0, 6);
//    lcd.print(analogInput2, 5);
//    lcd.print(v_string + output + blank);
//    lcd.setCursor(0, 7);
//    expected_psi = mapfloat(analogInput2, 3.806, 19.803, psi_low, psi_high);
//    lcd.print(expected_psi, 5);
//    lcd.print(psi + estimated + blank);


//    Serial.println("###########");
//    Serial.print(ch1string);
//    Serial.print(ch1_PWM, 5);
//    Serial.println(v_string);
//    Serial.print(analogInput1, 5);
//    Serial.println(mon_string + blank);
//    Serial.print(ch2string);
//    Serial.print(ch2_PWM, 5);
//    Serial.println(v_string);
//    Serial.print(analogInput2, 5);
//    Serial.println(mon_string + blank);
//    Serial.println("###########");
//    Serial.println("");
//    Serial.println("");
//    
    

    
       
  



}




//--------------------------------------------------------------------------------------------------------------------------------------------- 
// Peripheral functions
//--------------------------------------------------------------------------------------------------------------------------------------------- 

void ReadButtons(){ //reads the 3 buttons from a single analoge pin, filters and decides which button was pressed
  
  ButtonsAnalogValue = analogRead(analogInPin);  
 
   if (ButtonsAnalogValue>400 && ButtonsAnalogValue< 700 )
  {
  buttonEnterState = HIGH;
  }
  else
  {
  buttonEnterState = LOW;
  }
  
    if (ButtonsAnalogValue>100 && ButtonsAnalogValue<400)
  {
   buttonUpState = HIGH;
  }
  else
  {
  buttonUpState = LOW;
  }
   
   
    if (ButtonsAnalogValue>700 && ButtonsAnalogValue<1000 )
  {
  buttonDownState = HIGH;
  }
  else
  {
  buttonDownState = LOW;
  }
  
  
  //lcd.setCursor(0, 2);
  //lcd.print(blank + blank + blank);
  
  
  if (buttonUpState == HIGH && prevBtnUp == LOW) {
    
    lcd.setCursor(0, 1);
    lcd.print("** increase 500 ms **");
    delay(100);

    
    if ((millis() - lastBtnUp) > transInt) {
      
      interval += 500;
    }
    
    lastBtnUp = millis();
    lastAdminActionTime = millis();    
  }
  prevBtnUp = buttonUpState;

      
  if (buttonDownState == HIGH  && prevBtnDown == LOW) {

    lcd.setCursor(0, 1);
    lcd.print("** decrease 500 ms **");
    delay(100);

    if ((millis() - lastBtnUp) > transInt) {
      interval -= 500;
      if (interval < 0) {
        interval = 0;
      }
    }
    
    lastBtnUp = millis();
    lastAdminActionTime = millis();    
 

  }   
  prevBtnDown = buttonDownState;
  
  
  // USE ENTER BUTTON TO RESET TO DEFAULTS
  if (buttonEnterState == HIGH && prevBtnEnt == LOW) {

    lcd.setCursor(0, 2);
    String resetting = "***** RESETTING *****";
    lcd.print(resetting + blank);
    delay(100);
    
    if ((millis() - lastBtnEnt) > transEntInt) {

      counter = startAt;
      //interval = defaultInterval;
    
    }
    
    lastBtnEnt = millis();
    lastAdminActionTime = millis();
  }
  prevBtnEnt = buttonEnterState;
  

  
 }



float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


