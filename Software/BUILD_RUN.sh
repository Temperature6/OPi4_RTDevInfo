gcc -Wall -o RTCPUInfo ./*.c ./SSD1306/*.c -I./SSD1306 -I./ -lwiringPi -lpthread
sudo ./RTCPUInfo