#include <DHT.h>

/**************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 *
 * This example shows how to use Arduino Ethernet shield (W5100)
 * to connect your project to Blynk.
 * Feel free to apply it to any other example. It's simple!
 *
 * NOTE: Pins 10, 11, 12 and 13 are reserved for Ethernet module.
 *       DON'T use them in your sketch directly!
 *
 * WARNING: If you have an SD card, you may need to disable it
 *       by setting pin 4 to HIGH. Read more here:
 *       https://www.arduino.cc/en/Main/ArduinoEthernetShield
 *
 **************************************************************/

//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <SPI.h>
#include <Ethernet.h>
//#include <BlynkSimpleEthernet.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <Adafruit_BMP085.h>

#define DHTPIN 5 // номер пина, к которому подсоединен датчик

// Раскомментируйте в соответствии с используемым датчиком

// Инициируем датчик

DHT dht(DHTPIN, DHT22);

SoftwareSerial mySerial(2, 4); // RX, TX


// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here

Adafruit_BMP085 bmp;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "fc0a29d04a6d4d159cc6292da300d293";
char auth[] = "0a796fc5b09a420a925c0b9566bd5d61";

#define W5100_CS  10
//#define SDCARD_CS 4

#define TEMP_PIN 2
#define HUM_PIN 3

#define TEMP_PIN2 4
#define PRES_PIN 5

unsigned long interval = 360000;//6*60*1000 6 минут в миллисекундах - период опроса датчика
unsigned long webInterval = 30000; //30*1000 30 секунд в миллисекундах
unsigned long predMillis;
unsigned long period =0; //оставшееся время до срабатывания таймера датчика
unsigned long webPeriod =0; //оставшееся время до срабатывания таймера опроса сервера

EthernetClient client;
IPAddress server(192,168,1,10);
IPAddress ip(192, 168, 1, 11);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


void setup()
{
  Serial.begin(115200);

  
  mySerial.begin(9600);  //Скорость порта для связи Arduino с управляемым модулем
 // pinMode(SDCARD_CS, OUTPUT);
 // digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card

 if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
 

 dht.begin();
 Wire.begin();

  
 // Blynk.begin(auth);
  // You can also specify server.
  // For more options, see Boards_Ethernet/Arduino_Ethernet_Manual example
  //Blynk.begin(auth, "your_server.com", 8442);
 // Blynk.begin(auth, IPAddress(192,168,1,10), 8442);
  
  predMillis=millis();

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
}

void loop()
{
 // Blynk.run();
  //*****************таймер датчика*********************
  unsigned long nowMillis = millis();//записываем текущее время
  unsigned long moment;//переменная для расчета периода, прошедьшего с прошлого цикла
  if (nowMillis<predMillis)//переполнение миллис, 
  {
    //moment=0xFFFFFFFF-predMillis+nowMillis; 
    moment=nowMillis;  //пренебрегаем временем прошедьшим с момента прошлого цикла до момента переполнения счтечика миллис  
  }
  else
  {
    moment=nowMillis-predMillis;    
  }
  delay(1);//задержка на всякий случай
 // Serial.write("период ");Serial.write(moment);Serial.write(" оставшееся время ");Serial.println(period);
  predMillis=nowMillis; 
  if (period<= moment)
  {
    //час настал, делаем дело  
    period=interval;
    DHTask();    
  }
  else
  {
    // уменьшаем значение периода
    period=period-moment;  
  }
  //end************таймер датчика************************
  //***************таймер опроса заданий с сервера******
   if (webPeriod<= moment)
  {
    //час настал, делаем дело  
    webPeriod=webInterval;
    GetMission();    
  }
  else
  {
    // уменьшаем значение периода
    webPeriod=webPeriod-moment;  
  } 
   //end***************таймер опроса заданий с сервера******  
  
  
}

void execMission(String id, String command)
{
  if (command.equals("COND_ON"))
  {
    mySerial.write("START_COND");/*
    Wire.begin();
    Wire.beginTransmission(8); // transmit to device #8
    Wire.write("START_COND");       
    Wire.endTransmission();    // stop transmitting*/
  
  }
  if (command.equals("COND_OFF"))
  {
    mySerial.write("COND OFF");/*
    Wire.begin();
    Wire.beginTransmission(8); // transmit to device #8
    Wire.write("COND_OFF");       
    Wire.endTransmission();    // stop transmitting*/
  
  }
  if (client.connect(server, 8003)) {
      Serial.println("connected");
      // Make a HTTP request:
      client.print("GET /action.php?action=done&mission_id=");
      client.print(id);
      
      client.println(" HTTP/1.1");   
      client.println("Host: 192.168.1.10");
      client.println("User-Agent: arduino-web-client");
      client.println("Connection: close");
      client.println();
      delay(200);
      while (client.available()) {
       char c = client.read();
       Serial.print(c);
      }
      client.stop();
  }    

}





