/*
Circuito: 
 * pin RS collegato al pin digitale 7 
 * pin E (Enable) collegato al pin digitale 6 
 * pin D4 collegato al pin digitale 5 
 * pin D5 collegato al pin digitale 4 
 * pin D6 collegato al pin digitale 3 
 * pin D7 collegato al pin digitale 2 
 * pin R/W collegato al GND 
 * pin 1 e pin 4 collegati a GND 
 * pin 2 collegato a +Vcc 
 * centrale del potenziometro/trimmer da 10 KOhm collegato al pin 3 del'LCD 
 * pin SX potenziometro/trimmer collegato a +Vcc 
 * pin DX potenziometro/trimmer collegato a GND 
 * i pin SX e DX del potenziometro/trimmer possono essere interscambiati 
*/


#include <LiquidCrystal.h>
/* 
   Viene creata l'istanza dell'oggetto LiquidCrystal chiamata lcd in cui 
   sono indicati i pin dell'LCD collegati alle uscite digitali di Arduino 
*/
const int rs = 7, e = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, e, d4, d5, d6, d7);

/*Righe e colonne per display LCD*/
const int display_cols = 16;
const int display_rows = 2;

#define MS 20           // Intervallo tra i campioni in millisecondi
#define SAMPLES 22      // Numero di campioni da acquisire per la media
#define KY39 A0         // Pin analogico per leggere i dati del sensore
#define reliability 50  // Numero di letture affidabili da considerare

/*LED collegati a tre uscite digitali di Arduino*/
#define LedRosso 10
#define LedGiallo 9
#define LedVerde 8


const int MESSAGE_COUNT = 6; //numero totale dei messaggi


long heartbeats = 0;        // heartbeatsatore per le letture affidabili
double BPM = 0;             // Valore BPM grezzo
int calibrated_BPM = 0;  // Valore BPM calibrato

double start_time = 0;  // Ora di inizio delle letture affidabili
double end_time = 0;    // Ora di fine delle letture affidabili

int age = 0;  //eta' dell'utente

bool isCardioCheckDone = false;

// Definisco il carattere personalizzato del cuore
byte Cuore[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};


//bool isCardioCheckDone;
/*metodo void che utilizzando un ciclo for da 0 fino alla
lunghezza della stringa misurata con il metodo lenght() ad
ogni incremento della variabile i sposta il display verso 
sinistra di una posizione, in questo modo al termine del ciclo il
display sarà spostato di un numero di carattere pari alla 
lunghezza della stringa facendo scomparire di conseguenza la stringa 
visualizzata nel display */

void Scroll_Left(String text) {
  /*misuro la lunghezza della stringa utilizzando il metodo length()
della classe string*/

  int strLength = text.length();
  for (int i = 0; i < strLength; i++) {
    //scorro la stringa a sinistra
    lcd.scrollDisplayLeft();
    delay(500); /*attendo 150 millisecondi per ciascun incremento del ciclo for
                per consentire lo spostamento*/
  }
  delay(1); /*attende 1 sec tra un passaggio del loop e il successivo*/
}

/*Struttura di String*/
struct Message {
  String text1;
  String text2;
};

/*Messaggi che verranno visualizzati nello schermo lcd
in base alla frequenza cardiaca*/
Message messages[MESSAGE_COUNT] = {
  { "ETA: ", "BPM: " },
  { "INSERISCI ETA':", "TRA 20 E 70 ANNI" },
  { "BATTITI NORMALI", " " },
  { "FREQUENZA MAS.", "TACHICARDIA" },
  { "FREQUENZA MIN.", "BRACHICARDIA" },
  { "LETTURA IN CORSO...", " " },
};
/* Funzione che viene utilizzata per visualizzare un messaggio sullo schermo LCD.*/
void MessageLCD(int messageIndex) {
  lcd.setCursor(0, 0);
  delay(100);
  lcd.print(messages[messageIndex].text1);
  if (messages[messageIndex].text1.length() > display_cols) {
    Scroll_Left(messages[messageIndex].text1);
  }
  lcd.setCursor(0, 1);
  lcd.print(messages[messageIndex].text2);
  if (messages[messageIndex].text2.length() > display_cols) {
    Scroll_Left(messages[messageIndex].text2);
  }
}

/*Stampa sullo schermo LCD IL BPM E l'età dell'utente*/
void AgeAndBPM() {
  lcd.clear();
  MessageLCD(0);
  lcd.setCursor(5, 0);
  lcd.print(age);

  lcd.setCursor(5, 1);
  lcd.print(calibrated_BPM);
}

