#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    7
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 24

#define ButtonSelectPinte 2
#define ButtonSelectDemiPinte 3
#define ButtonStart 8

#define ComptPinte 10
#define ComptDemiPinte 9

#define Relai1 6
#define Relai2 5
#define Relai3 4
#define Relai4 12

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int EtatVerre = 0;
int statu = 1;

int statuB1 = 0;
int statuB2 = 0;

int TempsInclinaison = 2000;
int TempsPinte = 300;//200
int TempsDemiPinte = 600;//150

void setup() {
  Serial.begin(9600);

  InitRelais();
  InitCompteur();

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif

  strip.begin();
  strip.show();
  strip.setBrightness(50);
}

void loop() {
  //Serial.println(digitalRead(ButtonSelectPinte));
  Serial.println("0");
  if (StatuCapteur() == 1) {
    Serial.println("1");
    Chenillard(strip.Color(  255, 0, 0), 5);
    StatuBouton1(digitalRead(ButtonSelectPinte));
    StatuBouton2(digitalRead(ButtonSelectDemiPinte));
    Serial.println();
    if ((digitalRead(ButtonStart) == 1) && (EtatVerre == 1) && (statuB1 == 1)&& (statuB2 == 0)) {
      statuB1 = 0;
      statuB2 = 0;
      CompteurPinte(500);
      Inclinaison(TempsInclinaison);
      ChenillardRelais(strip.Color(255, 57, 0), 0, 0, 0, 1, TempsPinte*(3/4));
      Serial.println("5");
      Descent(TempsInclinaison);
      statu = 0;
      int compteur = 0;
      while (statu == 0) {
        compteur++;
        if ((compteur == 1) && (statu == 0)) {
          ChenillardRelais(strip.Color(0, 255, 0), 1, 1, 1, 1, 1);
          delay(1000);
        }
        else if ((compteur == 2) && (statu == 0)) {
          ChenillardRelais(strip.Color(255, 57, 0), 1, 1, 1, 1, 1);
          compteur = 0;
          delay(1000);
        }
      }
      
      EtatVerre = 0;
    }
    else if ((digitalRead(ButtonStart) == 0) && (EtatVerre == 1) && (statuB1 == 0) && (statuB2 == 1)) {
      statuB1 = 0;
      statuB2 = 0;
      Serial.println("2");
      CompteurDemiPinte(500);
      Inclinaison(TempsInclinaison);
      ChenillardRelais(strip.Color(255, 57, 0), 0, 0, 0, 1, TempsDemiPinte*(3/4));
      Serial.println("4");
      Descent(TempsInclinaison);
      statu = 0;
      int compteur = 0;
      
      while (statu == 0) {
          compteur++;
          if ((compteur == 1) && (statu == 0))  {
          ChenillardRelais( strip.Color(0, 0, 255), 1, 1, 1, 1, 1);
          delay(1000);
          }
          else if ((compteur == 2) && (statu == 0)) {
          ChenillardRelais(strip.Color(255, 57, 0), 1, 1, 1, 1, 1);
          compteur = 0;
          delay(1000);
          }
      }
      
      EtatVerre = 0;
    } else {
      Serial.print("Autre");
    }
  } else {
    Serial.println("5");
    //Serial.println("Verre Absent");
    Descent(TempsInclinaison);
    BaseColor(5);//Effet sans le verre
  }
  delay(5);
}

//Fonction utilisée pour
void BaseColor(int temps) {//Paramétrage du temps
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) {
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    if (StatuCapteur() == 1) {//Sortie de la boucle une fois le verre présent
      break;
    }
    strip.show();
    delay(5);
  }
}

//Fonction permettant de donner un effet chenillard
void ChenillardRelais(uint32_t color, int r1, int r2, int r3, int r4, int temps) {//Paramétrage de la couleur, de l'état des relais et du temps
  Relais(r1, r2, r3, r4);
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(temps);
    if (StatuCapteur() == 0) {//Sortie de la boucle une fois le verre absent
      statu = 1;
      break;
    }
  }
}
//Fonction permettant de donner un effet chenillard
void Chenillard(uint32_t color, int temps) {//Paramétrage de la couleur, de l'état des relais et du temps
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(temps);
    if (StatuCapteur() == 0) {//Sortie de la boucle une fois le verre absent
      statu = 1;
      break;
    }
  }
}
//Fonction retournant l'état du capteur optique
int StatuCapteur() {
  //Déclaration des variables
  int statuCapt = 0;
  int mesure = 0;

  mesure = analogRead(A0);      //Mesure du Capteur Optique
  //Serial.println(mesure);      //Affichage de la mesure

  if (mesure >= 700) {//Calibration de l'état du capteur optique
    statuCapt = 1;
  } else {
    statuCapt = 0;
    EtatVerre = 1;
  }
  //delay(500);
  return statuCapt;
}

