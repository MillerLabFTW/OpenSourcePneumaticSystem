/**
 * ControlP5 Slider. Horizontal and vertical sliders, 
 * with and without tick marks and snap-to-tick behavior.
 * by andreas schlegel, 2010
 */

/**
* ControlP5 Slider
*
* Horizontal and vertical sliders, 
* With and without tick marks and snap-to-tick behavior.
*
* find a list of public methods available for the Slider Controller
* at the bottom of this sketch.
*
* by Andreas Schlegel, 2012
* www.sojamo.de/libraries/controlp5
*
*/

import processing.serial.*;

Serial myPort;  // Create object from Serial class


import controlP5.*;

ControlP5 cp5;

controlP5.Slider min;
controlP5.Textlabel min_t;
controlP5.Slider max;
controlP5.Textlabel max_t;
controlP5.Slider interval_slider;
controlP5.Textlabel interval_t;

controlP5.Textlabel current_min;
controlP5.Textlabel current_max;
controlP5.Textlabel current_interval;

controlP5.Textlabel EtoP_label;
controlP5.Textlabel PWM_label;
controlP5.Textlabel expected_mA_label;
controlP5.Textlabel expected_psi_label;

RadioButton modeSelected;

int myColor = color(18,39,59);

int arduino_EtoP = 610;
int arduino_PWM = 850;
float psi_low, psi_high;
float expected_mA = 15;
float expected_psi = 20;

int sliderValue = 200;
int min_PWM = 0;
int max_PWM = 3995;
int interval = 1000;
int arduino_interval;


import processing.sound.*;
import ddf.minim.*;
 
Minim minim;
AudioPlayer song;
 



void setup() {
  size(700,600);
  smooth(8);
  //noStroke();
  cp5 = new ControlP5(this);
  
  
  // PREPARE WAV FILE TO BE PLAYED
  minim = new Minim(this);
 
  // this loads mysong.wav from the data folder
  song = minim.loadFile("tone.wav");

  
  
  // CONNECT TO ARDUINO
  
  //String portName = "/dev/cu.usbmodem1411";
  String portName = Serial.list()[0]; //change the 0 to a 1 or 2 etc. to match your port
  println(portName);
  
  myPort = new Serial(this, portName, 9600);

  // read bytes into a buffer until you get a linefeed (ASCII 10):
  myPort.bufferUntil('\n');

  // draw with smooth edges:
  smooth(8);
  
  // (2)
  // load a new font. ControlFont is a wrapper for processing's PFont
  // with processing 1.1 ControlFont.setSmooth() is not supported anymore.
  // to display a font as smooth or non-smooth, use true/false as 3rd parameter
  // when creating a PFont:
  
  PFont pfont = createFont("Arial",20,true); // use true/false for smooth/no-smooth
  ControlFont font = new ControlFont(pfont,30);
  
  
  //Radio Button
    modeSelected = cp5.addRadioButton("radioButton")
         .setPosition(5,5)
         .setSize(30,20)
         .setColorForeground(color(120))
         .setColorActive(color(255))
         .setColorLabel(color(255))
         .setItemsPerRow(5)
         .setSpacingColumn(50)
         .addItem("Constant",0)
         .addItem("Pulse",1)
         .addItem("Ramp",2)
         ;
     modeSelected.activate(0);

     

  min = cp5.addSlider("min_PWM")
     .setPosition(170,50)
     .setRange(min_PWM,max_PWM) // values can range from big to small as well
     .setSize(500,50)
     .setLabelVisible(false)
     //.setValue(5)
     //.setNumberOfTickMarks(7)
     .setSliderMode(Slider.FLEXIBLE)
     ;

  min_t = cp5.addTextlabel("min_t","Choose starting PWM",230,10)
  .setFont(font)
  .setColorValue(255)
  ;
  
  current_min = cp5.addTextlabel("min_value",str(min_PWM) + " PWM",10,50)
  .setFont(font)
  .setColorValue(255)
  ;

  max = cp5.addSlider("max_PWM")
     .setPosition(170,160)
     .setRange(min_PWM,max_PWM) // values can range from big to small as well
     .setSize(500,50)
     .setLabelVisible(false)
     //.setValue(2200)
     //.setNumberOfTickMarks(7)
     .setSliderMode(Slider.FLEXIBLE)
     ;


//float value = min_t.getValue();
  current_max = cp5.addTextlabel("max_value",str(max_PWM) + " PWM",10,160)
  .setFont(font)
  .setColorValue(255)
  ;


  max_t = cp5.addTextlabel("max_t","Choose ending PWM",230,120)
  .setFont(font)
  .setColorValue(255)
  ;


    interval_slider = cp5.addSlider("interval")
     .setPosition(170,260)
     .setRange(0,10000) // values can range from big to small as well
     .setSize(500,50)
     .setLabelVisible(false)
     //.setValue(2200)
     .setNumberOfTickMarks(21)
     .setSliderMode(Slider.FLEXIBLE)
     ;

  
 current_interval = cp5.addTextlabel("current_interval_label", str(interval) + " ms",10,260)
 .setFont(font)
 .setColorValue(255)
 ;


  interval_t = cp5.addTextlabel("interval_t_label","Choose ms interval",230,220)
  .setFont(font)
  .setColorValue(255)
  ;




  EtoP_label = cp5.addTextlabel("EtoP_label", "Regulator: ",10,350)
  .setFont(font)
  .setColorValue(255)
  ;

  PWM_label = cp5.addTextlabel("PWM_used", "PWM: ",10,400)
  .setFont(font)
  .setColorValue(255)
  ;

  expected_mA_label = cp5.addTextlabel("expected_mA", "Digital gauge measured mA: ",10,450)
  .setFont(font)
  .setColorValue(255)
  ;

  expected_psi_label = cp5.addTextlabel("expected_psi", "Digital gauge measured psi: ",10,500)
  .setFont(font)
  .setColorValue(255)
  ;
  
  
  //Textlabel myTextlabel = cp5.addTextlabel(this,"a single textlabel.",20,100,400,200,0xffff0000,ControlP5.synt24);
  //myTextlabel.setLetterSpacing(2);

}

