
// Include Gamer library, as well as the infrared library.
#include <Gamer.h>
#include <GamerIR.h>
#include <SoftwareSerial.h>

// Create a copy of the Gamer library.
Gamer gamer;
// Create a copy of the Gamer IR library.
GamerIR infrared;

// General
int score;
boolean isPlaying;
int delayBeforeShooting;

/*
    The mode variable keeps track of the mode that the game is in.
 
 0 = waiting for other Gamer.
 1 = countdown.
 2 = waiting for shot.
 3 = shot fired.
 */
int mode;

// Splash screen image
byte splashScreen[8] = {
  B00011000,
  B01111110,
  B00111100,
  B00011000,
  B01111110,
  B00011000,
  B01111110,
  B11000011
};

// Go image
byte go[8] = {
  B11100111,
  B10100101,
  B10000101,
  B10000101,
  B10110101,
  B10100101,
  B11100111,
  B00000000
};

void setup() {
  gamer.begin();
}

void loop() {
  if (isPlaying) {
    switch (mode) {
    case 0:
      // Waiting for other Gamer
      waitForOtherGamer();
      break;

    case 1:
      countdown();
      break;

    case 2:
      //detectShot();
      break;

    case 3:
      break;
    }
  }
  else {
    showSplashScreen();
  }
}

void waitForOtherGamer() {
  String temp = infrared.receive();
  if(temp > 0) {
    // Other Gamer has pressed start before this one. 
    delayBeforeShooting = temp * 20;
    mode++;
  } 
  else {
    // This is the first Gamer. Send random byte. 
    infrared.send(byte(random(100, 254)));
  }
}

void countdown() {
  gamer.printString("READY");
  delay(1000);
  gamer.printString("SET");
  delay(delayBeforeShooting);
  gamer.printImage(go);

  mode = mode + 1;
}

/* ---------------------------------------------------------------
 Shows an image when the game isn't being played.
 */
void showSplashScreen() {
  if (gamer.isPressed(START)) {
    isPlaying = true;
    // Start game!
  }
  else {
    gamer.printImage(splashScreen);
  }
}


