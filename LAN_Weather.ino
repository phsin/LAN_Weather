// ========================Задаем необходимые библиотеки================================
#include <SPI.h>
#include <String.h>
#include <Ethernet.h>
#include <OneWire.h>
//#include <DallasTemperature.h>
// ====================================================================================
// ========================Задаем данные сети==========================================
byte mac[] = {0x00, 0x13, 0xCD, 0xAB, 0x44, 0x22}; //mac - адрес ethernet shielda
byte ip[] = { 192, 168, 52, 99 };        // ip адрес ethernet shielda
byte subnet[] = { 255, 255, 255, 0 }; //маска подсети
EthernetServer server(80); //порт сервера
int ledPin = 4; // указываем что светодиод будет управляться через 4 Pin
String readString = String(30); //string for fetching data from address
boolean LEDON = false; //изначальный статус светодиода - выключен
// Датчики ds18b20 подключены на 2 пин
OneWire ds(2);

/*
DallasTemperature sensors(&oneWire);
// Адреса и имена датчиков
DeviceAddress sensor1 = {0x28, 0xFF, 0x14, 0x5A, 0x2C, 0x04, 0x00, 0xFC};
DeviceAddress sensor2 = {0x28, 0xFF, 0xA1, 0x3B, 0x2B, 0x04, 0x00, 0x04};
*/

// ========================СТАРТУЕМ===================================================
// ========================Управляем св.диодом на 4-м пине============================
void setup() {
  //запускаем Ethernet
  Ethernet.begin(mac, ip, subnet);

  //устанавливаем pin 4 на выход
  pinMode(ledPin, OUTPUT);
  //Серийный порт для отладки
  Serial.begin(9600);
  Serial.println("Port Test!"); // Тестовые строки для отображения в мониторе порта
  Serial.println("GO!");// Тестовые строки для отображения в мониторе порта

  //sensors.begin();

  Serial.print("Server at ");
  Serial.println(Ethernet.localIP());

}

// ===================================================================================
void loop() {
  byte i, present = 0;
  boolean findNext;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  // =============Создаем клиентское соединение====================================
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      //Serial.println("Client connected");
      if (client.available()) {
        char c = client.read();
        //read char by char HTTP request
        if (readString.length() < 30) {
          //store characters to string
          readString.concat( c);
        }
        //output chars to serial port
        Serial.print( c);
        //if HTTP request has ended
        if (c == '\n') {

          // =============Формируем HTML-страницу=================================================
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Refresh: 10");
          client.println();
          client.println("<head> ");
          client.println("<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> ");
          client.println("<title> :: Мониторинг температуры :: V1.1</title>");
          client.println("</head> ");
          client.println("<body>");

          //Последовательный опрос всех датчиков

          findNext = ds.search(addr);
          while (findNext) {
            client.print("ROM = [");
            for ( i = 0; i < 8; i++) {
              client.print(addr[i], HEX);
            }

            if (OneWire::crc8(addr, 7) != addr[7]) {
              client.println("CRC is not valid!");
              return;
            }
            client.print("]");

            ds.reset();
            ds.select(addr);
            ds.write(0x44, 1);        // start conversion, with parasite power on at the end

            delay(1000);     // maybe 750ms is enough, maybe not
            // we might do a ds.depower() here, but the reset will take care of it.

            present = ds.reset();
            ds.select(addr);
            ds.write(0xBE);         // Read Scratchpad

            //  Serial.print("  Data = ");
            //  Serial.print(present, HEX);
            //  Serial.print(" ");
            for ( i = 0; i < 9; i++) {           // we need 9 bytes
              data[i] = ds.read();
              //Serial.print(data[i], HEX);
              //Serial.print(" ");
            }
            //  Serial.print(" CRC=");
            //  Serial.print(OneWire::crc8(data, 8), HEX);
            //  Serial.println();

            // Convert the data to actual temperature
            // because the result is a 16 bit signed integer, it should
            // be stored to an "int16_t" type, which is always 16 bits
            // even when compiled on a 32 bit processor.
            int16_t raw = (data[1] << 8) | data[0];
            if (type_s) {
              raw = raw << 3; // 9 bit resolution default
              if (data[7] == 0x10) {
                // "count remain" gives full 12 bit resolution
                raw = (raw & 0xFFF0) + 12 - data[6];
              }
            } else {
              byte cfg = (data[4] & 0x60);
              // at lower res, the low bits are undefined, so let's zero them
              if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
              else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
              else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
              //// default is 12 bit resolution, 750 ms conversion time
            }
            celsius = (float)raw / 16.0;
            fahrenheit = celsius * 1.8 + 32.0;
            client.print("  Temperature = ");
            client.print(celsius);
            client.println(" Celsius ");
            client.println("<br> "); //перенос на след. строчку

            findNext = ds.search(addr);

          }

          client.println("<hr />");  //линия=====================================
          Serial.println("No more addresses.");
          Serial.println();
          ds.reset_search();
          client.println("</body></html>");
          //очищаем строку для следующего считывания
          //==============Останавливаем web-client===============================
          readString = "";
          client.stop();
          //====================================================================

          //  delay(2500);
        }
      }
    }
  }
}

