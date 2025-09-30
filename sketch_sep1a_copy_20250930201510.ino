
//Copyright (C) 2010-2014  Henk Jegers
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.

//for questions email to info@archeryclock.com  

// version MEGA01   Initial version
// version MEGA01_1 Adding robustness for receiving ABCD detailes which is implemented in AC 232. Using the ABCD details information is not yet implemented in MEGA01_1 Needs archeryclock version 2320 or newer.
// version MEGA01_2 Added end number information. Needs Archeryclock2401 or newer. (older archeryclock versions don't sent the end number information to arduine)
// version MEGA02_1 Changes because of a change in protocoll for AC 241 (nr of archers and E and F shooters) and added functionality: emergency stop, add archers E and F.  Changes for signal robustness. (0 (0000) is sent as 12 (1100) to prevent decoding issues when 3 times 0 is sent (000000000000) 

#include <SPI.h>
#include <Ethernet.h>
// ---------------- Ethernet Config ----------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);    // <-- CAMBIA esta IP a la de tu red
unsigned int localPort = 2323;     // Puerto UDP donde escucha el Arduino
EthernetServer server(localPort);
EthernetClient client;

String inputString;
int serialvalue;
int statevalue; //received data for state details.
int trafficvalue; //received data for traffic lights, and ABCD lights
int leftcountvalue; //received data for left countdown.
int rightcountvalue; //received data for rightcountdown.
int abcdvalue;   //received data for abcd details
int endnrvalue;  //received data for end nr
int buttonvalue;
int remember1;
int loop1;
int blinkk;
int blinkkk;
int blinkl;
int blinkr;
int rightdigit; //value of left digit   (0..10  (10 means dashes))
int middigit; //value of middle digit (0..10  (10 means dashes))
int leftdigit; //value of light digit  (0..10  (10 means dashes))

int lefend; //value of left digit to indicate end nr  (0..9 or 15  (15 means P to indicate practise end))
int midend; //value of middle digit to indicate end nr  (0..9)
int rigend; //value of middle digit to indicate end nr  (0..9)
int comend; //value of combined left and middle digit. In case of practise end the P from the left digit should be projected. else the middle digit (0..9 or 15  (15 means P to indicate practise end)) 

int segment[16]={0x03f,0x006,0x05b,0x04f,0x066,0x06d,0x07d,0x007,0x07f,0x06f,0x040,0x000,0x03f,0x000,0x000,0x073}; //0,1,2,3,4,5,6,7,8,9,-, , , , ,P

void setup() {
  
  DDRC = B00000000;               // Pins A0 to A5 are inputs
  buttonvalue=0;
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(42, OUTPUT);
  pinMode(43, OUTPUT);
  pinMode(44, OUTPUT);
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
  pinMode(47, OUTPUT);
  pinMode(48, OUTPUT);
  pinMode(49, OUTPUT);
  pinMode(50, OUTPUT);
  pinMode(51, OUTPUT);
  pinMode(52, OUTPUT);
  pinMode(53, OUTPUT);
  pinMode(A8, OUTPUT);
  pinMode(A9, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);
  pinMode(A13, OUTPUT);
  pinMode(A14, OUTPUT);
  pinMode(A15, OUTPUT);

  remember1=0;
  loop1=0;
  blinkk=0;
  blinkkk=0;
  blinkl=0;
  blinkr=0;

  rightcountvalue=0xAAA0;
  leftcountvalue=0xAAA0;
  serialvalue=0x0000;
  
  Serial.begin(9600);
  digitalWrite(53, HIGH);
  delay(100);
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print("Mi IP: ");
  Serial.println(Ethernet.localIP());
  

}

