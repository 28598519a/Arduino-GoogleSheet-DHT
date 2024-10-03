# Arduino-GoogleSheet-DHT
Use WeMos D1 R3 development board (esp8266) to upload temperature and humidity data to GoogleSheet every 1 hour

## Hardware
- WeMos D1 R3
- DHT22

### Circuit diagram
![](https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/main/WeMos_DHT_Pin.png)

## Arduino Library
### Board
- esp8266 v3.1.2
![](https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/main/Library/ESP8266.png)

### Library
- Copy `Libray/HTTPSRedirect` in to `C:/Users/<username>/Documents/Arduino/`

- DHT sensor library v1.4.6
![](https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/main/Library/DHT22.png)

## GoogleSheet.gs

docs.google.com/**spreadsheetId**/d/spreadsheetId/edit#gid=**sheetId**

https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/70c024151d39a7539bd5f3de63d751406d5f70dc/GoogleSheet.gs#L1
- 填入spreadsheetId

https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/70c024151d39a7539bd5f3de63d751406d5f70dc/GoogleSheet.gs#L21
- "" + WeMosD1 DeviceID = sheetId

- Google Apps Script (Deploy):<br>
https://www.youtube.com/watch?v=-AlstV1PAaA

- https://support.google.com/docs/table/25273#query=offset<br>
Tips. 可以建一個工作表用來顯示最新數值，以下是用法<br>
=OFFSET(sheetId!$A1,1,0)<br>
=OFFSET(sheetId!$B1,1,0)

## WeMosD1.ino
https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/70c024151d39a7539bd5f3de63d751406d5f70dc/WeMosD1.ino#L18
- 填入WiFi ID與密碼

https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/70c024151d39a7539bd5f3de63d751406d5f70dc/WeMosD1.ino#L27
- 填入Google Apps Script部署為網路應用程式拿到的ID

https://github.com/28598519a/Arduino-GoogleSheet-DHT/blob/70c024151d39a7539bd5f3de63d751406d5f70dc/WeMosD1.ino#L28
- WeMosD1 DeviceID
