int cBlinkCountOpen;
int cBlinkCountClose;
int cComingHomeCount;
int cLightValuesCountNeeded;
int cLightMinimum;
boolean iInitZvOpen;
boolean iInitZvClose;
boolean iInitIgnition;
int iInfoCounter;
int iBlinkCount;
int iComingHomeCount;
int iLightValues;
int iLightValuesCount;
int iLightAverage;
boolean iComingHomeStatus;
boolean iLightStatus;
long eTimeStart;
long eTime;
boolean iZvOpen;
boolean iZvClose;
boolean iIgnition;
int iLight;

void setup() {
  Serial.begin(115200);
  pinMode(33, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(17, INPUT);
  pinMode(34, INPUT);

  // Coming Home Ausgang aus
  digitalWrite(33, LOW);
  // Blinker an (initial, absichtlich als Signalisierung für Programmstart)
  digitalWrite(32, HIGH);
  // Konstanten
  cBlinkCountOpen = 5;
  cBlinkCountClose = 3;
  cComingHomeCount = 20;
  cLightValuesCountNeeded = 10;
  cLightMinimum = 2500;
  // Variablen für letzte Zustände / Bestimmung von Zustandsänderungen
  iInitZvOpen = true;
  iInitZvClose = true;
  iInitIgnition = true;
  // Interne Zähler
  iInfoCounter = 30;
  iBlinkCount = 0;
  iComingHomeCount = 0;
  iLightValues = 0;
  iLightValuesCount = 0;
  iLightAverage = 4096;
  // Statusvariablen
  iComingHomeStatus = false;
  iLightStatus = false;
  // Laufzeitzähler
  eTimeStart = millis();
  Serial.print("Started program at ");
  Serial.println(eTimeStart);

}

void loop() {
  eTime = millis();
  // Zustände der digitalen Eingänge einlesen
  iZvOpen = digitalRead(18);
  iZvClose = digitalRead(19);
  iIgnition = digitalRead(17);
  // v3: Bugfix: Lichtautomatik nur bei eingeschalteter Zündung, sonst Messwert > Max
  if (iIgnition == true) {
    iLight = analogRead(34);
  } else {
    iLight = 9999;
  }
  // Zustandsänderung von Zentralverriegelung öffnen
  if (iZvOpen != iInitZvOpen) {
    iInitZvOpen = iZvOpen;
    Serial.print("ZvOpen changed to ");
    Serial.println(iZvOpen);
    // Blinker und Coming Home nur wenn Eingang auf Masse
    if (iInitZvOpen == false) {
      iBlinkCount = cBlinkCountOpen;
      iComingHomeCount = cComingHomeCount;
    }
  }
  // Zustandsänderung von Zentralverriegelung schließen
  if (iZvClose != iInitZvClose) {
    iInitZvClose = iZvClose;
    Serial.print("ZvClose changed to ");
    Serial.println(iZvClose);
    // Blinker und Coming Home nur wenn Eingang auf Masse
    if (iZvClose == false) {
      iBlinkCount = cBlinkCountClose;
      iComingHomeCount = cComingHomeCount;
    }
  }
  // Zustandsänderung von Zündung an/aus
  if (iIgnition != iInitIgnition) {
    iInitIgnition = iIgnition;
    Serial.print("Ignition changed to ");
    Serial.println(iIgnition);
    iBlinkCount = 0;
    iComingHomeCount = 0;
    // v4: Bugfix: Coming-Home aktivieren, sobald Zündung abgeschaltet wird
    if (iIgnition == false) {
      iComingHomeCount = cComingHomeCount;
    }
  }
  // Bearbeiten der Ausgänge für Blinker und Coming Home nur alle 1/3 Sekunde
  if (eTime - eTimeStart > 333 || eTimeStart > eTime) {
    eTimeStart = eTime;
    iInfoCounter += -1;
    // Regelmäßige Ausgabe des Programmstatus in der seriellen Konsole (nur alle 30 * 1/3 Sekunden)
    if (iInfoCounter == 0) {
      iInfoCounter = 30;
      Serial.print("Program running since ");
      Serial.print(eTime);
      Serial.println("ms");
    }
    // Erfassung der Messwerte des Helligkeitssensors und Mittelwert der Messungen berechnen
    if (iLightValuesCount < cLightValuesCountNeeded) {
      iLightValuesCount += 1;
      iLightValues += iLight;
    } else {
      iLightAverage = iLightValues / iLightValuesCount;
      Serial.print("Light average: ");
      Serial.println(iLightAverage);
      iLightValues = 0;
      iLightValuesCount = 0;
    }
    // Ansteuerung des Blinker Ausgangs
    if (iBlinkCount > 0) {
      Serial.print("Blink count:");
      Serial.println(iBlinkCount);
      if (iBlinkCount % 2 == 1) {
        digitalWrite(32, HIGH);
      } else {
        digitalWrite(32, LOW);
      }
      iBlinkCount += -1;
    } else if (iBlinkCount == 0) {
      iBlinkCount += -1;
      Serial.println("Blinking off");
      digitalWrite(32, LOW);
    }
    // Ansteuerung des Coming Home Ausgangs
    if (iComingHomeCount > 0) {
      iComingHomeCount += -1;
      if (iComingHomeStatus == false) {
        iComingHomeStatus = true;
        Serial.println("Coming home on");
      }
    } else if (iComingHomeCount == 0) {
      iComingHomeCount += -1;
      if (iComingHomeStatus == true) {
        iComingHomeStatus = false;
        Serial.println("Coming home off");
      }
    }
    // Lichtautomatik
    if (iLightAverage < cLightMinimum) {
      // Es ist zu dunkel: Licht an
      if (iLightStatus == false) {
        iLightStatus = true;
        Serial.println("Licht an");
      }
    } else {
      // Es ist hell genug: Licht aus
      if (iLightStatus == true) {
        iLightStatus = false;
        Serial.println("Licht aus");
      }
    }
    // Licht an / aus
    if (iComingHomeStatus == true || iLightStatus == true) {
      digitalWrite(33, HIGH);
    } else {
      digitalWrite(33, LOW);
    }
  }
  delay(25);

}
