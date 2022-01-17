#include <Servo.h> //Die Servobibliothek wird aufgerufen.
#include <HX711_ADC.h> //Bibliothek für die HX711 Wägezelle wird aufgerufen
#include <EEPROM.h> // Bibliothek für den EEPROM Speicher wird aufgerufen

Servo servo1; // Erstellt für das Programm einen Servo namens:"Servo1"

int TasterOben = 7  ; // "int" erstellt eine Variable des Typs Integer. Hier wird dies benutzt um Pins festzulegen, was das Programm flexibler und übersichtlicher macht.
int TasterUnten = 6 ;
int GSM = 31 ; // Pin der den Gleichstrommotor Ansteuert, mit ihm lässt sich die maximale Drehgeschwindigkeit des GSM bestimmen
int in1 = 41  ; // Pins für den Gleichstrommotor der Lineareinheit
int in2 = 39 ;
int groß = 13 ; // Variable für den Pin der Schalter, welche die Größe der Kugel angeben
int mittel = 12 ;
int klein = 11 ;
int schwer = 10; // Variable für den Pin der Schalter, welche das gewünschte Gewicht angeben
int mschwer = 9 ;
int leicht = 8 ;
const int HX711_dout = 2;
const int HX711_sck = 3;
int eepromAdress = 0;
unsigned long t = 0;

HX711_ADC LoadCell(HX711_dout, HX711_sck);
HX711_ADC LoadCell(2, 3);

// in diesem Unterprogramm wird die Wägezelle kalibriert. Der Sensor funktioniert über Widerstände, welche sich bei Belastung leicht dehnen.
// Die Änderung wird durch einen A/D-Wandler ausgewertet und in einer Variable gespeichert. Anfangs muss man die Waage mit einem bekannten Gewicht belasten,
// und dies dem Programm über den seriellen Monitor weitergeben. Somit wird die Waage kalibriert.
void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("It is assumed that the mcu was started with no load applied to the load cell.");
  Serial.println("Now, place your known mass on the loadcell,");
  Serial.println("then send the weight of this mass (i.e. 100.0) from serial monitor.");
  float m = 0;
  boolean f = 0;
  while (f == 0) {
    LoadCell.update();
    if (Serial.available() > 0) {
      m = Serial.parseFloat();
      if (m != 0) {
        Serial.print("Known mass is: ");
        Serial.println(m);
        f = 1;
      }
      else {
        Serial.println("Invalid value");
      }
    }
  }
  float c = LoadCell.getData() / m;
  LoadCell.setCalFactor(c);
  Serial.print("Calculated calibration value is: ");
  Serial.print(c);
  Serial.println(", use this in your project sketch");
  f = 0;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(eepromAdress);
  Serial.println("? y/n");
  while (f == 0) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)
        EEPROM.begin(512);
#endif
        EEPROM.put(eepromAdress, c);
#if defined(ESP8266)
        EEPROM.commit();
#endif
        EEPROM.get(eepromAdress, c);
        Serial.print("Value ");
        Serial.print(c);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(eepromAdress);
        f = 1;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        f = 1;
      }
    }
  }
  Serial.println("End calibration");
  Serial.println("For manual edit, send 'c' from serial monitor");
  Serial.println("***");
}

void changeSavedCalFactor() { // über dieses Unterprogramm lässt sich der Kalibrationsfaktor ändern, indem man einen neuen im seriellen Monitor eingibt.
  float c = LoadCell.getCalFactor();
  boolean f = 0;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(c);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  while (f == 0) {
    if (Serial.available() > 0) {
      c = Serial.parseFloat();
      if (c != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(c);
        LoadCell.setCalFactor(c);
        f = 1;
      }
      else {
        Serial.println("Invalid value, exit");
        return;
      }
    }
  }
  f = 0;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(eepromAdress);
  Serial.println("? y/n");
  while (f == 0) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)
        EEPROM.begin(512);
#endif
        EEPROM.put(eepromAdress, c);
#if defined(ESP8266)
        EEPROM.commit();
#endif
        EEPROM.get(eepromAdress, c);
        Serial.print("Value ");
        Serial.print(c);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(eepromAdress);
        f = 1;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        f = 1;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
}
void Hoch() { // void ist ein Variablentyp ohne Rückgabewert, welcher hier dazu benutzt wird um Unterprogramme zu schreiben.
  digitalWrite(in1, HIGH); // Motor dreht sich so, dass die Lineareinheit hochfährt
  digitalWrite(in2, LOW); // je nachdem ob in1 oder in2 HIGH oder LOW ist, dreht sich der GSM in eine andere Richtung.
}
void Runter() {
  digitalWrite(in2, HIGH);
  digitalWrite(in1, LOW);
}
void Normal() {   // Der Servo wird mit Gradzahlen gesteuert. "Normal" bezeichnet die Waagerechte Position des Servos bzw. der daran befestigten Waageplatte
  servo1.write(90); // Damit man 90° nach links und rechts drehen kann, muss die Normale Position auf 90° sein.
}
void Weiter() { // "Weiter" dreht die Waageplatte so, dass die Kugel in die Laufbahn fällt, welche
  servo1.write(0); // Der Servo dreht sich auf die 0°-Position
}
void Müll() {
  servo1.write(180); // Der Servo dreht sich auf die 180° Position
}

