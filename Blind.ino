#define MY_DEBUG 
#define MY_TRANSPORT_WAIT_READY_MS 100
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX
//#define MY_NODE_ID 111
//#define MY_PARENT_NODE_ID 1
//#define MY_PASSIVE_NODE
#include <SPI.h>
#include <MySensors.h>
#include <Servo.h> 
#include <IRremote.h>
#include <avr/wdt.h>

#define IDP 1
#define IDTime 4
#define LED 2

#define K1 7
#define K2 8 

#define MaxSmallTry 5 //Количество попыток отправки пакета.

#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

MyMessage msg1(IDP,     V_STATUS);
MyMessage msgTime(IDTime, V_VAR4);

int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;
Servo myservo1;
Servo myservo2;


unsigned long previousMillis = 0; // время, когда состояние обновлялось
long interval = 60000; // период (в миллисекундах)

void presentation()  
{   
sendSketchInfo("jalousie sensor", "V1.0_03072017"); 

present(IDP,     S_LIGHT,  "jalousie status");
present(IDTime,  S_CUSTOM, "Time sleep");
}

int t = 500;
bool PL = 1, S = 1;


void setup() {
  wdt_enable(WDTO_8S);
  pinMode(K1, OUTPUT);  //Питание сервы 1 
  pinMode(K2, OUTPUT);  //Питание сервы 2
  myservo1.attach(5);
  myservo2.attach(6);
  digitalWrite(RECV_PIN, HIGH);
  pinMode(LED, OUTPUT);
  pinMode(3, INPUT);
  int L = loadState(3);
  if((L) > 0 && (L)< 255)
  interval=L*60000;
  irrecv.enableIRIn(); // Start the receiver
  send(msgTime.set((interval)/60000)); 
}

void loop() 
{
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    sendHeartbeat(); 
  }
     
  if (irrecv.decode(&results)) {
    if (results.value == (0x91EE7A85)) { //Код кнопки
      Serial.println("IR - ok");
      PL = 1;
      S = 1;
    }
  
    if (results.value == (0x91EE906F)){ //Код кнопки
      Serial.println("IR - ok");
      PL = 0;
      S = 1;
    }		
    
    irrecv.resume(); // Получаем следующее значение
  }

  if(S == 1){  
    if(PL == 0){
      digitalWrite(LED, HIGH);
      Serial.println("Servo - ok");
      myservo1.write(180);
      delay(100);
      digitalWrite(K1, HIGH);
      delay(t);    
      digitalWrite(K1, LOW);

      myservo2.write(0);
      delay(100);
      digitalWrite(K2, HIGH);
      delay(t);    
      digitalWrite(K2, LOW);
      delay(100);
      TX(1, 2, PL);
      S = 0;
      digitalWrite(LED, LOW);
    }  
  }

  if(S == 1){  
    if(PL == 1){
      digitalWrite(LED, HIGH);
      Serial.println("Servo - ok");
      myservo1.write(0);
      delay(100);
      digitalWrite(K1, HIGH); 
      delay(t);    
      digitalWrite(K1, LOW);

      myservo2.write(180);
      delay(100);
      digitalWrite(K2, HIGH);
      delay(t);    
      digitalWrite(K2, LOW);
      delay(100);
      TX(1, 2, PL);
      S = 0;
      digitalWrite(LED, LOW);
    }  
  }
} //END


void receive(const MyMessage &message) {
  
  if (message.sender == 0){  
    
    if (message.sensor == 1){  
      if (message.type==V_LIGHT) {
        PL = (message.getBool()?RELAY_ON:RELAY_OFF);
        S = 1;
      }
    }   

    if (message.sensor == 4){  
      if (message.type==V_VAR4) { 
        if((message.getInt())<255){
          saveState(3, message.getInt());
          interval = (loadState(3)*60000);
        }
      }
    }  
  }
}  

//=========================================================================================
///////////////////////////////////////////////////////////////////////////////////////////
//=========================================================================================

void TX(byte ID_sensors, byte Type_variable, bool variable)
{  
wdt_reset(); 
byte count;  
MyMessage msg(ID_sensors, Type_variable);
 int send_data = send(msg.set(variable), true);
   wait(1000, ID_sensors, Type_variable);
   if (send_data == 0)
     {
       Serial.print("Delivery failed: ");
       Serial.println(send_data);
       while (send_data == 0 && count < MaxSmallTry)
         {         
            wdt_reset(); 
            count++;  
            Serial.print("Sending a message try No.");      
            Serial.println(count);
            send_data = send(msg.set(variable), true);
            wait(1000, ID_sensors, Type_variable);   
         }
     }       
   else
     {
       Serial.print("Delivery: OK - ");
       Serial.println(send_data);   
     }
       count = 0;
}





