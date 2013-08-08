/* uselessBox v1 */

/* This is standard */
#include <Servo.h>

/* If you have not already done so, include the Bounce library in your Arduino
/* program. It is here : http://playground.arduino.cc/code/bounce */
#include <Bounce.h>

/* Depending on the type of servo you're using, these values might change.
/* A servo operates between 0° and 180°, but generally speaking these values 
/* are not respected. 5° to 180° is rather correct in many situations.
/* The aim here is to define the minimum angular position of the servo and the maximum
/* angular position of the servo.
/* Depending on the direction of mouvement, these two values will match the "switch" position
/* and the "home" position */
#define MINARMDEG 17
#define MAXARMDEG 170

/* Our software relies on mecanical equipment that suffer from different phenomena such as bounce.
/* To account for that, we need to define two parameters :
/* - the delay between the arm hitting the switcha and the switch reporting a change in value
/* - the number of msec (approx.) for the servo to go one degree further
/* These parameters should not be modified if you use standard servo and switch. */
#define MECANIC_SPEED_PER_DEGREE 2 // 2 msec to move 1 degree
#define MECANIC_DELAY_SWITCH 50 // 50 msec to acknowledge a hit

/* You have to define here the number of different behaviours that you have
/* coded for our little friend */
#define NUMBEHAVIORS 7

/* We define some positions for our arm */
#define POS_HOME MAXARMDEG
#define POS_SWITCH MINARMDEG
#define POS_NEAR (MINARMDEG + 20)
#define POS_CHECK_VIS (MAXARMDEG - 40)
#define POS_CHECK_NINJA (MAXARMDEG - 30)

// The led indicates that the arm is moving
const int led = 6; // It's the pin

// Servo Motor
Servo arm;
const int servo = 3; // It's the pin

// Servo utilities
int current_speed = 0; // Set the current speed for the current movement. 0 = full power, X (msec) = wait for X between each degree change
int last_write = POS_HOME; // Stores the last command passed to the motor so we know where do we come from and where we go
boolean is_home = true; // By default, we're home (we ensure this in the setup() )

// Switch
const int box_switch = 8; // It's the pin
Bounce bouncer = Bounce(box_switch, 20); // Let's debounce this mecanical part, yeah.

// Is the switch activated ?
int activated = LOW; // By default, no. But we'll check that soon enough in the loop() anyway.

int randNumber = 1; // This is used to choose the next behaviour 
long randCheck = 1; // Random check when not activated
boolean hasAlreadyChecked = false;
boolean mayday = false; // Is set to true when the switch has been changed while doing something. Allows for a quick check in the loop() for what we should do next

/* ----- */
/* SETUP */
/* ----- */
void setup() {

  // Initialize the led pin as an output.
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW); // ... and shuts it down.

  // Initialize the switch pin as an input 
  pinMode(box_switch, INPUT);

  // We start at home.
  move_arm(POS_HOME);
  arm.attach(servo);

  // Debug
  Serial.begin(9600);
  Serial.println("Started.");
}

/* ------------------------------- */
/* ------ HELPER FUNCTIONS ------- */
/* ------------------------------- */

// Moves the arm with the specified speed
// (This function can be interrupted by manual switch change)
void move_arm(int degree) {

  Serial.print("Moving arm to position : ");
  Serial.print(degree);
  Serial.println("deg.");
  
  // Update the switch value.
  bouncer.update();
  int val = bouncer.read();

  // Check the last command we gave to the servo
  int current_degree = arm.read();
  last_write = current_degree;

  // And then moves ! degree by degree, checking the switch position each time
  while (current_degree != degree) {
    if (current_degree < degree) {
      current_degree++;
    }
    else {
      current_degree--;
    }

    arm.write(current_degree);
    delay(MECANIC_SPEED_PER_DEGREE + current_speed);

    bouncer.update();
    int current_value = bouncer.read();

    if (current_value != val) {
      Serial.println("/!\\ Mayday - The switch was operated while I was moving !");
      mayday = true;
      break;
    }
  }

}

// A "soft" delay function
// (This function can be interrupted by manual switch change)
void soft_delay(int msec) {

  Serial.print("Delaying for ");
  Serial.print(msec);
  Serial.println(" msec...");
  
  // Update the switch value.
  bouncer.update();
  int val = bouncer.read();

  // Inits a counters
  long time_counter = 0;

  // And wait... msec by msec, checking the switch position each time
  do {
    delay(1);
    time_counter++;

    bouncer.update();
    int current_value = bouncer.read();

    if (current_value != val) {
     Serial.println("/!\\ Mayday - The switch was operated while I was waiting on purpose !");
      mayday = true;
      break;
    }
  } 
  while(time_counter <= msec);

}


/* ------------------------------- */
/* ---- STANDARD BEHAVIOURS ------ */
/* ------------------------------- */