void setup() {
  pinMode(GSM, OUTPUT); // alle pinMode Befehle bestimmen den Modus der jeweiligen Pins
  pinMode(in1, OUTPUT); // wird etwas vom Mikrocontroller gesteuert ist es in der Regel OUTPUT, bei Sensoren, welche einen Wert zurückgeben INPUT
  pinMode(in2, OUTPUT);
  pinMode(TasterOben, INPUT);
  pinMode(TasterUnten, INPUT);
  pinMode(groß, INPUT);
  pinMode(mittel, INPUT);
  pinMode(klein, INPUT);
  pinMode(schwer, INPUT);
  pinMode(mschwer, INPUT);
  pinMode(leicht, INPUT);

  servo1.attach(4); // setzt fest, dass der Servo an pin 4 angeschlossen ist



  Serial.begin(9600); delay(10); // Waage wird gestartet und das Unterprogramm zur Kalibrierung ausgeführt.
  Serial.println();
  Serial.println("Starting...");
  LoadCell.begin();
  LoadCell.start();
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(1.0); // user set calibration value (float)
    Serial.println("Startup + tare is complete");
  }
  while (!LoadCell.update()); {
    calibrate();
  }
}

void loop() {

// Die Waage misst alle 250ms das Gewicht und speichert es in der Variablen i. Außerdem gibt sie den Wart am seriellen Monitor an.
  LoadCell.update();
  //get smoothed value from the data set
  if (millis() > t + 250) {
    float i = LoadCell.getData();
    Serial.print("Load_cell output val: ");
    Serial.println(i);
    t = millis();
  }
  int value = analogRead(A0); // analogRead liest den Wert an Port A0 ab und speichert ihn in der Variablen "value". Am Port A0 ist der Photoresistor angeschlossen.

  if (value <= 150) { // Fällt "value" unter 150, weil eine Kugel vorbeirollt, wird die If-Schleife ausgelöst.
    Hoch (); // Das Unterprogramm "Hoch wird ausgeführt.
  }
  if (digitalRead(TasterOben) == HIGH) { // Abfrage ob der TasterOben gedrückt wird
    Runter();
  }
  if (digitalRead(TasterUnten) == HIGH) {
    digitalWrite(in2, LOW); // Der Motor hört auf sich zu drehen
    digitalWrite(in1, LOW);
  }


  if (digitalRead(groß) == HIGH) { // Abfrage ob der Schalter für die großen Kugeln umgelegt ist, wenn ja wird die Schleife ausgeführt


    if (digitalRead(schwer) == HIGH{ // Abfrage über den Schalter, ob das gewünschte Material schwer ist. Wenn ja wird die Schleife ausgeführt
    if (i >= x && i <= y) { // i ist der Rückgabewert der Wägezelle. Abfrage ob i im Zielbereich liegt, also zwischen x und y, da die Waage einen Toleranzbereich braucht.
        // x und y werden noch für jede Größen- und Wunschgewicht/Material -Kombi ersetzt, nachdem wir die Werte ermittelt haben.
        Weiter(); // Unterprogramm "Weiter" wird ausgeführt.
      } else{
        Müll(); // Unterprogramm "Müll" wird ausgeführt, wenn das Gewicht nicht im passenden Bereich liegt(else).
      }
    }
    // das Muster wiederholt sich jetzt für die drei Materialtypen, es ändern sich lediglich x und y.
    if (digitalRead(mschwer) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }
    if (digitalRead(leicht) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }
  }
  // Es wiederholt sich jetzt die Große if-Schleife. Sie wird aktiviert wenn der Schalter für den Eingang von mittelgroßen Kugeln umgelegt ist.
  // Da eine große Kugel, aus dem gleichen Material wie eine mittlere Kugel trotzdem mehr wiegt, werden in diese Schleife die x- und y- Werte angepasst, sobald sie gemessen sind. Bis auf die Werte unterscheidet sich trotzdem nichts.

  if (digitalRead(mittel) == HIGH) {


    if (digitalRead(schwer) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }

    if (digitalRead(mschwer) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }
    if (digitalRead(leicht) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }
  }

  // Erneut die gleiche if- Schleife, welche aktiviert wird, wenn der Schalter für den Eingang von kleinen Kugeln umgelegt ist.
  // Die Programmierweise mit 3 verschiedenen großen if-Schleifen für den Eingang der Kugeln, welche durch Schalter gesteuert werden, ermöglicht die Schnelle Anpassung des Wunschgewichts, je nach Kugelgröße.
  // Innerhalb der großen If-Schleife befinden sich 3 kleinere If-Schleifen, welche ebenfalls mit Schaltern aktiviert werden können. Durch die Kombination der kleinen und Großen if-Schleifen lassen sich leicht
  // die Gewichtswerte für alle möglichen Größen und Materialien anpassen.
  if (digitalRead(klein) == HIGH) {


    if (digitalRead(schwer) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }

    if (digitalRead(mschwer) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }
    if (digitalRead(leicht) == HIGH{
    if (i >= x && i <= y) {
        Weiter();
      } else{
        Müll();
      }
    }
  }



}// loop Klammer zu
