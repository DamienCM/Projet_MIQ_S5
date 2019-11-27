#include <Servo.h>


#define sensor A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define enable_motor D3
#define PIN_SENS_1 4
#define PIN_SENS_2 2


#define ETAT_INIT 0
#define PHASE_PLAT 1
#define PHASE_AVANT_ESC 2
#define PHASE_ESCALIER 3
#define PHASE_PALIER 4
#define PHASE_FINAL 5
#define PIN_SERVO_1 3
#define PIN_SERVO_2 4

#define ANGLE_HAUT 70
#define ANGLE_BAS -70

#define TROP_LOIN 42424242

/* ==== CONSTANTES CAPTEUR US ====== */

/* Constantes pour les broches */
const byte TRIGGER_PIN = 2; // Broche TRIGGER
const byte ECHO_PIN = 3;    // Broche ECHO

/* Constantes pour le timeout */
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s

/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;

const float var_angle_par_tick = 0.1;

Servo Servo1;
Servo Servo2;

int passe = 0;
int etat = ETAT_INIT;

int bouton_activation = 0;
int reset = 0;
char receivedChar = 'c';

void setup() {
  /* Initialisation du port série */
  Serial.begin(115200);
   
  /* Initialisation des broches */
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(PIN_SENS_1,OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit être à LOW au repos
  pinMode(ECHO_PIN, INPUT);
  
  /*Initialisation des servo*/
  Servo1.attach(PIN_SERVO_1);
  Servo2.attach(PIN_SERVO_2);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*Serial.println("UR:");
  Serial.println(get_distance_US());
  delay(500);*/
  Serial.println("Tension IR:");
  Serial.println(analogRead(sensor));
  delay(500);
  
  //Switch entre les cas en fonction des variables et de l'état précédent
  //Selon la phase dans laquele on est, on effectue cette loop
  switch(etat){
    case ETAT_INIT:
      //inclure la lecture des button et du bluetooth
      //On attend que le bouton passe à l'etat 1
      if(bouton_activation == 1){
        etat = PHASE_PLAT;
      }
      break;

    case PHASE_PLAT:
      //Lorsque l'on est à 8 cm de la marche, on passe en etat 
      if(get_distance_US() <= 80){
        etat = PHASE_AVANT_ESC;
      }
      break;

    case PHASE_AVANT_ESC:
      //On se place dans l'angle d'approche
      Servo1.write(ANGLE_HAUT);
      Servo2.write(ANGLE_HAUT);
      //Condition de passage à la phase suivante : le capteur IR passe en dessous de 3.5 cm
      if(get_distance_IR() <= 35){
        etat = PHASE_ESCALIER;
      }
      break;
    case PHASE_ESCALIER:
      //on se remet à plat lentement et on avance jusqu'au palier
      Servo1.write(max(ANGLE_HAUT - passe*angle_enleve_par_tick,0));
      Servo2.write(max(ANGLE_HAUT - passe*angle_enleve_par_tick,0));
      //on compte le nb de passe que l'on a dans cette état
      passe = passe + 1;
      //Condition de passage à la phase suivante : le capteur US passe au dessus de 15 cm ? 10 ?
      if(get_distance_US() >= 150){
        passe = 0;
        etat = PHASE_PALIER;
      }
      break;

    case PHASE_PALIER:
      //on se remet à plat lentement et on avance jusqu'au palier
      Servo1.write(min(ANGLE_BAS + passe*var_angle_par_tick,0));
      Servo2.write(min(ANGLE_BAS + passe*var_angle_par_tick,0));
      //on compte le nb de passe que l'on a dans cette état
      passe = passe + 1;
      
      //Condition de passage à la phase suivante : le capteur US mesure une distance supérieure à 30cm
      if(get_distance_US() > 300){
        etat = PHASE_FINAL;
      }
      break;

    case PHASE_FINAL:
      //On arrete les moteur et on attend un reset
      if(reset == 1){
        etat = ETAT_INIT;
      }
      break;
  }
  
  if(etat > ETAT_INIT && etat<PHASE_FINAL){
    //Tant que l'on est entre la phase initiale et la phase finale, on avance à la vitesse Vitesse
    analogWrite(enable_motor, 100);
    //On met IN1 à vrai pour fermer le pont en H en sens avant
    PIN_SENS_1 = 1;
  }else{//On fait en sorte d'être à l'arret dans les autres cas
    //On met IN1 à faux pour ouvrir le pont en H
    PIN_SENS_1 = 0;
  }
}

float get_distance_US(){ //On retourne une valeur lissée sur 10 lectures
  float valeur_moyene = 0;
  for(int i=0;i<10;i++){
    /* 1. Lance une mesure de distance en envoyant une impulsion HIGH de 10µs sur la broche TRIGGER */
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
  
    /* 2. Mesure le temps entre l'envoi de l'impulsion ultrasonique et son écho (si il existe) */
    long measure = pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT);
   
    /* 3. Calcul la distance à partir du temps mesuré */
    float valeur_moyene = valeur_moyene + measure;
  }
  return (valeur_moyene/10)/ 2.0 * SOUND_SPEED;
}

float get_distance_IR(){
  float val = 0;
  //moyennage des valeurs pour attenuer les dépassements ocasionels
  for(int i = 0; i<10;i++){
    float volts = analogRead(sensor)*0.0048828125;  // value from sensor * (5/1024)
    float distance = 130*pow(volts, -1); // worked out from datasheet graph
    val = val + distance;
  }
  
  if (distance <= 300 && distance >= 40){
      return val;
    }else{
      return TROP_LOIN;
    }
}

void recvOneChar() {
 if (Serial.available() > 0) {
 receivedChar = Serial.read();
 newData = true;
 }
}