void draw() {
  background(myColor);
  
  current_min.setValue(str(min_PWM) + " PWM");
  current_max.setValue(str(max_PWM) + " PWM");
  current_interval.setValue(str(interval) + " ms");

  if (mousePressed == true) 
  {                           //if we clicked in the window
   //myPort.write(str(min.getValue()) + "," + str(max.getValue()));         //send a 1
   //println("1");   
  } else 
  {                           //otherwise
  //myPort.write('0');          //send a 0
  }   
  
  
   EtoP_label.setValue("Regulator: " + arduino_EtoP + ", interval: " + str(arduino_interval) + " ms");
   PWM_label.setValue("PWM: " + arduino_PWM);
   expected_mA_label.setValue("Digital Gauge mA: " + expected_mA);
   expected_psi_label.setValue("Digital Gauge psi: " + expected_psi);
   
   
      
}


void slider(float theColor) {
  myColor = color(theColor);
  //println("a slider event. setting background to "+theColor);
}




void keyPressed() {
  switch(key) {
    case('1'):
      modeSelected.activate(0);
      break;
    case('2'):
      modeSelected.activate(1);
      break;
    case('3'):
      modeSelected.activate(2);
      break;
    case('5'):
      //Using minim library to simply play the wav file:
      song.play(); // play the tone.wav file in the current directory
      song.rewind(); // "rewind" it so it can be played again
      min.setValue(1300);
      max.setValue(2100);
      interval = 0;
      modeSelected.activate(2);
      break;
    case('0'):
      min.setValue(0);
      max.setValue(0);
      interval = 0;
      modeSelected.activate(0);
      break;
    case(ENTER):
       // send GUI-string for processing:
      sendDataToArduino();
      //myPort.write("2" + ";" + str(min_PWM) + ';' + str(max_PWM) + ';' + str(interval));
      //int theModeToRequest = int(modeSelected.getValue());
      //myPort.write(str(theModeToRequest) + ";" + str(min_PWM) + ';' + str(max_PWM) + ';' + str(interval));
      //myPort.write('\n');
      break;
  }
}