void loop() {
  inputString = ""; // Limpiar buffer cada ciclo

  // ---------- Leer datos desde TCP ----------
  client = server.available();
  if (client) {
    while (client.connected() && client.available()) {
      char c = client.read();
      if (c != '\n') {
        inputString += c;
      }
      //delay(2);
    }
  }

  // Convertir a valor numérico
  if (inputString.length() > 0) {
    serialvalue = inputString.toInt();

    // ---------- Decodificación ----------
    if ((serialvalue & 0x01) && not(serialvalue & 0x06)) trafficvalue = serialvalue;
    if ((serialvalue & 0x03) && not(serialvalue & 0x0C)) abcdvalue = serialvalue;
    if ((serialvalue & 0x01) && (serialvalue & 0x02) && (serialvalue & 0x04) && not(serialvalue & 0x08)) endnrvalue = serialvalue;
    if ((serialvalue & 0x01) && (serialvalue & 0x02) && (serialvalue & 0x04) && (serialvalue & 0x08)) statevalue = serialvalue;
    if ((not(serialvalue & 0x01)) && ((serialvalue & 0x02) or (serialvalue & 0x04))) rightcountvalue = serialvalue;
    if ((not(serialvalue & 0x01)) && ((not(serialvalue & 0x02)) or (serialvalue & 0x04))) leftcountvalue = serialvalue;
  }

  // ---------- Logica de semáforos, ABCD, flechas, displays ----------
  // Blink timers
  if (blinkk == 0) blinkk = 300; else blinkk--;
  if (blinkkk == 0) blinkkk = 2400; else blinkkk--;

  if (blinkkk > 1200) {
    blinkr = 0;
    blinkl = ((blinkk > 200) ? HIGH : LOW);
  } else {
    blinkr = ((blinkk > 200) ? HIGH : LOW);
    blinkl = 0;
  }

  // Not A-F según estado
  if ((((statevalue >> 4) & B00000111) != 6) ? HIGH : LOW) {
    digitalWrite(2, (((((statevalue >> 13) & B00000111) < 1) ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 2 ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 4 ? LOW : HIGH)
                     and (not(trafficvalue & 0x0400) ? HIGH : LOW))); // notA
    digitalWrite(3, (((((statevalue >> 13) & B00000111) < 1) ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 2 ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 4 ? LOW : HIGH)
                     and (not(trafficvalue & 0x0800) ? HIGH : LOW))); // notB
    digitalWrite(4, (((((statevalue >> 13) & B00000111) < 2) ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 2 ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 4 ? LOW : HIGH)
                     and (not(trafficvalue & 0x1000) ? HIGH : LOW))); // notC
    digitalWrite(5, (((((statevalue >> 13) & B00000111) < 3) ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 2 ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 4 ? LOW : HIGH)
                     and (not(trafficvalue & 0x2000) ? HIGH : LOW))); // notD
    digitalWrite(8, (((((statevalue >> 13) & B00000111) < 4) ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 2 ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 4 ? LOW : HIGH)
                     and (not(trafficvalue & 0x4000) ? HIGH : LOW))); // notE
    digitalWrite(9, (((((statevalue >> 13) & B00000111) < 5) ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 2 ? LOW : HIGH)
                     and (((statevalue >> 7) & B00000111) == 4 ? LOW : HIGH)
                     and (not(trafficvalue & 0x8000) ? HIGH : LOW))); // notF
  } else {
    digitalWrite(2, blinkl); digitalWrite(3, blinkl);
    digitalWrite(4, blinkr); digitalWrite(5, blinkr);
    digitalWrite(8, blinkl); digitalWrite(9, blinkl);
  }
  digitalWrite(17,  (trafficvalue&0x0400)?HIGH:LOW);        //A
  digitalWrite(15,  (trafficvalue&0x1000)?HIGH:LOW);        //C

  digitalWrite(22,  (trafficvalue&0x0008)?HIGH:LOW);        //buzzer
  //if (digitalRead(A4)){ //buzzer yes/no switch
  //  digitalWrite(18,  (trafficvalue&0x0008)?HIGH:LOW);        //buzzer
  //}else{
  //  digitalWrite(18, LOW);                                    //if switch on A4 is off, buzzer must be always off
  //};
  
  // ---------- Selección derecha/izquierda ----------
  if (digitalRead(A5)) { // Right side
    digitalWrite(19, (trafficvalue & 0x0010) ? HIGH : LOW); // green
    digitalWrite(20, (trafficvalue & 0x0020) ? HIGH : LOW); // orange
    digitalWrite(21, (trafficvalue & 0x0040) ? HIGH : LOW); // red
    rightdigit = ((rightcountvalue >> 4) & B00001111);
    middigit  = ((rightcountvalue >> 8) & B00001111);
    leftdigit = ((rightcountvalue >> 12) & B00001111);
    digitalWrite(23, ((rightcountvalue & 0x0008) ? HIGH : LOW) and ((segment[leftdigit] == 0x040) ? LOW : HIGH));
  } else { // Left side
    digitalWrite(19, (trafficvalue & 0x0080) ? HIGH : LOW); // green
    digitalWrite(20, (trafficvalue & 0x0100) ? HIGH : LOW); // orange
    digitalWrite(21, (trafficvalue & 0x0200) ? HIGH : LOW); // red
    rightdigit = ((leftcountvalue >> 4) & B00001111);
    middigit  = ((leftcountvalue >> 8) & B00001111);
    leftdigit = ((leftcountvalue >> 12) & B00001111);
    digitalWrite(23, ((leftcountvalue & 0x0008) ? HIGH : LOW) and ((segment[leftdigit] == 0x040) ? LOW : HIGH));
  }

  // ---------- Segmentos dígitos ----------
  digitalWrite(37, ((segment[leftdigit]&0x001)?HIGH:LOW));
  digitalWrite(35, ((segment[leftdigit]&0x002)?HIGH:LOW));
  digitalWrite(33, ((segment[leftdigit]&0x004)?HIGH:LOW));
  digitalWrite(31, ((segment[leftdigit]&0x008)?HIGH:LOW));
  digitalWrite(29, ((segment[leftdigit]&0x010)?HIGH:LOW));
  digitalWrite(27, ((segment[leftdigit]&0x020)?HIGH:LOW));
  digitalWrite(25, ((segment[leftdigit]&0x040)?HIGH:LOW));

  digitalWrite(39, ((segment[middigit]&0x001)?HIGH:LOW));
  digitalWrite(38, ((segment[middigit]&0x002)?HIGH:LOW));
  digitalWrite(49, ((segment[middigit]&0x004)?HIGH:LOW));
  digitalWrite(47, ((segment[middigit]&0x008)?HIGH:LOW));
  digitalWrite(45, ((segment[middigit]&0x010)?HIGH:LOW));
  digitalWrite(43, ((segment[middigit]&0x020)?HIGH:LOW));
  digitalWrite(41, ((segment[middigit]&0x040)?HIGH:LOW));

  digitalWrite(A8, ((segment[rightdigit]&0x001)?HIGH:LOW));
  digitalWrite(A9, ((segment[rightdigit]&0x002)?HIGH:LOW));
  digitalWrite(A10, ((segment[rightdigit]&0x004)?HIGH:LOW));
  digitalWrite(A11, ((segment[rightdigit]&0x008)?HIGH:LOW));
  digitalWrite(A12, ((segment[rightdigit]&0x010)?HIGH:LOW));
  digitalWrite(A13, ((segment[rightdigit]&0x020)?HIGH:LOW));
  digitalWrite(A14, ((segment[rightdigit]&0x040)?HIGH:LOW));

  // ---------- Segmentos endnr ----------
  lefend = ((endnrvalue>>4)&B00001111);
  midend = ((endnrvalue>>8)&B00001111);
  rigend = ((endnrvalue>>12)&B00001111);
  comend = (lefend==15)? lefend : midend;

  digitalWrite(24, ((segment[comend]&0x001)?HIGH:LOW));
  digitalWrite(26, ((segment[comend]&0x002)?HIGH:LOW));
  digitalWrite(28, ((segment[comend]&0x004)?HIGH:LOW));
  digitalWrite(30, ((segment[comend]&0x008)?HIGH:LOW));
  digitalWrite(32, ((segment[comend]&0x010)?HIGH:LOW));
  digitalWrite(34, ((segment[comend]&0x020)?HIGH:LOW));
  digitalWrite(36, ((segment[comend]&0x040)?HIGH:LOW));

  digitalWrite(40, ((segment[rigend]&0x001)?HIGH:LOW));
  digitalWrite(42, ((segment[rigend]&0x002)?HIGH:LOW));
  digitalWrite(44, ((segment[rigend]&0x004)?HIGH:LOW));
  digitalWrite(46, ((segment[rigend]&0x008)?HIGH:LOW));
  digitalWrite(48, ((segment[rigend]&0x010)?HIGH:LOW));
//  digitalWrite(50, ((segment[rigend]&0x020)?HIGH:LOW));
//  digitalWrite(52, ((segment[rigend]&0x040)?HIGH:LOW));

  // ---------- Botones ----------
  buttonvalue=0;
  if (digitalRead(A4)) buttonvalue=5;
  if (digitalRead(A3)) buttonvalue=4;
  if (digitalRead(A0)) buttonvalue=1;
  if (digitalRead(A1)) buttonvalue=2;
  if (digitalRead(A2)) buttonvalue=3;

  if (loop1==0) {
    if (buttonvalue!=remember1) {
      Serial.print(buttonvalue); delay(3);
      remember1=buttonvalue; loop1=15;
    }
  } else {
    delay(10);
    loop1--;
  }
}
  
