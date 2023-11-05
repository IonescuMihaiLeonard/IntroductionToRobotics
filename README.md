## Commponents
- 1  7-segment display
- 1  joystick
- Resistors and wires as needed

## Requirement
The  initial  position  should  be  on  the  DP.  The  currentposition always blinks (irrespective of the fact that the segment is on oroff).  Use the joystick to move from one position to neighbors (see table forcorresponding movement).  Short pressing the button toggles the segmentstate  from  ON  to  OFF  or  from  OFF  to  ON.  Long  pressing  the  buttonresets the entire display by turning all the segments OFF and moving thecurrent position to the decimal point.Interrupts are required for CTIand bonus for Computer Science.
### Be careful:
1.  Joystick movements should be done with toggle, as in the lab (joy-Moved, etc)
2.  For CTI, it is part of requirement to use interrupts.  For ComputerScience, it is a bonus.
3.  The  code  in  the  lab  for  joystick  movement  is  not  perfect  and  canbe  improved.   The  code  should  be  wrapped  in  a  function  and  theconstraint between 0 and 9 can be simplified.


| Current Segment | UP | DOWN | LEFT | RIGHT |
| --------------- | -- | ---- | ---- | ----- |
| A | N/A | G | F | B |
| B | A | G | F | N/A |
| C | G | D | E | DP |
| D | G | N/A | E | C |
| E | G | D | N/A | C |
| F | A | G | N/A | B |
| G | A | D | N/A | N/A |
| DP | N/A | N/A | C | N/A |
## Tasks
### Technical Task

## Setup
![first photo](1.jpg)
![second photo](2.jpg)
![third photo](3.jpg)



## YouTube Link
  [Video](https://youtu.be/xDJeSaS9Vx0?si=Z8Rpvtngq318q_VQ)https://youtu.be/xDJeSaS9Vx0?si=Z8Rpvtngq318q_VQ
