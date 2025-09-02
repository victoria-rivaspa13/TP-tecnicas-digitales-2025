// Pines
const int buzzerPin = 13; // Cambia al pin que uses

// Canal y frecuencia para ledc (PWM ESP32)
const int channel = 0;      
const int freq = 2000;      // Frecuencia inicial (Hz)
const int resolution = 8;   // Resolución de 8 bits

void setup() {
  // Configuramos el canal PWM
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(buzzerPin, channel);
}

void loop() {
  // Generamos un tono de 1000 Hz
  ledcWriteTone(channel, 1000); 
  delay(500);

  // Cambiamos a 1500 Hz
  ledcWriteTone(channel, 1500); 
  delay(500);

  // Apagamos el buzzer
  ledcWriteTone(channel, 0); 
  delay(500);
}
