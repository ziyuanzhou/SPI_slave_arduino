#include <SPI.h>
#include <Servo.h>

// Since we have 6 joints and each joint number is the pwm
// for every servo motor. Therefore, each joint angles is 
// between 500 and 2500. So we need 2 bytes for each joint angle.
// Hence we need 6*2 bytes for all 6 joint states.
// At the same time, we use jointPWM to store the real PWM numbers
// for servo motors, which we will use to drive the motors.
const int numJoints = 6;
volatile byte jointStates[numJoints*2];
volatile short jointPWM[numJoints];
// this is the index for jointStates array
volatile byte index;
// We use two NULL bytes to indicate the beginning of a jointStates array
// sent from ROS through SPI. Once we receive two NULL bytes, we tell Arduino
// to start storing bytes into jointStates array
volatile boolean receive_one_zero;
volatile boolean receive_two_zero;

Servo armServo[6];
////// TO DO: correct order of arm /////////
int servoPins[6] = {2, 3, 4, 5, 6, 7}; // 2, 3, 4, 5, 6, 7

void setup (void) {
   // Initialize USB serial communication for debugging
   Serial.begin(9600);
   // Configure SPI
   pinMode(MISO, OUTPUT); // have to send on master in so it set as output
   SPCR |= _BV(SPE); // turn on SPI in slave mode
   SPI.attachInterrupt(); // turn on interrupt
   // Put index to 0, put process, receive_one_zero and receive_two_zero
   // to false.
   index = 0; // jointStates empty
   receive_one_zero = false;
   receive_two_zero = false;
   ///// To DO /////
   // attach servos
   for (int i = 0; i < numJoints; i++) {
    armServo[i].attach(servoPins[i]);
   }
}

// This the interrupt function for SPI
// Everytime SPI receives a byte, Arduino will jump from the loop()
// to this interrupt function.
ISR (SPI_STC_vect)  { 
//   Serial.println(SPDR);
   byte c = SPDR; // read byte from SPI Data Register
   
   // if we receive a NULL byte and we haven't received a NULL byte before
   // then we turn the receive_one_zero flag to true
   if (c == 0 && receive_one_zero == false) {
    receive_one_zero = true;
   } 
   
   // if we receive a NULL byte and we have already received a NULL byte before
   // then we know we have received two NULL bytes consecutively, so we turn
   // receive_two_zero to true
   else if (c == 0 && receive_one_zero == true) {
    receive_two_zero = true;
   }
   
   // if we have received two NULL bytes consecutivele and the current
   // index is smaller than the size of jointStates (means we are still
   // filling in the jointStates array), then we keep filling in the array
   else if (receive_two_zero == true && index < sizeof jointStates) {
      jointStates[index] = c;
      index++;
   }
   
   // if we have received two zeros, and jointStates array is full, then we
   // reset all the flags, reset index
   // We also combine two bytes to form an integer that indicates the joint PWM
   // The joint PWM is also sent the Servo motors.
   else if (receive_two_zero == true && index == sizeof jointStates){
      for (int i = 0; i < numJoints; i++) {
        jointPWM[i] = 256*jointStates[2*i] + jointStates[2*i + 1];
        Serial.println(jointPWM[i]);
        ////// to do ///////
        // Send jointPWM[i] to the corresponding servo motor
        armServo[i].writeMicroseconds(jointPWM[i]);
      }
      Serial.println("---");
      receive_one_zero = false;
      receive_two_zero = false;
      index = 0;
   }
}

void loop (void) {
  // In main loop we do nothing :)
  // All the processing is done in the interrupt function
}
