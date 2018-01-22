// -------------------------------- НАСТРОЙКИ --------------------------------
// матрица
#define BRIGHTNESS 15     // яркость матрицы (0 - 15)

// сигнал
#define INPUT_GAIN 1.5    // коэффициент усиления входного сигнала
#define LOW_PASS 35       // нижний порог чувствительности шумов (нет скачков при отсутствии звука)
#define MAX_COEF 1.1      // коэффициент, который делает "максимальные" пики чуть меньше максимума, для более приятного восприятия
#define NORMALIZE 0       // нормализовать пики (столбики низких и высоких частот будут одинаковой длины при одинаковой громкости) (1 вкл, 0 выкл)

// анимация
#define SMOOTH 0.4        // плавность движения столбиков (0 - 1)
#define DELAY 4           // задержка между обновлениями матрицы (периодичность основного цикла), миллиисекунды

// громкость
#define DEF_GAIN 100       // максимальный порог по умолчанию (при MANUAL_GAIN или AUTO_GAIN игнорируется)
#define MANUAL_GAIN 0     // ручная настройка потенциометром на громкость (1 вкл, 0 выкл)
#define AUTO_GAIN 1       // автонастройка по громкости (экспериментальная функция) (1 вкл, 0 выкл)

// точки максимума
#define MAX_DOTS 1        // включить/выключить отрисовку точек максимума (1 вкл, 0 выкл)
#define FALL_DELAY 50     // скорость падения точек максимума (задержка, миллисекунды)
#define FALL_PAUSE 700    // пауза перед падением точек максимума, миллисекунды

// массив тонов, расположены примерно по параболе. От 80 Гц до 16 кГц
byte posOffset[33] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20, 25, 32, 36, 40, 44, 48, 52, 57, 62, 67, 72, 78, 84, 90, 96, 102, 108, 120
};
// -------------------------------- НАСТРОЙКИ --------------------------------

// ---------------------- ПИНЫ ----------------------
// для увеличения точности уменьшаем опорное напряжение,
// выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
// GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3

// Матрица: CS -> 10 / CLK -> 13 / DIN -> 11
#define AUDIO_IN 0         // пин, куда подключен звук
#define POT_PIN 7          // пин потенциометра настройки (если нужен MANUAL_GAIN)
// ---------------------- ПИНЫ ----------------------

// --------------- ДЛЯ РАЗРАБОТЧИКОВ ---------------
#define MATR_NUM 4        // количество матриц последовательно
#define FHT_N 256         // ширина спектра х2
#define LOG_OUT 1
#include <FHT.h>           // преобразование Хартли
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
Max72xxPanel matrix = Max72xxPanel(10, 4, 1);

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

int gain = DEF_GAIN;   // усиление по умолчанию
unsigned long gainTimer, fallTimer;
byte maxValue;
float k = 0.05, maxValue_f = 0.0;
int maxLevel[32];
byte posLevel_old[32];
unsigned long timeLevel[32], mainDelay;
boolean fallFlag;
// --------------- ДЛЯ РАЗРАБОТЧИКОВ ---------------

void setup() {
  // поднимаем частоту опроса аналогового порта до 38.4 кГц, по теореме
  // Котельникова (Найквиста) максимальная частота дискретизации будет 19 кГц
  // http://yaab-arduino.blogspot.ru/2015/02/fast-sampling-from-analog-input.html
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  analogReference(EXTERNAL);

  Serial.begin(9600);
  matrix.setIntensity(BRIGHTNESS);
  for (byte i = 0; i < MATR_NUM; i++) {
    // матрицы расположены криво, здесь поворачиваем
    matrix.setRotation(i, 1);
  }
  matrix.fillScreen(LOW);
  matrix.write();
}

