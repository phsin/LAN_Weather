// ========================Задаем необходимые библиотеки================================
#include <SPI.h>
#include <String.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);
// Адреса и имена датчиков
DeviceAddress sensor1 = {0x28, 0xFF, 0x14, 0x5A, 0x2C, 0x04, 0x00, 0xFC};
DeviceAddress sensor2 = {0x28, 0xFF, 0xA1, 0x3B, 0x2B, 0x04, 0x00, 0x04};

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

  sensors.begin();
  // Тревога если температура c sensor1 выше 33C
  //sensors.setHighAlarmTemp(sensor1, 33);
  // Тревога если температура меньше -10C
  //sensors.setLowAlarmTemp(insideThermometer, -10);
  // тревога если температура с sensor2 > 31C
  //sensors.setHighAlarmTemp(sensor2, 31);
  // Тревога если температура на sensor2 < 27C
  //sensors.setLowAlarmTemp(sensor2, -30);

  Serial.print("Server at ");
  Serial.println(Ethernet.localIP());

}
// ===================================================================================
void loop() {
  // ========================ДАТЧИКИ==================================================
  sensors.requestTemperatures();

  /*
  Serial.print("Sensor 1: ");
  Serial.print(sensors.getTempC(sensor1));
  Serial.println("C");
  Serial.print("Sensor 2: ");
  Serial.print(sensors.getTempC(sensor2));
  Serial.println("C");
  Serial.println();
  */

  // =============================================================================
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
          //Проверяем включили ли светодиод?
          //Level=1 - включен
          //Level=0 - выключен
          if (readString.indexOf("Level=1") >= 0) {
            //Включаем светодиод
            digitalWrite(ledPin, HIGH); // set the LED on
            LEDON = true;
          } else {
            //Выключаем светодиод
            digitalWrite(ledPin, LOW); // set the LED OFF
            LEDON = false;
          }
          // =============Формируем HTML-страницу=================================================
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<head> ");
          client.println("<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> ");
          client.println("<title> :: Упр.Arduino:: V1.1</title>");
          client.println("</head> ");
          client.println("<body>");
          /*
          client.println("<hr />");
          client.println("<h1> ::Упр.Arduino:: </h1>");
          if (LEDON){
            client.println("<form method=get name=LED><input type=radio name=Level value=1 CHECKED>Включить<input type=radio name=Level value=0>Выключить<input type=submit value=OK></form>");
            client.println("<font size=’5′>LED-статус: ");
            client.println("<font size=’5′>Вкл.");
          }else{
            client.println("<form method=get name=LED><input type=radio name=Level value=1>Включить<input type=radio name=Level value=0 CHECKED>Выключить<input type=submit value=OK></form>");
            client.println("<font size=’5′>LED-статус: ");
            client.println("<font size=’5′>Выкл");
          }
          //==============Вывод значений на web-страницу============================
              client.println("<hr />");//линия=====================================
             */
          client.print("temp 1 = ");    //Температура с 1датчика
          client.print(sensors.getTempC(sensor1));
          client.println(" *C");
          client.println("<br> "); //перенос на след. строчку
          client.print("temp 2 = ");    //Температура со 2 датчика
          client.print(sensors.getTempC(sensor2));
          client.println(" *C");
          client.println("<br> "); //перенос на след. строчку
          client.println("<hr />");  //линия=====================================
          
          /*
          //====================================================================
          if (sensors.hasAlarm(sensor1))
          {
            client.println("Тревога на 1 датчике!!!");    //Проверка на повышение порога тревоги 1 датчика
            client.println("<br> ");
            Serial.print("ALARM: Sensor1 ");
            Serial.println();
          }
          else
          {
            client.println("Температура на 1 датчике в норме!");
            client.println("<br> ");
            Serial.print("ALARM: Sensor1 not found");
            Serial.println();
          }
          if (sensors.hasAlarm(sensor2)) //Проверка на повышение порога тревоги 2 датчика
          {
            client.println("Тревога на 2 датчике!!!");
            client.println("<br> ");
            client.println("<hr />");
            Serial.print("ALARM: Sensor2 ");
            Serial.println();
          }
          else
          {
            client.println("Температура на 2 датчике в норме!");
            client.println("<br> ");
            client.println("<hr />");
            Serial.print("ALARM: Sensor2 not found");
            Serial.println();
          }
          *///
          
          client.println("</body></html>");
          //очищаем строку для следующего считывания
          //==============Останавливаем web-client===============================
          readString = "";
          client.stop();
          //====================================================================
        }
      }
    }
  }
}

