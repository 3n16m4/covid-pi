# Prerequisites
| Hardware                          | Amount 
|-----------------------------------|--------
| Female Jumpers                    | 16     
| LCD Display SSD1306 128x64        | 1      
| Raspberry Pi Zero W               | 1      
| Push Buttons                      | 2      
| LEDs (Green & Red)                | 2      
| 10kΩ Resistors (Buttons)          | 2      
| 220Ω Resistors (LEDs)             | 2      
| Power Supply                      | 1      
| Breadboard                        | 1      
| MicroSD Card with Raspberry Pi OS | 1      

Screenshot:

![Preview](img/Screenshot.png)

Software

*   Raspberry Pi OS https://www.raspberrypi.org/downloads/
*   Raspberry Pi Cross Compiler https://github.com/raspberrypi/tools
*   Libraries:
    -   libcurl (https://curl.haxx.se/libcurl/)
    -   ssd1306 (https://github.com/iliapenev/ssd1306_i2c/)
    -   WiringPi (http://wiringpi.com/reference/i2c-library/)
    -   cxxopts (https://github.com/jarro2783/cxxopts/)
    -   rapidjson (https://github.com/Tencent/rapidjson/)
    -   fmt (https://github.com/fmtlib/fmt/)

Enable I2C Interface:

``` bash
sudo raspi-config
5 Interfacing Options --> P5 I2C --> Yes
```

Faster I2C display refresh rates:
set 1000 kbit/s boadrate for Raspberry Pi

``` bash
sudo echo dtparam=i2c_baudrate=1000000 > /boot/config.txt
```

Installation:

``` bash
git clone git@github.com:3n16m4/covid-pi.git && cd covid-pi
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-rpi.cmake && make -j
./covid-pi
```

Features:

``` bash
pi@raspberrypi:~/src/covid-pi/build $ ./covid-pi --help
A covid-19 live tracker.
Usage:
  ./covid-pi [OPTION...]

  -h, --help                 Print usage
  -c, --cities alpha-2 code  Filter by country and show its cities
  -s, --sort low / high      Sort by confirmed cases.
```