void loop() {
  if (millis() - mainDelay > DELAY - 2) {     // итерация главного цикла
    mainDelay = millis();

    analyzeAudio();   // функция FHT, забивает массив fht_log_out[] величинами по спектру

    for (int i = 0; i < 128; i ++) {
      // вот здесь сразу фильтруем весь спектр по минимальному LOW_PASS
      if (fht_log_out[i] < LOW_PASS) fht_log_out[i] = 0;

      // усиляем сигнал
      fht_log_out[i] = (float)fht_log_out[i] * INPUT_GAIN;

      // уменьшаем громкость высоких частот (пропорционально частоте) если включено
      if (NORMALIZE) fht_log_out[i] = (float)fht_log_out[i] / ((float)1 + (float)i / 128);
    }
    maxValue = 0;
    matrix.fillScreen(LOW);
    delay(2);///////////////////СУКАДИЛЕЙ//////БЕЗ НЕГО МАТРИЦА НАЧИНАЕТ КОСОЖОПИТЬ///////////
    for (byte pos = 0; pos < MATR_NUM * 8; pos++) {    // для кажого столбца матрицы
      int posLevel = fht_log_out[posOffset[pos]];
      byte linesBetween;
      if (pos > 0 && pos < MATR_NUM * 8) {
        linesBetween = posOffset[pos] - posOffset[pos - 1];
        for (byte i = 0; i < linesBetween; i++) {  // от предыдущей полосы до текущей
          posLevel += (float) ((float)i / linesBetween) * fht_log_out[posOffset[pos] - linesBetween + i];
        }
        linesBetween = posOffset[pos + 1] - posOffset[pos];
        for (byte i = 0; i < linesBetween; i++) {  // от предыдущей полосы до текущей
          posLevel += (float) ((float)i / linesBetween) * fht_log_out[posOffset[pos] + linesBetween - i];
        }
      }
      // найти максимум из пачки тонов
      if (posLevel > maxValue) maxValue = posLevel;

      // фильтрация длины столбиков, для их плавного движения
      posLevel = posLevel_old[pos] * SMOOTH + posLevel * (1 - SMOOTH);
      posLevel_old[pos] = posLevel;

      // преобразовать значение величины спектра в диапазон 0..7 с учётом настроек
      posLevel = map(posLevel, LOW_PASS, gain, 0, 7);
      posLevel = constrain(posLevel, 0, 7);

      // если столбик больше нуля, то начать линию с координаты (pos, 7) и рисовать в (pos, 7 - значение)
      if (posLevel > 0) matrix.drawLine(pos, 7, pos, 7 - posLevel, HIGH);

      if (posLevel > 0 && posLevel > maxLevel[pos]) {    // если для этой полосы есть максимум, который больше предыдущего
        maxLevel[pos] = posLevel;                        // запомнить его
        timeLevel[pos] = millis();                       // запомнить время
      }

      // если точка максимума выше нуля (или равна ему) - включить пиксель
      if (MAX_DOTS && maxLevel[pos] >= 0) matrix.drawPixel(pos, 7 - maxLevel[pos], HIGH);

      if (fallFlag) {                                           // если падаем на шаг
        if ((long)millis() - timeLevel[pos] > FALL_PAUSE) {     // если максимум держался на своей высоте дольше FALL_PAUSE
          if (maxLevel[pos] >= 0) maxLevel[pos]--;              // уменьшить высоту точки на 1
          // внимание! Принимает минимальное значение -1 !
        }
      }
    }
    matrix.write();  // отправить на матрицу

    fallFlag = 0;                                 // сбросить флаг падения
    if (millis() - fallTimer > FALL_DELAY) {      // если настало время следующего падения
      fallFlag = 1;                               // поднять флаг
      fallTimer = millis();
    }

    // если разрешена ручная настройка уровня громкости
    if (MANUAL_GAIN) gain = map(analogRead(POT_PIN), 0, 1023, 0, 150);

    // если разрешена авто настройка уровня громкости
    if (AUTO_GAIN) {
      if (millis() - gainTimer > 10) {      // каждые 10 мс
        maxValue_f = maxValue * k + maxValue_f * (1 - k);
        // если максимальное значение больше порога, взять его как максимум для отображения
        if (maxValue_f > LOW_PASS) gain = (float) MAX_COEF * maxValue_f;
        // если нет, то взять порог побольше, чтобы шумы вообще не проходили
        else gain = 100;
        gainTimer = millis();
      }
    }
  }
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

/*
   Алгоритм работы:
   Анализ спектра, на выходе имеем массив величин полос спектра (128 полос)
   Фильтрация по нижним значениям для каждой полосы (128 полос)
   Переход от 128 полос к 16 полосам с сохранением межполосных значений по линейной зависимости
   Поиск максимумов для коррекции высоты столбиков на матрице
   Перевод чистого "веса" полосы к высоте матрицы
   Отправка полос на матрицу
   Расчёт позиций точек максимума и отправка их на мтарицу

   Мимоходом фильтрация верхних пиков, коррекция высоты столбиков от громкости и прочее
*/