void sendDataToArduino() {
      int theModeToRequest = int(modeSelected.getValue());
      String stringToSend = str(theModeToRequest) + ";" + str((min_PWM)) + ';' + str((max_PWM)) + ';' + str((interval));
      //String stringToSend = "0;" + str((min_PWM)) + ';' + str((max_PWM)) + ';' + str((interval));
      //println("SENDING DATA TO ARDUINO:" + stringToSend);
      myPort.write(stringToSend);
      myPort.write('\n');
}

void controlEvent(ControlEvent theEvent) {
  if(theEvent.isFrom(modeSelected)) {
    //println("RADIO ACTIVATED");
    sendDataToArduino();
  }
}


void serialEvent(Serial myPort) {
  // read the serial buffer:
  String myString = myPort.readStringUntil('\n');
  // if you got any bytes other than the linefeed:
    myString = trim(myString);
    //println("RawStringReceivedFromArduino: " + myString + "\t");
    
    //RECALL: FORMAT IS: 1;610,0.00,30.00;1750.00;9.13630;9.63056
    //    Arduino_string sent to GUI: output channel used (1or2),int which_EtoP,int psi_low,int psi_high, PWM (0-4095),interval(ms);mA(expected),psi(expected)
    //String Arduino_string = "1;610,0,30;3650;1000;20.00000;30.00000";

    //arduino_EtoP = 610;
    //arduino_PWM = 850;
    //psi_low, psi_high;
    //arduino_interval;
    //expected_mA = 15;
    //expected_psi = 20;

    //// split the string at the semicolons
    String[] arduino_strings = split(myString, ';');
    
    // ONLY PROCESS COMPLETE STRINGS! OTHERWISE IT WILL CRASH
    if (arduino_strings.length == 6){
    String[] EtoP_details = split(arduino_strings[1],',');
    
    arduino_EtoP = int(EtoP_details[0]);
    arduino_PWM = int(arduino_strings[2]);
    arduino_interval = int(arduino_strings[3]);
    //expected_mA = float(arduino_strings[3]);
    float testing1 = float(arduino_strings[4]);
    expected_mA = testing1;
    float testing2 = float(arduino_strings[5]);
    expected_psi = testing2;
    }


  }
  




/**
* ControlP5 Slider
*
*
* find a list of public methods available for the Slider Controller
* at the bottom of this sketch.
*
* by Andreas Schlegel, 2012
* www.sojamo.de/libraries/controlp5
*
*/

