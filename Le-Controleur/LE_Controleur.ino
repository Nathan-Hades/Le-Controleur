int x;
int y;
int z;
int potiWert1[16];
//int potiWert1;
int controlChange = 176;
int midiStart = 250;
int midiStop = 252;
int controllerNummer[] = {5,4,3,6,7,10,8,9};
int controllerWert[32];
int controllerWertAlt[32];
int potiWert[32];
byte bit1 = 0;
byte bit2 = 0;
byte bit3 = 0;
int buttonPin[] = {0,1,2,3,4,5,6,7,8,12};
boolean buttonWert[10];
boolean buttonWertAlt[10];
boolean schalter = LOW;
boolean schalterAlt = LOW;
unsigned long schalterTime1 = 0;         // letzter Zeitwert bei dem der Ausgangzustand wechselte.
unsigned long schalterDebounce1 = 100;   // Entprellzeit
unsigned long schalterTime2 = 0;         // letzter Zeitwert bei dem der Ausgangzustand wechselte.
unsigned long schalterDebounce2 = 100;   // Entprellzeit
unsigned long tasterTime[10];         // letzter Zeitwert für Buttons
unsigned long tasterDebounce = 200;   // Entprellzeit für Buttons

void setup() {
	Serial2.begin(31250);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);
	pinMode(4, INPUT_PULLUP);
	pinMode(5, INPUT_PULLUP);
	pinMode(6, INPUT_PULLUP);
	pinMode(7, INPUT_PULLUP);
	pinMode(8, INPUT_PULLUP);
	pinMode(9, INPUT_PULLUP);
	pinMode(12, INPUT_PULLUP);
	pinMode(13, OUTPUT);
	pinMode(14, OUTPUT);
	pinMode(15, OUTPUT);
	for (y = 0; y < 10; y++) {
		buttonWertAlt[y] = LOW;
    tasterTime[y] = 0;
	}
 for (z = 0; z < 8; z++) {
    bit1 = bitRead(z, 0);
    bit2 = bitRead(z, 1);
    bit3 = bitRead(z, 2);

    digitalWrite(13, bit1);
    digitalWrite(14, bit2);
    digitalWrite(15, bit3);
    
    potiWert1[z] = analogRead(A2);
    potiWert1[z+8] = analogRead(A3);
    controllerWert[z] = map(potiWert1[z],0,1010,0,127);
    controllerWert[z+8] = map(potiWert1[z+8],0,1010,0,127);
    controllerWert[z+16] = map(potiWert1[z],0,1010,0,127);
    controllerWert[z+24] = map(potiWert1[z+8],0,1010,0,127);
    
    /*controllerWert[z] = map(analogRead(A2),0,1010,0,127);
    controllerWert[z+8] = map(analogRead(A3),0,1010,0,127);
    controllerWert[z+16] = map(analogRead(A2),0,1010,0,127);
    controllerWert[z+24] = map(analogRead(A3),0,1010,0,127);*/
  }
}

void loop () {
	for ( x = 0 ; x <= 7; x++ ) {
		bit1 = bitRead(x, 0);
		bit2 = bitRead(x, 1);
		bit3 = bitRead(x, 2);

		digitalWrite(13, bit1);
		digitalWrite(14, bit2);
		digitalWrite(15, bit3);
   
		check16(x);
	}
	sendeButton(0);
}

void sendePoti(int zaehler, int messung, int wert, int analogPin) {
  //potiWert1 = analogRead(analogPin);
  potiWert1[messung] = 0.96 * potiWert1[messung] + 0.04 * analogRead(analogPin);
  potiWert[zaehler] = map(potiWert1[messung],0,975,0,127);
  //controllerWert[zaehler] = map(potiWert1[zaehler],0,1010,0,127);
  controllerWert[zaehler] = 0.7 * controllerWert[zaehler] + 0.3 * potiWert[zaehler];
  if (controllerWertAlt[zaehler] != controllerWert[zaehler]) {
    usbMIDI.sendControlChange(wert, controllerWert[zaehler], 1);
    Serial2.write(controlChange);
    Serial2.write(wert);
    Serial2.write(controllerWert[zaehler]);
  }
  controllerWertAlt[zaehler] = controllerWert[zaehler];
  if (zaehler < 16) {
    controllerWert[zaehler+16] = controllerWert[zaehler];
    controllerWertAlt[zaehler+16] = controllerWert[zaehler];
  } else {
    controllerWert[zaehler-16] = controllerWert[zaehler];
    controllerWertAlt[zaehler-16] = controllerWert[zaehler];
  }
}

void sendeButton(int i){
	for (i = 0; i < 10; i++) {
		//buttonWertAlt[i] = LOW;
		buttonWert[i] = digitalRead(buttonPin[i]);
		if (buttonWert[i] == LOW && buttonWertAlt[i] == HIGH && millis() - tasterTime[i] > tasterDebounce) {
      if (i == 1) {
        usbMIDI.sendRealTime(usbMIDI.Start);
        Serial2.write(midiStart);
      } else if (i == 2) {
        usbMIDI.sendRealTime(usbMIDI.Stop);
        Serial2.write(midiStop);
      }
      usbMIDI.sendControlChange(35+i, 127, 1);
      Serial2.write(controlChange);
      Serial2.write(35+i);
      Serial2.write(127);
			buttonWertAlt[i] = buttonWert[i];
		} 
		if(buttonWert[i] == HIGH && buttonWertAlt[i] == LOW) {
			buttonWertAlt[i] = buttonWert[i];
      tasterTime[i] = millis();
		}
	}
}

void check16(int a){
  schalter = digitalRead(9);

  if (schalter == HIGH) {
    if (millis() - schalterTime1 > schalterDebounce1) {
      schalterAlt = HIGH;
    }
    schalterTime2 = millis();
  }
  if (schalter == LOW) {
    if (millis() - schalterTime2 > schalterDebounce2) {
      schalterAlt = LOW;
    }
    schalterTime1 = millis();
  }
  if (schalterAlt == HIGH){
    sendePoti(a,a,controllerNummer[a],A2);
    sendePoti(a+8,a+8,controllerNummer[a]+8,A3);
  } else if (schalterAlt == LOW){
    sendePoti(a+16,a,controllerNummer[a]+16,A2);
    sendePoti(a+24,a+8,controllerNummer[a]+24,A3);
  }
}

// Code-Reste:
/*
firstRead[zaehler] = 0.7 * firstRead[zaehler] + 0.3 * analogRead(analogPin);
  controllerWert[zaehler] = map(firstRead[zaehler],0,924,0,127);
  //controllerWert[zaehler] = map(analogRead(analogPin),0,924,0,127);
  if (firstRead[zaehler] > 127) {
    firstRead[zaehler] = 127;
  }
  if (controllerWertAlt[zaehler] != controllerWert[zaehler]) {
    usbMIDI.sendControlChange(wert, controllerWert[zaehler], 1);
    Serial2.write(controlChange);
    Serial2.write(wert);
    Serial2.write(controllerWert[zaehler]);
  }
  controllerWertAlt[zaehler] = controllerWert[zaehler];
*/
