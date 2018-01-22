// ---------------- НАСТРОЙКИ ----------------
#define DRIVER_VERSION 0   // 0 - маркировка драйвера кончается на 4АТ, 1 - на 4Т
#define GAIN_CONTROL 0     // ручная настройка потенциометром на громкость (1 - вкл, 0 - выкл)

#define AUTO_GAIN 0       // автонастройка по громкости (экспериментальная функция)
#define VOL_THR 35        // порог тишины (ниже него отображения на матрице не будет)

#define LOW_PASS 30        // нижний порог чувствительности шумов (нет скачков при отсутствии звука)
#define DEF_GAIN 80       // максимальный порог по умолчанию (при GAIN_CONTROL игнорируется)

#define FHT_N 256          // ширина спектра х2
// вручную забитый массив тонов, сначала плавно, потом круче
byte posOffset[16] = {2, 3, 4, 6, 8, 10, 12, 14, 16, 20, 25, 30, 35, 60, 80, 100};
// ---------------- НАСТРОЙКИ ----------------

// ---------------------- ПИНЫ ----------------------
#define AUDIO_IN 0          // пин, куда подключен звук
#define POT_PIN 7          // пин потенциометра настройки
// ---------------------- ПИНЫ ----------------------

// --------------- БИБЛИОТЕКИ ---------------
#define LOG_OUT 1
#include <FHT.h>          // преобразование Хартли
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define printByte(args) write(args);
double prevVolts = 100.0;
// --------------- БИБЛИОТЕКИ ---------------
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// --------- АВТОВЫБОР ОПРЕДЕЛЕНИЯ ДИСПЛЕЯ -------------
// Если кончается на 4Т - это 0х27. Если на 4АТ - 0х3f
#if (DRIVER_VERSION)
LiquidCrystal_I2C lcd(0x27, 16, 2);
#else
LiquidCrystal_I2C lcd(0x3f, 16, 2);
#endif
// --------- АВТОВЫБОР ОПРЕДЕЛЕНИЯ ДИСПЛЕЯ -------------

// ------------------------------------- ПОЛОСОЧКИ -------------------------------------
byte v1[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111};
byte v2[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111};
byte v3[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111};
byte v4[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111};
byte v5[8] = {0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
byte v6[8] = {0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
byte v7[8] = {0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
byte v8[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
// ------------------------------------- ПОЛОСОЧКИ -------------------------------------
byte gain = DEF_GAIN;   // усиление по умолчанию
unsigned long gainTimer;
byte maxValue, maxValue_f;
float k = 0.1;

void setup() {
  // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
  // Котельникова (Найквиста) частота дискретизации будет 19 кГц
  // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  // для увеличения точности уменьшаем опорное напряжение,
  // выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
  // GND ---[2х10 кОм] --- REF --- [10 кОм] --- 3V3
  analogReference(EXTERNAL);

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AlexGyver");
  lcd.setCursor(0, 1);
  lcd.print("SpektrumAnalyzer");
  lcdChars();   // подхватить коды полосочек
  delay(2000);
}

void loop() {
  // если разрешена ручная настройка уровня громкости
  if (GAIN_CONTROL) gain = map(analogRead(POT_PIN), 0, 1023, 0, 150);

  analyzeAudio();   // функция FHT, забивает массив fht_log_out[] величинами по спектру

  for (int pos = 0; pos < 16; pos++) {   // для окошек дисплея с 0 по 15
    // найти максимум из пачки тонов
    if (fht_log_out[posOffset[pos]] > maxValue) maxValue = fht_log_out[posOffset[pos]];

    lcd.setCursor(pos, 0);

    // преобразовать значение величины спектра в диапазон 0..15 с учётом настроек
    int posLevel = map(fht_log_out[posOffset[pos]], LOW_PASS, gain, 0, 15);
    posLevel = constrain(posLevel, 0, 15);

    if (posLevel > 7) {               // если значение больше 7 (значит нижний квадратик будет полный)
      lcd.printByte(posLevel - 8);    // верхний квадратик залить тем что осталось
      lcd.setCursor(pos, 1);          // перейти на нижний квадратик
      lcd.printByte(7);               // залить его полностью
    } else {                          // если значение меньше 8
      lcd.print(" ");                 // верхний квадратик пустой
      lcd.setCursor(pos, 1);          // нижний квадратик
      lcd.printByte(posLevel);        // залить полосками
    }
  }

  if (AUTO_GAIN) {
     maxValue_f = maxValue * k + maxValue_f * (1 - k);
    if (millis() - gainTimer > 1500) {      // каждые 1500 мс
      // если максимальное значение больше порога, взять его как максимум для отображения
      if (maxValue_f > VOL_THR) gain = maxValue_f;

      // если нет, то взять порог побольше, чтобы шумы вообще не проходили
      else gain = 100;
      gainTimer = millis();
    }
  }
}

void lcdChars() {
  lcd.createChar(0, v1);
  lcd.createChar(1, v2);
  lcd.createChar(2, v3);
  lcd.createChar(3, v4);
  lcd.createChar(4, v5);
  lcd.createChar(5, v6);
  lcd.createChar(6, v7);
  lcd.createChar(7, v8);
}
void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(AUDIO_IN);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
}