/*
a list of all methods available for the Slider Controller
use ControlP5.printPublicMethodsFor(Slider.class);
to print the following list into the console.

You can find further details about class Slider in the javadoc.

Format:
ClassName : returnType methodName(parameter type)


controlP5.Controller : CColor getColor() 
controlP5.Controller : ControlBehavior getBehavior() 
controlP5.Controller : ControlWindow getControlWindow() 
controlP5.Controller : ControlWindow getWindow() 
controlP5.Controller : ControllerProperty getProperty(String) 
controlP5.Controller : ControllerProperty getProperty(String, String) 
controlP5.Controller : ControllerView getView() 
controlP5.Controller : Label getCaptionLabel() 
controlP5.Controller : Label getValueLabel() 
controlP5.Controller : List getControllerPlugList() 
controlP5.Controller : Pointer getPointer() 
controlP5.Controller : Slider addCallback(CallbackListener) 
controlP5.Controller : Slider addListener(ControlListener) 
controlP5.Controller : Slider addListenerFor(int, CallbackListener) 
controlP5.Controller : Slider align(int, int, int, int) 
controlP5.Controller : Slider bringToFront() 
controlP5.Controller : Slider bringToFront(ControllerInterface) 
controlP5.Controller : Slider hide() 
controlP5.Controller : Slider linebreak() 
controlP5.Controller : Slider listen(boolean) 
controlP5.Controller : Slider lock() 
controlP5.Controller : Slider onChange(CallbackListener) 
controlP5.Controller : Slider onClick(CallbackListener) 
controlP5.Controller : Slider onDoublePress(CallbackListener) 
controlP5.Controller : Slider onDrag(CallbackListener) 
controlP5.Controller : Slider onDraw(ControllerView) 
controlP5.Controller : Slider onEndDrag(CallbackListener) 
controlP5.Controller : Slider onEnter(CallbackListener) 
controlP5.Controller : Slider onLeave(CallbackListener) 
controlP5.Controller : Slider onMove(CallbackListener) 
controlP5.Controller : Slider onPress(CallbackListener) 
controlP5.Controller : Slider onRelease(CallbackListener) 
controlP5.Controller : Slider onReleaseOutside(CallbackListener) 
controlP5.Controller : Slider onStartDrag(CallbackListener) 
controlP5.Controller : Slider onWheel(CallbackListener) 
controlP5.Controller : Slider plugTo(Object) 
controlP5.Controller : Slider plugTo(Object, String) 
controlP5.Controller : Slider plugTo(Object[]) 
controlP5.Controller : Slider plugTo(Object[], String) 
controlP5.Controller : Slider registerProperty(String) 
controlP5.Controller : Slider registerProperty(String, String) 
controlP5.Controller : Slider registerTooltip(String) 
controlP5.Controller : Slider removeBehavior() 
controlP5.Controller : Slider removeCallback() 
controlP5.Controller : Slider removeCallback(CallbackListener) 
controlP5.Controller : Slider removeListener(ControlListener) 
controlP5.Controller : Slider removeListenerFor(int, CallbackListener) 
controlP5.Controller : Slider removeListenersFor(int) 
controlP5.Controller : Slider removeProperty(String) 
controlP5.Controller : Slider removeProperty(String, String) 
controlP5.Controller : Slider setArrayValue(float[]) 
controlP5.Controller : Slider setArrayValue(int, float) 
controlP5.Controller : Slider setBehavior(ControlBehavior) 
controlP5.Controller : Slider setBroadcast(boolean) 
controlP5.Controller : Slider setCaptionLabel(String) 
controlP5.Controller : Slider setColor(CColor) 
controlP5.Controller : Slider setColorActive(int) 
controlP5.Controller : Slider setColorBackground(int) 
controlP5.Controller : Slider setColorCaptionLabel(int) 
controlP5.Controller : Slider setColorForeground(int) 
controlP5.Controller : Slider setColorLabel(int) 
controlP5.Controller : Slider setColorValue(int) 
controlP5.Controller : Slider setColorValueLabel(int) 
controlP5.Controller : Slider setDecimalPrecision(int) 
controlP5.Controller : Slider setDefaultValue(float) 
controlP5.Controller : Slider setHeight(int) 
controlP5.Controller : Slider setId(int) 
controlP5.Controller : Slider setImage(PImage) 
controlP5.Controller : Slider setImage(PImage, int) 
controlP5.Controller : Slider setImages(PImage, PImage, PImage) 
controlP5.Controller : Slider setImages(PImage, PImage, PImage, PImage) 
controlP5.Controller : Slider setLabel(String) 
controlP5.Controller : Slider setLabelVisible(boolean) 
controlP5.Controller : Slider setLock(boolean) 
controlP5.Controller : Slider setMax(float) 
controlP5.Controller : Slider setMin(float) 
controlP5.Controller : Slider setMouseOver(boolean) 
controlP5.Controller : Slider setMoveable(boolean) 
controlP5.Controller : Slider setPosition(float, float) 
controlP5.Controller : Slider setPosition(float[]) 
controlP5.Controller : Slider setSize(PImage) 
controlP5.Controller : Slider setSize(int, int) 
controlP5.Controller : Slider setStringValue(String) 
controlP5.Controller : Slider setUpdate(boolean) 
controlP5.Controller : Slider setValue(float) 
controlP5.Controller : Slider setValueLabel(String) 
controlP5.Controller : Slider setValueSelf(float) 
controlP5.Controller : Slider setView(ControllerView) 
controlP5.Controller : Slider setVisible(boolean) 
controlP5.Controller : Slider setWidth(int) 
controlP5.Controller : Slider show() 
controlP5.Controller : Slider unlock() 
controlP5.Controller : Slider unplugFrom(Object) 
controlP5.Controller : Slider unplugFrom(Object[]) 
controlP5.Controller : Slider unregisterTooltip() 
controlP5.Controller : Slider update() 
controlP5.Controller : Slider updateSize() 
controlP5.Controller : String getAddress() 
controlP5.Controller : String getInfo() 
controlP5.Controller : String getName() 
controlP5.Controller : String getStringValue() 
controlP5.Controller : String toString() 
controlP5.Controller : Tab getTab() 
controlP5.Controller : boolean isActive() 
controlP5.Controller : boolean isBroadcast() 
controlP5.Controller : boolean isInside() 
controlP5.Controller : boolean isLabelVisible() 
controlP5.Controller : boolean isListening() 
controlP5.Controller : boolean isLock() 
controlP5.Controller : boolean isMouseOver() 
controlP5.Controller : boolean isMousePressed() 
controlP5.Controller : boolean isMoveable() 
controlP5.Controller : boolean isUpdate() 
controlP5.Controller : boolean isVisible() 
controlP5.Controller : float getArrayValue(int) 
controlP5.Controller : float getDefaultValue() 
controlP5.Controller : float getMax() 
controlP5.Controller : float getMin() 
controlP5.Controller : float getValue() 
controlP5.Controller : float[] getAbsolutePosition() 
controlP5.Controller : float[] getArrayValue() 
controlP5.Controller : float[] getPosition() 
controlP5.Controller : int getDecimalPrecision() 
controlP5.Controller : int getHeight() 
controlP5.Controller : int getId() 
controlP5.Controller : int getWidth() 
controlP5.Controller : int listenerSize() 
controlP5.Controller : void remove() 
controlP5.Controller : void setView(ControllerView, int) 
controlP5.Slider : ArrayList getTickMarks() 
controlP5.Slider : Slider setColorTickMark(int) 
controlP5.Slider : Slider setHandleSize(int) 
controlP5.Slider : Slider setHeight(int) 
controlP5.Slider : Slider setMax(float) 
controlP5.Slider : Slider setMin(float) 
controlP5.Slider : Slider setNumberOfTickMarks(int) 
controlP5.Slider : Slider setRange(float, float) 
controlP5.Slider : Slider setScrollSensitivity(float) 
controlP5.Slider : Slider setSize(int, int) 
controlP5.Slider : Slider setSliderMode(int) 
controlP5.Slider : Slider setTriggerEvent(int) 
controlP5.Slider : Slider setValue(float) 
controlP5.Slider : Slider setWidth(int) 
controlP5.Slider : Slider showTickMarks(boolean) 
controlP5.Slider : Slider shuffle() 
controlP5.Slider : Slider snapToTickMarks(boolean) 
controlP5.Slider : Slider update() 
controlP5.Slider : TickMark getTickMark(int) 
controlP5.Slider : float getMax() 
controlP5.Slider : float getMin() 
controlP5.Slider : float getValue() 
controlP5.Slider : float getValuePosition() 
controlP5.Slider : int getDirection() 
controlP5.Slider : int getHandleSize() 
controlP5.Slider : int getNumberOfTickMarks() 
controlP5.Slider : int getSliderMode() 
controlP5.Slider : int getTriggerEvent() 
java.lang.Object : String toString() 
java.lang.Object : boolean equals(Object) 

created: 2015/03/24 12:25:46

*/