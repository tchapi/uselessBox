/* Inspired by http://v3-1255.vlinux.de/mediawiki/index.php/Uselessbox */

#include <Servo.h>
#include <Bounce.h>
  
#define MINARMDEG 5
#define MAXARMDEG 95

#define MINWAIT 0 // msec
#define NUMBEHAVIORS 6

#define POS_HOME 5 // min position for servo, might change from servo to servo
#define POS_SWITCH 95
#define POS_NEAR 88
#define POS_CHECK_VIS 45
#define POS_CHECK_INVIS 30

Servo arm;

// Led indicates arm is moving
const int led = 6;

// Servo Motor
const int servo = 3;

// Switch
const int box_switch = 8;
Bounce bouncer = Bounce(box_switch, 10);

// Is the switch activated ?
int activated = LOW;
int randNumber = 1;

// the setup routine runs once when you press reset:
void setup() {

  // Initialize the led pin as an output.
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  
  // Switch pin as input 
  pinMode(box_switch, INPUT);

  // Servo
  arm.write(POS_HOME); // Home
  arm.attach(servo);

}

// Standard behaviour
void goOut() { move_arm(POS_SWITCH,5); }
void backHome() { move_arm(POS_HOME,5); }

// Helper functions
void tryFail() {  move_arm(POS_NEAR,10); move_arm(POS_NEAR - 10,10); }
void goCheck() { move_arm(POS_CHECK_VIS,20); }
void goCheckSlow() { move_arm(POS_CHECK_VIS,20); }
void goStealthCheck() { move_arm(POS_CHECK_INVIS,30); }

// Check once
void check() {
  goCheck();
  soft_delay(800);
  goOut();
}

// Check, return
void checkReturn() {
  goCheck();
  soft_delay(1500);
  backHome();
  soft_delay(800);
  goOut();
}

// Check, check, return
void checkCheckReturn() {
  goStealthCheck();
  soft_delay(2000);
  backHome();
  goCheck();
  soft_delay(1000);
  goStealthCheck();
  soft_delay(800);
  goOut();
}

// Multi try
void multiTry() {
  tryFail();
  tryFail();
  tryFail();
  backHome();
  soft_delay(500);
  goOut();
}

// Afraid
void afraid() {
  soft_delay(1000);
  goCheckSlow();
  soft_delay(1500);
  goOut();
}


// the loop routine 
void loop() {

  bouncer.update();

  activated = bouncer.read(); // get box switch position

  if (activated == HIGH) {
    
    digitalWrite(led, HIGH);
    arm.attach(servo);

    goOut();
    /*
    switch(randNumber) { // opening animation
      case 1: // Standard
        goOut();
        break;
      case 2:
        check();
        break;
      case 3:
        multiTry();
        break;
      case 4:
        checkReturn();
        break;
      case 5:
        checkCheckReturn();
        break;
      case 6:
        afraid();
        break;
    }
    */

  } else {

    digitalWrite(led, LOW);
    backHome();
  }

  // Power off servo
  if (arm.read() == MINARMDEG) {
    arm.detach();
  }

  randNumber = random(1, NUMBEHAVIORS);

}


// move arm to position within the specified time
void move_arm(int degree, int msec) {

  int val = digitalRead(box_switch);
  int current_degree = arm.read();

  while (current_degree != degree) {
    if (current_degree < degree) {
      current_degree++;
    }
    else {
      current_degree--;
    }
    
    arm.write(current_degree);
    delay(MINWAIT + msec);

    int current_value = digitalRead(box_switch);
    if (current_value != val) {
      break;
    }
  }

}

// delay (can be interrupted by manual switch change)
void soft_delay(int msec) {
  
  int val = digitalRead(box_switch);
  long time_counter = 0;

  do {
    delay(1);
    time_counter++;
    int current_value = digitalRead(box_switch);
    if (current_value != val) {
      break;
    }
  } 
  while(time_counter <= msec);

}

