
#define sensor A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)

/* ==== CONSTANTES CAPTEUR US ====== */

/* Constantes pour les broches */
const byte TRIGGER_PIN = 2; // Broche TRIGGER
const byte ECHO_PIN = 3;    // Broche ECHO

/* Constantes pour le timeout */
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s

/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;

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
}

float get_distance_US(){ //est ce qu'on essaye de retourné une valeur lissée sur 10 lecture ?
  /* 1. Lance une mesure de distance en envoyant une impulsion HIGH de 10µs sur la broche TRIGGER */
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  /* 2. Mesure le temps entre l'envoi de l'impulsion ultrasonique et son écho (si il existe) */
  long measure = pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT);
   
  /* 3. Calcul la distance à partir du temps mesuré */
  float distance_mm = measure / 2.0 * SOUND_SPEED;
  return distance_mm;
}

float get_distance_IR(){
  // 5v
  float volts = analogRead(sensor)*0.0048828125;  // value from sensor * (5/1024)
  float distance = 130*pow(volts, -1); // worked out from datasheet graph
  
  if (distance <= 300){
    return distance;
  }else{
    return 0;
  }
}
