# ESP32animatedGifDMA
# add or delete Gifs whilst matrix is running.  

Animated Gifs on Led matrix with very basic ftp  

Create SPIFFS area using partitions.csv or Arduino IDE. 

Use Winscp to upload file to the SPIFFS area. Gifs are stored in src/data/gifs

WinSCP  

To force WinSCP to use the primary connection for data transfers:  
In Login, go to Tools/Preferences.../Transfer/Background,  
set "Maximum number of transfers at the same time" to 1.  

username admin  
password esp32  

port 21  

Use terminal to gain the IP address for ftp.  
Remeber you are limited to how much SPIFFS space you have, run serial terminal on ESP32
when booting you will see something like...  

Listing directory: /  

  FILE: /shock-gs.gif   SIZE: 34454  
  FILE: /donkeyKong.gif SIZE: 84973  
  FILE: /hello.gif      SIZE: 39645  

Total bytes:    1378241  
Used bytes:     161142  
Free bytes:     1217099  
