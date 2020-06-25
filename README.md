# Prerequisites
| Hardware                      | Amount | Price |
|-------------------------------|--------|-------|
| Female Jumpers                | 40     | N/A   |
| LCD Display SSD1306 128x64    | 1      | 5,29€ |
| Raspberry Pi Zero W           | 1      | N/A   |
| Push Buttons                  | 2      | N/A   |
| LEDs (Green & Red)            | 2      | N/A   |
| Resistors                     | 4      | N/A   |
| Power Supply                  | 1      | N/A   |
| Breadboard                    | 1      | N/A   |
| MicroSD Card with GNU / Linux | 1      | N/A   |

Software
-   Raspbian https://www.raspberrypi.org/downloads/
-   Raspberry Pi Cross Compiler https://github.com/raspberrypi/tools
-   Libraries:
    -   libcurl (https://curl.haxx.se/libcurl/)
    -   ssd1306 (https://github.com/lexus2k/ssd1306)
    -   WiringPi http://wiringpi.com/reference/i2c-library/
    -   cxxopts https://github.com/jarro2783/cxxopts
    -   rapidjson https://github.com/Tencent/rapidjson

set 1000 kbit/s boadrate for Raspberry Pi 3b+
```bash
sudo echo dtparam=i2c_baudrate=1000000 > /boot/config.txt
```

Installation:
```bash
git clone git@github.com:3n16m4/covid-pi.git && cd covid-pi
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-rpi.cmake && make -j
./covid-pi
```

Features:
```bash
./covid-pi --help to list all available options.
```
