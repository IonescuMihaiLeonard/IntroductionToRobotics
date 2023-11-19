# RGB LED Controller

## Commponents
- RGB LED 
- ThermoResistor
- Ultrasonic Sensor
- Resistors and wires as needed

## Objective
Develop a ”Smart Environment Monitor and Logger” using Arduino.  This sys-tem will utilize various sensors to gather environmental data, log this data intoEEPROM, and provide both visual feedback via an RGB LED and user interac-tion through a Serial Menu.  The project focuses on integrating sensor readings,memory management, Serial Communication and the general objective of build-ing a menu.See the partial example video (only of menu parsing) here:https://www.youtube.com/watch?v=mh0KYdul1Sk

## Menu Structure

Menu Structure
1. Sensor Settings// Go to submenu1.
  - 1.1. Sensors Sampling Interval.  Here you should be prompted fora value between 1 and 10 seconds.  Use this value as a sampling ratefor the sensors.  You can read a separate value for each or have thesame for both.
  - 1.2. Ultrasonic Alert Threshold.  Here you should be promptedfor a threshold value for the ultrasonic sensor.  You can decide if thatis the min or max value (you can signal that something is too close).When sensor value exceeds the threshold value,  an alert should begiven.  This can be in the form of a message.  If the LED is set toAutomatic Mode (see section 4.2), it should also turn red if any ofthe sensors are outside the value.
  - 1.3. LDR Alert Threshold.  Here you should be prompted for athreshold  value  for  the  LDR  sensor.   You  can  decide  if  that  is  themin or max value (for example, it could signal that night is coming).When sensor value exceeds the threshold value,  an alert should begiven.  This can be in the form of a message.  If the LED is set toAutomatic Mode (see section 4.2), it should also turn red if any ofthe sensors are outside the value.
  - 1.4. Back// Return to main menu2.
2. Reset Logger Data.Should  print  a  message,  promting  if  you  toconfirm to delete all  data.  Something like ”are you sure?”,  followed bythe submenu with YES or NO. You can reset both sensor data at the sametime, or you can do it individually.  Your choice.  Individually makes moresense, but I’m trying to simplify the homework.
  - 2.1. Yes.
  - 2.2. No.
3. System Status// Check current status and health
  - 3.1. Current Sensor Readings.Continuously print sensor readingsat the set sampling rate, from all sensors.  Make sure you have a wayto exit this (such as pressing a specific key) and inform the user ofthis method through a message.
  - 3.2. Current Sensor Settings.Displays  the  sampling  rate  andthreshold value for all sensors.
  - 3.3. Display Logged Data.Displays last 10 sensor readings for allsensors.  (or be creative and do it another way).
  - 3.4. Back.Return to Main menu.
4. RGB LED Control// Go to submenu
  - 4.1 Manual Color Control.  Set the RGB colors manually.  Youdecide how to input them, either by making an option for each chan-nel, or by putting a string etc.  If you expect a specific format, makesure to inform the user.
  - 4.2 LED: Toggle Automatic ON/OFF.  If  automatic  mode  isON, then the led color should be GREEN when all sensors value donot exceed threshold values (aka no alert) and RED when there is analert (aka ANY sensor value exceeds the threshold). When automaticmode is OFF, then the LED should use the last saved RGB values.
  - 4.3 Back// Return to main menu
   
## Setup
![first photo](1.jpg)
![second photo](2.jpg)
![third photo](3.jpg)



## YouTube Link
https://youtu.be/KXYdTyPgNpM
