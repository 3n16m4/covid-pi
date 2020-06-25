# Prerequisites
| Hardware                      | Amount | Price |
|-------------------------------|--------|-------|
| Female Jumpers                | 40     | N/A   |
| LCD Display SSD1306 128x64    | 1      | 5,29€ |
| Speaker                       | 1      | N/A   |
| Raspberry Pi 3b+              | 1      | N/A   |
| Push Buttons                  | 2      | N/A   |
| Power Supply                  | 1      | N/A   |
| Breadboard                    | 1      | N/A   |
| MicroSD Card with GNU / Linux | 1      | N/A   |

## Example

### Debug-Build | Time: 23:59:59 | News ticker scrolling here
| Country | Confirmed | Dead    | Recovered |
|---------|-----------|---------|-----------|
| Global  | 4,339,824 | 292,808 | N/A       |
| Germany | 173,273   | 7,754   | N/A       |
| USA     | 1,411,175 | 83,552  | N/A       |

Specification:
-   only support provinces from germany for now
-   have a menu to switch between provinces (prev, next, shows the page number)
-   2 interrupt service routines for buttons (prev, next)
    -   switch between provinces (pages on menu)
    -   update lcd
    
-   while(1) loop:
    -   handle_requests()
        -   request new updates from covid every 20 min (multi interface, async)
            -   read json data when memory chunk is complete and extract the data somewhere (provinces)
            -   display the data on the display once completely fetched (update lcd)
        -   toggle LEDs while fetching data (every 250ms toggle an led and switch to next one)
        -   sleep for 20min (the button isrs (which are just posix threads) can 'interrupt' here and refresh the display i.e. show the next / prev page while the mainthread is sleeping)

Software
-   Raspbian https://www.raspberrypi.org/downloads/
-   LLVM CLang Compiler 10.0.0
-   Libraries:
    -   libcurl for requesting the covid-19 data (https://curl.haxx.se/libcurl/)
    -   ssd1306 library (https://github.com/lexus2k/ssd1306)
    -   WiringPi http://wiringpi.com/reference/i2c-library/

set 1000 kbit/s boadrate for Raspberry Pi 3b+
```bash
sudo echo dtparam=i2c_baudrate=1000000 > /boot/config.txt
```

Installation:
```bash
sudo apt install clang-9 cmake git wiringpi
mkdir src && cd src
git clone git@github.com:3n16m4/covid-pi.git && cd covid-pi
mkdir build && cd build
cmake .. && make -j
./covid-pi
```