/*La funzione sample_difference calcola la differenza tra due campioni 
consecutivi acquisiti dal sensore. Questa differenza viene utilizzata successivamente
 per determinare se si è verificato un picco o una discesa nel segnale dei battiti cardiaci.*/
int sample_difference() {
  float first_sample = read_sample();  //Primo campione
  delay(MS);
  float second_sample = read_sample();  //secondo campione
  float difference = second_sample - first_sample;
  return difference;
}

/*La funzione read_sample() 
acquisisce un numero specificato di campioni 
dal sensore analogico KY-039 e ne calcola la media*/
float read_sample() {
  float sum = 0;  //accumula la somma dei valori letti dal sensore.
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(KY39);  //leggo il valore del sensore KY-039
  }

  /*Dopo aver acquisito tutti i campioni, il valore medio viene 
 calcolato dividendo la somma totale (sum) per il numero di campioni (SAMPLES)*/
  return sum / SAMPLES;
}


/*Funzione utilizzata per acquisire in input */
void InputAge() {

  lcd.setCursor(0, 0);
  MessageLCD(1);

  while (!Serial.available()) {
    // Aspetta che ci siano dati disponibili sul monitor seriale
  }
/* Questa funzione legge la stringa inviata dal monitor seriale 
fino a quando non viene rilevato il carattere di nuova riga */
  String input = Serial.readStringUntil('\n');
  
  age = input.toInt(); //Converte la stringa letta in un numero intero utilizzando la funzione 

  while (age < 20 || age > 70) {
    // Continua a richiedere l'età finché non viene inserita un'età valida
    lcd.setCursor(0, 0);
    MessageLCD(1);

    while (!Serial.available()) {
      // Aspetta che ci siano dati disponibili sul monitor seriale
    }
/* Questa funzione legge la stringa inviata dal monitor seriale 
fino a quando non viene rilevato il carattere di nuova riga */
    input = Serial.readStringUntil('\n');
    age = input.toInt(); //Converte la stringa letta in un numero intero utilizzando la funzione 
  }
  lcd.clear();
}

/*Funzoine che in base al BPM rilevato fa accendere/spegnere i led e stampa sullo schermo LCD un avviso per l'utente*/
void LedBPM() {

  // Prendiamo in considerazione soltanto la fascia d'età compresa tra 20 e 70, quindi i valori tra 75 e 170
  if (calibrated_BPM >= 75 && calibrated_BPM <= 170) {
    // Frequenza cardiaca ideale

    for (int i = 0; i < 10; i++) {

      digitalWrite(LedVerde, HIGH);
      delay(100);
      digitalWrite(LedVerde, LOW);
      delay(100);
    }
    lcd.clear();
    MessageLCD(2);
    delay(1000);
    digitalWrite(LedGiallo, LOW);
    digitalWrite(LedVerde, LOW);
    digitalWrite(LedRosso, LOW);

  } else if (calibrated_BPM > 170) {
    // frequenza massima

    for (int i = 0; i < 10; i++) {

      digitalWrite(LedGiallo, HIGH);
      delay(100);
      digitalWrite(LedGiallo, LOW);
      delay(100);
      digitalWrite(LedGiallo, LOW);
      delay(100);
      digitalWrite(LedGiallo, HIGH);
      delay(100);
    }
    lcd.clear();
    MessageLCD(3);
    delay(1000);
    digitalWrite(LedGiallo, LOW);
    digitalWrite(LedVerde, LOW);
    digitalWrite(LedRosso, LOW);

  } else if (calibrated_BPM < 75) {

    for (int i = 0; i < 10; i++) {

      digitalWrite(LedRosso, LOW);
      delay(100);
      digitalWrite(LedRosso, HIGH);
      delay(100);
      digitalWrite(LedGiallo, LOW);
      delay(100);
      digitalWrite(LedGiallo, HIGH);
      delay(100);
    }
    lcd.clear();
    MessageLCD(4);
    delay(1000);
    digitalWrite(LedGiallo, LOW);
    digitalWrite(LedVerde, LOW);
    digitalWrite(LedRosso, LOW);
  }
}

/* La funzione CardioCheck effettua il controllo dei battiti cardiaci utilizzando la differenza tra i campioni acquisiti.
 Conta i picchi del segnale dei battiti cardiaci e calcola il valore BPM grezzo e calibrato.*/
