![PROJECT_PHOTO](https://github.com/AlexGyver/FHTSpectrumAnalyzer/blob/master/proj_img.jpg)
# Графический анализатор аудио спектра на Arduino
* [Описание проекта](#chapter-0)
* [Папки проекта](#chapter-1)
* [Схемы подключения](#chapter-2)
* [Материалы и компоненты](#chapter-3)
* [Как скачать и прошить](#chapter-4)
* [FAQ](#chapter-5)
* [Полезная информация](#chapter-6)
[![AlexGyver YouTube](http://alexgyver.ru/git_banner.jpg)](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)

<a id="chapter-0"></a>
## Описание проекта
Графоанализатор спектра с кучей настроек и возможностей
- Вывод спектра на:
  - Дисплей 1602
  - Матрица из 4х блоков 8х8 (MAX7219)
  - Матрица адресных WS2812
- Настройка яркости
- Настройка цветовой гаммы (для WS2812)
- Настройка усиления и подавления шумов
- Настройка плавности анимации
- Настройка громкости:
  - Фиксированная
  - С потенциометра
  - Автоматическая
- Точки максимума
  - Вкл выкл
  - Время зависания
  - Скорость падения
- Ручная выборка по частотам
- Подробности в видео: https://youtu.be/xMdRmrXdSxU

<a id="chapter-1"></a>
## Папки
**ВНИМАНИЕ! Если это твой первый опыт работы с Arduino, читай [инструкцию](#chapter-4)**
- **libraries** - библиотеки проекта. Заменить имеющиеся версии
- **Firmware** - прошивка для Arduino, файлы в папках открыть в Arduino IDE ([инструкция](#chapter-4))
  - **spektrumFHT** - "голая" прошивка для вывода спектра
  - **spertrum1602** - анализатор с дисплеем 1602
  - **spertrumMatrix_MAX7219** - анализатор с матрицей 8х32
  - **spertrumWS2812_16x16_full** - анализатор с цветной матрицей 16х16
- **schemes** - схемы

<a id="chapter-2"></a>
## Схемы
![SCHEME](https://github.com/AlexGyver/FHTSpectrumAnalyzer/blob/master/Schemes/simple.png)
![SCHEME](https://github.com/AlexGyver/FHTSpectrumAnalyzer/blob/master/Schemes/8x32.png)

<a id="chapter-3"></a>
## Материалы и компоненты
### Ссылки оставлены на магазины, с которых я закупаюсь уже не один год
* Arduino NANO 328p – искать
* https://ali.ski/tI7blh
* https://ali.ski/O4yTxb
* https://ali.ski/6_rFIS
* https://ali.ski/gb92E-
* Giant4 (Россия)
* Макетная плата и провода https://ali.ski/w8AFTm
* Дисплей 1602
* https://ali.ski/2VMIU
* https://ali.ski/4fLT6w
* Матрица MAX7219 – искать
* https://ali.ski/URaNlv
* https://ali.ski/id65D
* Матрица 16×16 – искать
* Giant4
* https://ali.ski/pOMcck
* https://ali.ski/nSJCP
* https://ali.ski/hI6tov
* https://ali.ski/ZSliU7

## Вам скорее всего пригодится
* [Всё для пайки (паяльники и примочки)](http://alexgyver.ru/all-for-soldering/)
* [Недорогие инструменты](http://alexgyver.ru/my_instruments/)
* [Все существующие модули и сенсоры Arduino](http://alexgyver.ru/arduino_shop/)
* [Электронные компоненты](http://alexgyver.ru/electronics/)
* [Аккумуляторы и зарядные модули](http://alexgyver.ru/18650/)

<a id="chapter-4"></a>
## Как скачать и прошить
* [Первые шаги с Arduino](http://alexgyver.ru/arduino-first/) - ультра подробная статья по началу работы с Ардуино, ознакомиться первым делом!
* Скачать архив с проектом
> На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**
* Установить библиотеки в  
`C:\Program Files (x86)\Arduino\libraries\` (Windows x64)  
`C:\Program Files\Arduino\libraries\` (Windows x86)
* Подключить Ардуино к компьютеру
* Запустить файл прошивки (который имеет расширение .ino)
* Настроить IDE (COM порт, модель Arduino, как в статье выше)
* Настроить что нужно по проекту
* Нажать загрузить
* Пользоваться  

## Настройки в коде (пример для WS2812)
    // матрица
    #define WIDTH 16          // ширина матрицы (число диодов)
    #define HEIGHT 16         // высота матрицы (число диодов)
    #define BRIGHTNESS 20     // яркость (0 - 255)

    // цвета высоты полос спектра. Длины полос задаются примерно в строке 95
    #define COLOR1 CRGB::Green
    #define COLOR2 CRGB::Yellow
    #define COLOR3 CRGB::Orange
    #define COLOR4 CRGB::Red

    // сигнал
    #define INPUT_GAIN 1.5    // коэффициент усиления входного сигнала
    #define LOW_PASS 35       // нижний порог чувствительности шумов (нет скачков при отсутствии звука)
    #define MAX_COEF 1.2      // коэффициент, который делает "максимальные" пики чуть меньше максимума, для более приятного восприятия
    #define NORMALIZE 0       // нормализовать пики (столбики низких и высоких частот будут одинаковой длины при одинаковой громкости) (1 вкл, 0 выкл)

    // анимация
    #define SMOOTH 0.3        // плавность движения столбиков (0 - 1)
    #define DELAY 4           // задержка между обновлениями матрицы (периодичность основного цикла), миллиисекунды

    // громкость
    #define DEF_GAIN 70       // максимальный порог по умолчанию (при MANUAL_GAIN или AUTO_GAIN игнорируется)
    #define MANUAL_GAIN 0     // ручная настройка потенциометром на громкость (1 вкл, 0 выкл)
    #define AUTO_GAIN 1       // автонастройка по громкости (экспериментальная функция) (1 вкл, 0 выкл)

    // точки максимума
    #define MAX_DOTS 1        // включить/выключить отрисовку точек максимума (1 вкл, 0 выкл)
    #define MAX_COLOR CRGB::Red // цвет точек максимума
    #define FALL_DELAY 50     // скорость падения точек максимума (задержка, миллисекунды)
    #define FALL_PAUSE 700    // пауза перед падением точек максимума, миллисекунды

    // массив тонов, расположены примерно по параболе. От 80 Гц до 16 кГц
    byte posOffset[17] = {2, 3, 4, 6, 8, 10, 12, 14, 16, 20, 25, 30, 35, 60, 80, 100, 120};

<a id="chapter-5"></a>
## FAQ
### Основные вопросы
В: Как скачать с этого грёбаного сайта?  
О: На главной странице проекта (где ты читаешь этот текст) вверху справа зелёная кнопка **Clone or download**, вот её жми, там будет **Download ZIP**

В: Скачался какой то файл .zip, куда его теперь?  
О: Это архив. Можно открыть стандартными средствами Windows, но думаю у всех на компьютере установлен WinRAR, архив нужно правой кнопкой и извлечь.

В: Я совсем новичок! Что мне делать с Ардуиной, где взять все программы?  
О: Читай и смотри видос http://alexgyver.ru/arduino-first/

В: Компьютер никак не реагирует на подключение Ардуины!  
О: Возможно у тебя зарядный USB кабель, а нужен именно data-кабель, по которому можно данные передавать

В: Ошибка! Скетч не компилируется!  
О: Путь к скетчу не должен содержать кириллицу. Положи его в корень диска.

В: Сколько стоит?  
О: Ничего не продаю.

### Вопросы по этому проекту

<a id="chapter-6"></a>
## Полезная информация
* [Мой сайт](http://alexgyver.ru/)
* [Основной YouTube канал](https://www.youtube.com/channel/UCgtAOyEQdAyjvm9ATCi_Aig?sub_confirmation=1)
* [YouTube канал про Arduino](https://www.youtube.com/channel/UC4axiS76D784-ofoTdo5zOA?sub_confirmation=1)
* [Мои видеоуроки по пайке](https://www.youtube.com/playlist?list=PLOT_HeyBraBuMIwfSYu7kCKXxQGsUKcqR)
* [Мои видеоуроки по Arduino](http://alexgyver.ru/arduino_lessons/)