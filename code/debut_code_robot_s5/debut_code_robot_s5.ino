
#define sensor A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)

#define ETAT_INIT 0
#define PHASE_PLAT 1
#define PHASE_AVANT_ESC 2
#define PHASE_ESCALIER 3
#define PHASE_PALIER 4
#define PHASE_FINAL 5

#define TROP_LOIN 424242424242

/* ==== CONSTANTES CAPTEUR US ====== */

/* Constantes pour les broches */
const byte TRIGGER_PIN = 2; // Broche TRIGGER
const byte ECHO_PIN = 3;    // Broche ECHO

/* Constantes pour le timeout */
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s

/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;

int etat = ETAT_INIT;

int bouton_activation = 0;

void setup() {
  /* Initialisation du port série */
  Serial.begin(115200);
   
  /* Initialisation des broches */
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit être à LOW au repos
  pinMode(ECHO_PIN, INPUT);
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
      //On attend que le bouton passe à l'etat 1
      if(bouton_activation == 1){
        etat = PHASE_PLAT;
      }
      break;

    case PHASE_PLAT:
      //On avance tout droit tant que l'on est pas à moins de 8cm de la première marche
      
      //Lorsque l'on est à 8 cm de la marche, on passe en etat 
      if(get_distance_US() <= 80){
        etat = PHASE_AVANT_ESC;
      }
      break;

    case PHASE_AVANT_ESC:
      //On continue d'avancer
      
      //On se place dans l'angle d'approche
      
      //Condition de passage à la phase suivante : le capteur IR passe en dessous de 3.5 cm
      if(get_distance_IR() <= 35){
        etat = PHASE_ESCALIER;
      }
      break;
    case PHASE_ESCALIER:
      //on se remet à plat lentement et on avance jusqu'au palier

      //Condition de passage à la phase suivante : le capteur IR passe au dessus de 15 cm
      if(get_distance_IR() >= 150){
        etat = PHASE_PALIER;
      }
      break;

    case PHASE_PALIER:
      //On se place dans l'angle d'arrivé, puis on se remet à plat lentement

      //Condition de passage à la phase suivante : le capteur IR passe en etat NULL
      if(get_distance_IR() == TROP_LOIN){
        etat = PHASE_FINAL;
      }
      break;

    case PHASE_FINAL:
      //On arrete les moteur et on attend
      break;
  }
}

float get_distance_US(){ //est ce qu'on essaye de retourné une valeur lissée sur 10 lecture ?
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
  //filtrage des valeurs 
  float volts = analogRead(sensor)*0.0048828125;  // value from sensor * (5/1024)
  float distance = 130*pow(volts, -1); // worked out from datasheet graph
  
  if (distance <= 300 && distance >= 80){
    return distance;
  }else{
    return TROP_LOIN;
  }
}