void CardioCheck() {

  // Calcola la differenza tra due campioni consecutivi
  float difference = sample_difference();

  // heartbeats continua a calcolare la differenza finché non si verifica una discesa nel segnale
  if (difference > 0) {
    while (difference > 0) {
      difference = sample_difference();
    }
    heartbeats++;  // Incrementa il contatore di picchi
    if (heartbeats == 1) {
      start_time = millis();  // Registra il tempo di inizio del primo picco
    }
  } else  //f(x+1)<f(x)=> abbiamo raggiunto un picco di massimo/minimo-funzione decrescente
  {
    while (difference <= 0) {
      difference = sample_difference();  // Continua a calcolare la differenza finché non si verifica un picco nel segnale
      Serial.println(difference);
    }
    heartbeats++;  // Incrementa il contatore di picchi
    if (heartbeats == 1) {
      start_time = millis();  // Registra il tempo di inizio del primo picco
    }
  }


  if (heartbeats == reliability) {
    end_time = millis();                                     // Registra il tempo di fine dell'ultimo picco
    BPM = ((heartbeats * 30000) / (end_time - start_time));  // Calcola il valore BPM grezzo (battiti*minuto)
    calibrated_BPM = BPM * 0.21 - 12.56;                     // Applico una calibrazione al valore BPM grezzo piu vicino possibile ai battiti*minuto

    digitalWrite(LedVerde, HIGH);
    delay(300);
    digitalWrite(LedGiallo, HIGH);
    delay(600);
    digitalWrite(LedRosso, HIGH);
    delay(800);

    MessageLCD(5);

    digitalWrite(LedRosso, LOW);
    delay(300);
    digitalWrite(LedGiallo, LOW);
    delay(600);
    digitalWrite(LedVerde, LOW);
    delay(800);

    AgeAndBPM();  // Visualizza l'età e il valore BPM sul display LCD
    LedBPM();

    delay(1000);

    do {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("0->NUOVA MISURAZ.");
      lcd.setCursor(0, 1);
      lcd.print("1->ARRIVEDERCI");
      while (!Serial.available()) {
        // Aspetta che ci siano dati disponibili sul monitor seriale
      }

      isCardioCheckDone = Serial.parseInt();
   
    } while ((isCardioCheckDone != false && isCardioCheckDone != true));

    if (isCardioCheckDone == true) {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("ARRIVEDERCI!");
      for (int cols = 0; cols < display_cols; cols++) {
        lcd.setCursor(cols, 1);
        lcd.write(byte(0));  // Visualizza il carattere personalizzato del cuore
        delay(100);
      }
      digitalWrite(LedVerde, HIGH);
      delay(100);
      digitalWrite(LedGiallo, HIGH);
      delay(100);
      digitalWrite(LedRosso, HIGH);
      delay(100);
      digitalWrite(LedRosso, LOW);
      delay(100);
      digitalWrite(LedGiallo, LOW);
      delay(100);
      digitalWrite(LedVerde, LOW);
      lcd.clear();
      return; //blocco il programma
    } else {
      lcd.clear();
      InputAge();  // Richiede l'input dell'età per effettuare una nuova misurazione
    }

    heartbeats = 0;  // Reimposta il contatore di picchi
  }
}

void setup() {
  Serial.begin(9600);  //velocita' lettura del monitor seriale dall'Arduino

  pinMode(LedRosso, OUTPUT);
  pinMode(LedGiallo, OUTPUT);
  pinMode(LedVerde, OUTPUT);

  //impostiamo il numero di colonne ed il numero di righe dello schermo LCD
  lcd.begin(display_cols, display_rows);

  lcd.noCursor();  //rende il cursore invisibile
  lcd.noBlink();   //cursore fisso non lampeggiante

  lcd.createChar(0, Cuore); //creo il nuovo carattere
  lcd.setCursor(1, 0);
  /*scritta iniziale*/
  lcd.print("BPM CALCULATOR");
  for (int cols = 0; cols < display_cols; cols++) {
    lcd.setCursor(cols, 1);
    lcd.write(byte(0));  // Visualizza il carattere personalizzato del cuore
    delay(100);
  }
  delay(2500);
  lcd.clear();

  InputAge();  // Richiede l'input dell'età per la prima esecuzione del programma
}
void loop() {

/*ciclo "do-while" che esegue continuamente la funzione CardioCheck() finché la condizione !isCardioCheckDone è true.*/
  do {

    CardioCheck();

  } while (!isCardioCheckDone);
}