void GetMission()
{
  if (client.connect(server, 8003)) {
      Serial.println("connected");
      // Make a HTTP request:
      client.print("GET /get_mission.php?dev_id=100010001");
      client.println(" HTTP/1.1");   
      client.println("Host: 192.168.1.10");
      client.println("User-Agent: arduino-web-client");
      client.println("Connection: close");
      client.println();
      delay(200);
      String ans="";
      char f="";
      while (client.available()) {
        f=client.read();
       ans+=String((char)f);
       Serial.print(f);
       
      }
      Serial.print("Server ansverd ");
      Serial.println(ans);
      client.stop();
      //разбор задания
      if (ans.indexOf(F("id=0;command=NO;"))>=0)
      {
        //заданий нет
      }else{
        String id="";
        String command="";
        int pos=ans.indexOf("id=");
        
        if (pos>=0)
        {
          ans=ans.substring(pos+3);
          pos=ans.indexOf(";");
          if (pos>0)
          {
            id=ans.substring(0,pos);
            pos=ans.indexOf("command=");
            if (pos>=0){
              ans=ans.substring(pos+8);
              pos=ans.indexOf(";");
              if (pos>0)
              {
                command=ans.substring(0,pos);
                execMission(id,command);
              }
            }
          }
        }
      }
      
      
      
    }else {
      // if you didn't get a connection to the server:
      Serial.println("connection to SQL failed");
    }  
    


}



void Send_to_base(String SensorName, float value)
{
   if (client.connect(server, 8003)) {
      Serial.println("connected");
      // Make a HTTP request:
      client.print("GET /index.php?Sensor=");
      client.print(SensorName);
      client.print("&Value=");
      client.print(value);
      client.println(" HTTP/1.1");   
      client.println("Host: 192.168.1.10");
      client.println("User-Agent: arduino-web-client");
      client.println("Connection: close");
      client.println();
      delay(200);
      while (client.available()) {
       char c = client.read();
       Serial.print(c);
      }
      client.stop();
    }else {
      // if you didn't get a connection to the server:
      Serial.println("connection to SQL failed");
    }  
  
  
  
}


void DHTask ()

{


  float t2=bmp.readTemperature();
  float p=bmp.readPressure()/133.3;

  
  //Считываем влажность

  float h = dht.readHumidity();

  // Считываем температуру

  float t = dht.readTemperature();

  // Проверка удачно прошло ли считывание.

  if (isnan(h) || isnan(t)) {

  Serial.println("Не удается считать показания");  
  
  }
  else
  {
   // Blynk.virtualWrite(TEMP_PIN, t);
   // Blynk.virtualWrite(TEMP_PIN2, t2);
   // Blynk.virtualWrite(HUM_PIN, h);
   // Blynk.virtualWrite(PRES_PIN, p);
    //выводим все параметры в сериал
    Serial.print("Влажность: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Температура: ");
    Serial.print(t);
    Serial.println(" *C ");
    Serial.print("Давление: ");
    Serial.print(p);
    Serial.print(" mm.Hg.");
    Serial.print("Температура 2: ");
    Serial.print(t2);
    Serial.println(" *C ");
    
    //--------------------------------------------------

    Send_to_base("t1",t);
    Send_to_base("h1",h);
    Send_to_base("t2",t2);
    Send_to_base("p",p);
    
     
    //народный мониторинг
    
     if (client.connect("narodmon.ru", 8283)) {
      delay(200);
       
      client.println("#c8-60-00-d2-32-fe");
      client.print("#H1#");
      client.println(h);
      client.print("#T1#");
      client.println(t);
      client.print("#T2#");
      client.println(t2);
      client.print("#P#");
      client.println(p);
      client.println("##");        
      delay(200);
      Serial.print("narodmon.ru: ");
      while (client.available()) {
       char c = client.read();
       Serial.print(c);
      }

      client.stop();
      
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection to narodmon.ru failed");
    }
    





    //--------------------------------------------------
    
  }
}

/*

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("V1 Slider value is: ");
  Serial.println(pinValue);
  if (pinValue!=0)
  {
    mySerial.write("START_COND");
    
  }
  else
  {
    mySerial.write("COND OFF");
    
  }
  
}*/

