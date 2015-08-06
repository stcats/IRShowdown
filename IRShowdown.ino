#include <Gamer.h>
#include <GamerIR.h>
#include <SoftwareSerial.h>

#define G  254
#define Ab 240
#define A  226
#define Bb 213
#define B  201
#define C  190
#define Db 179

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
    playDefeat();
    restart();

    return;
  }

  if (gamer.isPressed(UP)) {
    transmit("B"); //B for "Bang!"
    
    playWin();
    restart();
  }
  
  checkForRestart();
}

void playMorricone() {
  gamer.playTone(Ab);
  delay(100);
  gamer.playTone(Db);
  delay(100);
  gamer.playTone(Ab);
  delay(100);
  gamer.playTone(Db);
  delay(100);
  gamer.playTone(Ab);
  delay(400);
  gamer.stopTone();
}

void playMario() {
  gamer.playTone(Bb);
  delay(100);
  gamer.playTone(Db);
  delay(400);
  gamer.stopTone();
}

void playChopin() {
  gamer.playTone(G);
  delay(400);
  gamer.stopTone();
  delay(100);
  
  gamer.playTone(G);
  delay(300);
  gamer.stopTone();
  delay(100);
  
  gamer.playTone(G);
  delay(50);
  gamer.stopTone();
  delay(50);
  
  gamer.playTone(G);
  delay(400);
  gamer.stopTone();
  delay(100);
  
  gamer.playTone(Bb);
  delay(400);
  gamer.stopTone();
  delay(100);

  gamer.playTone(A);
  delay(50);
  gamer.stopTone();
  delay(50);

  gamer.playTone(A);
  delay(300);
  gamer.stopTone();
  delay(100);

  gamer.playTone(G);
  delay(50);
  gamer.stopTone();
  delay(50);

  gamer.playTone(G);
  delay(300);
  gamer.stopTone();
  delay(100);

  gamer.playTone(G);
  delay(50);
  gamer.stopTone();
  delay(50);

  gamer.playTone(G);
  delay(400);
  
  gamer.stopTone();
}

void checkForRestart() {
  if (gamer.isPressed(START)) {
    restart();
  }
}

void restart() {
  mode = 0;
}

void playDefeat() {
  byte defeat[21][8] = {
    {
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B11100111,
      B11100111,
      B11111111,
      B11111111,
      B11111111
    },
    {
      B11111111,
      B11111111,
      B11100111,
      B11000011,
      B11000011,
      B11100111,
      B11111111,
      B11111111
    },
    {
      B11111111,
      B11100111,
      B11000011,
      B10000001,
      B10000001,
      B11000011,
      B11100111,
      B11111111
    },
    {
      B11111111,
      B11000011,
      B10000001,
      B10000001,
      B10000001,
      B10000001,
      B11000011,
      B11111111
    },
    {
      B11100111,
      B11000011,
      B10000001,
      B00000000,
      B00000000,
      B10000001,
      B11000011,
      B11100111
    },
    {
      B11000011,
      B10000001,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B10000001,
      B11000011
    },
    {
      B10000001,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B10000001
    },
    {
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000
    },
    {
      B00100000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000
    },
    {
      B00100100,
      B00100000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000
    },
    {
      B00110100,
      B00100100,
      B00100000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000
    },
    {
      B01111110,
      B00110100,
      B00100100,
      B00100000,
      B00000000,
      B00000000,
      B00000000,
      B00000000
    },
    {
      B11111111,
      B01111110,
      B00110100,
      B00100100,
      B00100000,
      B00000000,
      B00000000,
      B00000000
    },
    {
      B11111111,
      B11111111,
      B01111110,
      B00110100,
      B00100100,
      B00100000,
      B00000000,
      B00000000
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B01111110,
      B00110100,
      B00100100,
      B00100000,
      B00000000
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B01111110,
      B00110100,
      B00100100,
      B00100000
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B01111110,
      B00110100,
      B00100100
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B01111110,
      B00110100
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B01111110
    },
    {
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111,
      B11111111
    }
  };
 
  for (int i = 0; i < 21; i++) {
    gamer.printImage(defeat[i]);
    delay(100);
  } 
}

void playWin() {
  byte frames[30][8] = {
    {
      B00000000,
      B00001111,
      B00010100,
      B00010100,
      B00010100,
      B00010100,
      B00001111,
      B00000000
    },
    {
      B00000000,
      B00000111,
      B00001010,
      B00001010,
      B00001010,
      B00001010,
      B00000111,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00000101,
      B00000101,
      B00000101,
      B00000101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00000101,
      B00000111,
      B00000111,
      B00000101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00000111,
      B00000101,
      B00000101,
      B00000111,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00000111,
      B00001001,
      B00001001,
      B00000111,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00001111,
      B00010001,
      B00010001,
      B00001111,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00011111,
      B00100011,
      B00100011,
      B00011111,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00111111,
      B01000101,
      B01000101,
      B00111111,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B01111111,
      B10001011,
      B10001011,
      B01111111,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B11111101,
      B00010101,
      B00010101,
      B11111101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B11111101,
      B00101101,
      B00101101,
      B11111101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B11110101,
      B01010101,
      B01010101,
      B11110101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B11100101,
      B10100101,
      B10100101,
      B11100101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B11000101,
      B01000101,
      B01000101,
      B11000101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B10000101,
      B10000101,
      B10000101,
      B10000101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000011,
      B00000101,
      B00000101,
      B00000101,
      B00000101,
      B00000011,
      B00000000
    },
    {
      B00000000,
      B00000001,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00000011,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00000110,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00001100,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00011000,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00110000,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B01100000,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B11000000,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B10000000,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00000000,
      B00000000,
      B00100000,
      B01111110,
      B01111110,
      B00110000,
      B01110000
    },
    {
      B00000000,
      B00000000,
      B00000000,
      B00100000,
      B01010110,
      B01101010,
      B00010000,
      B01110000
    },
    {
      B00000000,
      B00000000,
      B00000000,
      B00100000,
      B01010100,
      B00101010,
      B00010000,
      B00100000
    },
    {
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00000000
    }
  };

  for (int i = 0; i < 30; i++) {
    gamer.printImage(frames[i]);
    delay(80);
  }
}