void StatuBouton1(int valor){
  if(valor == 1){
    statuB1 = 1;
    statuB2 = 0;
  }
}

void StatuBouton2(int valor){
  if(valor == 1){
    statuB1 = 0;
    statuB2 = 1;
  }
}

//Initialisation des relais
void InitRelais() {
  pinMode(Relai3, OUTPUT);    // Déclaration relais 3
  pinMode(Relai4, OUTPUT);     // Déclaration relais 4
  pinMode(Relai2, OUTPUT);     // Déclaration relais 2
  pinMode(Relai1, OUTPUT);     // Déclaration relais 1
  digitalWrite(Relai4, HIGH);
  digitalWrite(Relai3, HIGH);
  digitalWrite(Relai2, HIGH);
  digitalWrite(Relai1, HIGH);
}

//Initialisation des compteurs

void InitCompteur() {
  pinMode(ComptPinte, OUTPUT);    // Déclaration du compteur 1
  pinMode(ComptDemiPinte, OUTPUT);    // Déclaration du compteur 2
  digitalWrite(ComptPinte, LOW);
  digitalWrite(ComptDemiPinte, LOW);
}

//Fonction pour lancer une séquence de test des relais
void test_Relai() {
  digitalWrite(Relai1, LOW);      // Activation des relais 1
  digitalWrite(Relai3, LOW);      // Activation des relais 3
  delay(1000);
  digitalWrite(Relai3, HIGH);      // Désactivation des relais 3
  digitalWrite(Relai1, HIGH );      // Désactivation des relais 1
  delay(1000);
  digitalWrite(Relai4, LOW);      // Activation des relais 4
  digitalWrite(Relai2, LOW);      // Activation des relais 2
  delay(1000);
  digitalWrite(Relai1, HIGH);      // Désactivation des relais 1
  digitalWrite(Relai2, HIGH);      // Désactivation des relais 2
  digitalWrite(Relai3, HIGH);     // Désactivation des relais 3
  digitalWrite(Relai4, HIGH);      // Désactivation des relais 4
  delay(1000);
}

//Fonction pour le paramétrage des relais
void Relais(int r1, int r2, int r3, int r4) {
  digitalWrite(Relai1, r1);      // pilotage relais 1
  digitalWrite(Relai2, r2);      // pilotage relais 2
  digitalWrite(Relai3, r3);     // pilotage relais 3
  digitalWrite(Relai4, r4);      // pilotage relais 4
}

//Fonction pour Tester Chacque Relais
void DemiPinte(int temps) {
  Relais(0, 0, 1, 0);
  delay(temps);
  Relais(0, 0, 0, 0);
}
void Pinte(int temps) {
  Relais(0, 0, 1, 0);
  delay(temps);
  Relais(0, 0, 0, 0);
}
void Inclinaison(int temps) {
  Relais(0, 0, 1, 1);
  delay(temps);
}

void Descent(int temps) {
  Relais(1, 1, 1, 1);
  delay(temps);
}

//Fonction pour le pilotage des compteurs de pinte
void CompteurPinte(int temps) {
  digitalWrite(ComptPinte, HIGH);
  delay(temps);
  digitalWrite(ComptPinte, LOW);
  delay(temps);
}
void CompteurDemiPinte(int temps) {
  digitalWrite(ComptDemiPinte, HIGH);
  delay(temps);
  digitalWrite(ComptDemiPinte, LOW);
  delay(temps);
}

void CalculPinte(int Cl, int comptPinte, int comptDemiPinte){
  
  int resultCl = Cl - (comptPinte*50 + comptDemiPinte*25);
  int PourcentCl = (100*resultCl)/Cl;
  int PintePossible = resultCl/50;
  int DemiPintePossible = resultCl/25;

  Serial.print("Quantitée restante : ");
  //Serial.print();
  Serial.println(" Cl");

  Serial.print("Pourcentage : ");
  //Serial.print();
  Serial.println("%");

  Serial.print("Nombre de Pinte Possible : ");
  //Serial.print();
  Serial.println(" Verre");


  Serial.print("Nombre de Demi Pinte Possible : ");
  Serial.println(" Verre");

}
