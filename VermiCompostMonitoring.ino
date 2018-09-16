#include <OneWire.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <dht.h>
#define ONE_WIRE_BUS 13
#define dht_dpin 10

dht DHT;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //To lcd (RS,E,D4,D5,D6,D7)
byte degree[8] = 
              {
                0b00011,
                0b00011,
                0b00000,
                0b00000,
                0b00000,
                0b00000,
                0b00000,
                0b00000
              };
    
int up = 8;               //Up button
int down = 9;           //Down button
double sensorValue = 0;
int page_counter=1 ;       //To move beetwen pages
unsigned long previousMillis = 0;
unsigned long interval = 10000; //Desired wait time 10 seconds
//Storage
boolean current_up = LOW;          
boolean last_up = LOW;            
boolean last_down = LOW;
boolean current_down = LOW;
long duration;
int distance;
int solenoid1 = 50;
int solenoid2 = 52;
float tempC = 0,tempF = 0;
int fan = 24;

void setup() {
  pinMode(solenoid1, OUTPUT);
  pinMode(solenoid2, OUTPUT);
  pinMode(fan,OUTPUT);
  //pinMode(3, OUTPUT);
  pinMode(6, OUTPUT); //trigPin
  pinMode(7, INPUT); //echoPin
  lcd.begin(16,2); 
  sensors.begin();
  analogWrite(3, 0);
  Serial.begin(9600);
 
}
   //---- De-bouncing function for all buttons----//
boolean debounce(boolean last, int pin)
{
boolean current = digitalRead(pin);
if (last != current)
{
  delay(5);
  current = digitalRead(pin);
}
  return current;
}

void loop() {
  DHT.read11(dht_dpin);
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);
  tempF = sensors.toFahrenheit(tempC);
  Serial.print(analogRead(A0));
  Serial.print(",");
  Serial.print(analogRead(A1));
  Serial.print(",");
  Serial.print(analogRead(A2));
  Serial.print(",");
  Serial.print(analogRead(A3));
  Serial.print(",");
  Serial.print(DHT.humidity);
  Serial.print(",");
  Serial.print(DHT.temperature);
  Serial.print("
  Serial.print(",");
  Serial.print(tempC);
  Serial.print(",");
  Serial.print(distance);
  Serial.print("|");
  if (analogRead(A0)>=400 && analogRead(A0)<=1000)
{
  digitalWrite(solenoid1, LOW);
  }
  else {
  digitalWrite(solenoid1,HIGH);
  }   
if (analogRead(A1)>=400 && analogRead(A1)<=1000)
{
  digitalWrite(solenoid1, LOW);
  }
  else {
  digitalWrite(solenoid1, HIGH);
  } 
if (analogRead(A2)>=400 && analogRead(A2)<=1000)
{
  digitalWrite(solenoid2, LOW);
  }
  else {
  digitalWrite(solenoid2,HIGH);
  }
if (analogRead(A3)>=400 && analogRead(A3)<=1000)
{
  digitalWrite(solenoid2, LOW);
  }
  else {
    digitalWrite(solenoid2, HIGH);
  } 
if(tempC>35)
 {
  digitalWrite(fan,HIGH);
  }
 else{
  digitalWrite(fan,LOW);
  }
 
current_up = debounce(last_up, up);         //Debounce for Up button
current_down = debounce(last_down, down);   //Debounce for Down button

//----Page counter function to move pages----//
//Page Up
    if (last_up== LOW && current_up == HIGH){  //When up button is pressed
      lcd.clear();                     //When page is changed, lcd clear to print new page  
      if(page_counter <9){              //Page counter never higher than 3(total of pages)
      page_counter= page_counter +1;   //Page up
      
      }
      else{
      page_counter= 9;  
      }
  }
  
    last_up = current_up;

//Page Down
    if (last_down== LOW && current_down == HIGH){ //When down button is pressed
      lcd.clear();                     //When page is changed, lcd clear to print new page    
      if(page_counter >1){              //Page counter never lower than 1 (total of pages)
      page_counter= page_counter -1;   //Page down
      
      }
      else{
      page_counter= 1;  
      }
  }
    
  
 {digitalWrite (6,LOW);
  delayMicroseconds (2);
  digitalWrite (6, HIGH);
  delayMicroseconds (10);
  digitalWrite (6, LOW);
  duration = pulseIn (7, HIGH);
  distance = duration/58   ;
  }
  {
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);
  tempF = sensors.toFahrenheit(tempC);
  delay(1000);}
  {DHT.read11(dht_dpin);}
    last_down = current_down;
//------- Switch function to write and show what you want---// 
  switch (page_counter) {
    case 1:{     //TITLE 
      lcd.setCursor(2,0);
      lcd.print("Vermicompost");
      lcd.setCursor(2,1);
      lcd.print(" Production");
    }
    break;
    case 2:{ //SOIL MOISTURE A
      lcd.setCursor(0,0);
       lcd.print("Soil Moisture A :");// print the results to the LCD Display:
      sensorValue = analogRead(A0);// read the analog in value:
      lcd.setCursor(0, 1);
      //lcd.print(sensorValue);
       lcd.print(100-((sensorValue-360)/(680-360))*100);
    }
    break;
    case 3: {//SOIL MOISTURE B
       lcd.setCursor(0,0);
       lcd.print("Soil Moisture B :");// print the results to the LCD Display:
      sensorValue = analogRead(A1);// read the analog in value:
      lcd.setCursor(0, 1);
      lcd.print(100-((sensorValue-360)/(680-360))*100);
    }
    break;
    
    case 4: { //SOIL MOISTURE C
       lcd.setCursor(0,0);
       lcd.print("Soil Moisture C :");// print the results to the LCD Display:
      sensorValue = analogRead(A2);// read the analog in value:
      lcd.setCursor(0, 1);
      lcd.print(100-((sensorValue-360)/(680-360))*100);
    }
        break;
      case 5: { //SOIL MOISTURE D
       lcd.setCursor(0,0);
      lcd.print("Soil Moisture D :");// print the results to the LCD Display:
      sensorValue = analogRead(A3);// read the analog in value:
      lcd.setCursor(0, 1);
    lcd.print(100-((sensorValue-360)/(680-360))*100);
    }
    break;
     case 6: {  //SOIL TEMPERATURE
   lcd.setCursor(0,0);
  lcd.print("SOIL TEMPERATURE ");
   lcd.setCursor(0,1);
  lcd.print(tempC);
  lcd.print(" DEEGRES C");
    }
    break;
    case 7: {  //HUMDITY AND AMBIENT TEMPERATURE
    lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(DHT.humidity);   // printing Humidity on LCD
  lcd.print(" %");
  lcd.setCursor(0,1);
  lcd.print("Temperature:");
  lcd.print(DHT.temperature);   // Printing temperature on LCD
  lcd.write(1);
  lcd.print("C");
    }
    break;
    case 8: {  //WATER LEVEL
    lcd.setCursor(0,0); // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("Water Level: "); // Prints string "Distance" on the LCD
  lcd.print(distance); // Prints the distance value from the sensor
  lcd.print("  cm");     
    }
    break;
    
  }//switch end
  
}//loop end
/*
void setup()
{
  
  Serial.begin(9600);
}
void loop()
{
 
}
 */
