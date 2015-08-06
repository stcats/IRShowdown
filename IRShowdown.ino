#include <Gamer.h>
#include <GamerIR.h>
#include <SoftwareSerial.h>

Gamer gamer;
GamerIR infrared;

/* The aim of this game is to shoot before getting shot at, but the trick
 is that the time to wait before being able to shoot (the time spent
 during the countdown) is unpredictable.

 The random value used in the countdown is stored in this variable.
 */
int delayBeforeShooting;

/* The game changes between different states, and the "mode" variable
 is used to store the current active state.
 
 The possible game states are:
 
     0 = in this state the game displays an image (the "splash screen")
         and waits for the player to press START
     1 = the game tries to connect to other players (others that have
         also pressed START)
     2 = the countdown begins and, as soon as a shot is fired the result
         is displayed

 This instruction adds one to the "mode" variable, and therefore advances
 the game to the next stage:

     mode++;

 */
int mode;

/* Images.
 */
byte cowboy[8] = {
  B00011000,
  B01111110,
  B00111100,
  B00011000,
  B01111110,
  B00011000,
  B01111110,
  B11000011
};

byte go[8] = {
  B11100111,
  B10100101,
  B10000101,
  B10000101,
  B10110101,
  B10100101,
  B10100101,
  B11100111
};

void setup() {
  gamer.begin();
  randomSeed(gamer.ldrValue());
}

void loop() {
  switch (mode) {
    case 0:
      introduction();
      break;

    case 1:
      synchronizeGamers();
      break;

    case 2:
      countdown();
      break;

    case 3:
      detectShot();
      break;
  }
}

void introduction() {
  gamer.printImage(cowboy);

  if (gamer.isPressed(START)) {
    delayBeforeShooting = 0; //reset
    mode++;
  }
}

/* Both DIY Gamer Kits in play need to agree on what random wait time
 to use. One way to solve this is for one of the DIY Gamer Kits to take
 charge always and broadcast the wait time to use.

 But that would mean having two different programs: the "master" program
 and the "slave" program.

 The alternative used in this program is for each kit to attempt to take
 charge randomly.

 So in "broadcastReceived()" both kits listen for the other. But because
 that function runs for a random amount of time, the first kit to finish
 will take charge and broadcast the value to the other kit (it's very
 unlikely that they both finish listening at the same time).

 Described in a different way:

     1. Kits "A" and "B" join the game
     2. Both kits begin listening for a broadcast from the other
     3. Randomly, one of the kits stops listening (e.g., "A") and it
        broadcasts the wait time to the other kit ("B")
     4. Now that both "A" and "B" have the same wait time, the countdown
        can begin

 Can this algorithm accommodate more than two DIY Gamer Kits joining the
 game at the same time?
 */
void synchronizeGamers() {
  waitForOtherPlayersToJoin();

  if (!broadcastReceived()) {
    broadcastMessage();
  }
  
  mode++;
}

void waitForOtherPlayersToJoin() {
  delay(100);
}

boolean broadcastReceived() {
  /* We would like to wait for a while in case other DIY Gamer Kits
   decide to broadcast before us. But using "delay()" will not do because
   we need to actively listen whilst letting the time pass.

   The solution is to loop over and over until "timeToListen" is greater
   than the time now ("millis()"), and the time when the loop first started
   running ("timeWhenListenBegan").
   */
  int timeToListen = random(100, 1000);
  unsigned long timeWhenListenBegan = millis();

  while (millis() < (timeWhenListenBegan + timeToListen)) {
    String data = infrared.receive();

    if (data.length() > 0) {
      delayBeforeShooting = data.toInt();

      if (delayBeforeShooting > 0) { //have we received a valid number?
        return true;
      }
    }
  }

  return false;
}

void broadcastMessage() {
  /* A larger range will reduce the reliability because the transmission will
   require two characters.
   */
  delayBeforeShooting = random(1, 9);

  transmit(delayBeforeShooting);
}

void transmit(int number) {
  transmit((String) number);
}

void transmit(String string) {
  gamer.setLED(1);
  infrared.send(string);
  gamer.setLED(0);
}

void countdown() {
  gamer.printString("Ready");
  gamer.printString("Set");
  delay(delayBeforeShooting * 500);
  infrared.receive(); //clear any buffered values before detecting shot
  gamer.printImage(go);

  mode++;
}

/* This function runs over and over (it's called by "loop()"). So in
 each instant it first checks to see if the other player has pressed
 the "fire" button or whether this player has pressed the button.

 There is a chance that bother players will press the "fire" button at
 the same time. In that case, they're both given the win and can call it
 a draw.
 */
void detectShot() {
  String data = infrared.receive();
  if (data.length() > 0 && data.equals("B")) {
    gamer.printString("You lose");
    restart();

    return;
  }

  if (gamer.isPressed(UP)) {
    transmit("B"); //B for "Bang!"
    
    gamer.printString("You win");  
    restart();
  }
  
  checkForRestart();
}

void checkForRestart() {
  if (gamer.isPressed(START)) {
    restart();
  }
}

void restart() {
  mode = 0;
}