// Go out and flip that switch the user just
void goFlipThatSwitch(int msec = 0) { 
  current_speed = msec; // MAXIMUM POWWAAAAAA 
  move_arm(POS_SWITCH); // Go to the switch
  delay(MECANIC_DELAY_SWITCH); // Wait for the switch to acknowledge before returning in the loop
}

// Ok, back home now
void backHome() { 
  current_speed = 0; 
  move_arm(POS_HOME); 
}


/* ------------------------------- */
/* ------------------------------- */
/* ------------------------------- */

// Go near the arm, but not as close as to push the switch, and then retracts a bit
void tryFail(int msec = 5) {  
  current_speed = msec; 
  move_arm(POS_NEAR);
  move_arm(POS_NEAR + 7); 
}
// Open the lid to see out
void goCheck(int msec = 10) {  
  current_speed = msec; 
  move_arm(POS_CHECK_VIS); 
}
// Open the lid stealthly. User does not see me. I'm a NINJA !
void goStealthCheck() {  
  current_speed = 30; 
  move_arm(POS_CHECK_NINJA); 
}

// Check once, wait, then flip the switch
void check() {
  if (!mayday) goCheck();
  if (!mayday) soft_delay(1000);
  if (!mayday) goFlipThatSwitch();
}

// Check, return back home, then flip
void checkReturn() {
  if (!mayday) goCheck();
  if (!mayday) soft_delay(1500);
  if (!mayday) backHome();
  if (!mayday) soft_delay(800);
  if (!mayday) goFlipThatSwitch();
}

// Check once slowly, return, check again, return to stealth position, then flip
void checkCheckReturn() {
  if (!mayday) goStealthCheck();
  if (!mayday) soft_delay(1500);
  if (!mayday) backHome();
  if (!mayday) goCheck();
  if (!mayday) soft_delay(1000);
  if (!mayday) goStealthCheck();
  if (!mayday) soft_delay(800);
  if (!mayday) goFlipThatSwitch();
}

// Multi tries. At the end, succeeds...
void multiTry() {
  if (!mayday) tryFail();
  if (!mayday) soft_delay(500);
  if (!mayday) tryFail();
  if (!mayday) soft_delay(500);
  if (!mayday) tryFail();
  if (!mayday) soft_delay(500);
  if (!mayday) backHome();
  if (!mayday) soft_delay(500);
  if (!mayday) goFlipThatSwitch();
}

// I'm afraid ...
void afraid() {
  if (!mayday) tryFail(0);
  if (!mayday) goCheck(0);
  if (!mayday) soft_delay(1500);
  if (!mayday) goFlipThatSwitch(5);
}

// #OhWait 
void ohWait() {
  if (!mayday) tryFail(0);
  if (!mayday) backHome();
  if (!mayday) soft_delay(700);
  if (!mayday) goCheck(2); // Woops. Forgot something ?
  if (!mayday) soft_delay(1000);
  if (!mayday) goFlipThatSwitch(15);
}


/* ----- */
/* LOOP! */
/* ----- */
void loop() {

  // Update the switch position
  bouncer.update();
  activated = bouncer.read();

  // If the user wants me to go out
  if (activated == HIGH) {

    Serial.println("Going out ! Yippee ! ");
    digitalWrite(led, HIGH);
    is_home = false; // We're just leaving !
    arm.attach(servo);

    switch(randNumber) { // What animation shall I do today ?
      case 1: // Standard
        goFlipThatSwitch();
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
      case 7:
        ohWait();
        break;
      default:
        goFlipThatSwitch();
        break;
    }

    mayday = false; // Now that I have finished, I can rest.
    hasAlreadyChecked = false; // We can check once after that

  } else if (is_home == false) {

    // If we're not home yet, we shall go there !
    Serial.println("Going back home");
    backHome();
    digitalWrite(led, LOW);
    
  } else {
    
    if (randCheck < 5 && hasAlreadyChecked == false) {
      
      Serial.println("Random check, baby. ");
      arm.attach(servo);
      is_home == false;
      // We only check once after an activation
      if (!mayday) goCheck();
      if (!mayday) soft_delay(1500);
      if (!mayday) backHome();
      //hasAlreadyChecked = true;
      
    }
  
  }

  // if we're home, then we should "power off" the servo (the command, more specifically)
  if (arm.read() == POS_HOME && is_home == false) {
    Serial.println("Powering off the servo ...");
    int time_to_wait = abs(last_write-POS_HOME)*(current_speed + MECANIC_SPEED_PER_DEGREE);
    delay(time_to_wait);
    is_home = true;
    arm.detach();
  }

  // Find a new behaviour for next time
  randNumber = random(1, NUMBEHAVIORS);
  
  // Random number to check sometimes
  randCheck = abs(random(1, 1000000));

}

