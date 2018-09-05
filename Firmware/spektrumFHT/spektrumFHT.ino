#define LOG_OUT 1 // use the log output function
#define FHT_N 256  // set to 256 point fht
#include <FHT.h>
#define MIC_PIN 0
double prevVolts = 100.0;

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup() {
  Serial.begin(9600);

  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  analogReference(EXTERNAL);
}

void loop() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(MIC_PIN);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht

  for (int curBin = 0; curBin < FHT_N / 2; ++curBin) {
    Serial.print(fht_log_out[curBin]);
    Serial.print(" ");
  }
  Serial.println("");
  delay(500);
}