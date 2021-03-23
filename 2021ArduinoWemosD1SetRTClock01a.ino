//Arduino C.  Wemos D1R1 ESP8266Boards (2.5.2) Mini DS32 w/ Mini Shield.  
//Set Real Time Clock Module DS1307.  COM7.  VCC->5v, Gnd->Gnd, SDA (data)->A4, SCL (clk)->A5
//RTC Pinouts, I2C.  D1 = SCL, D2 = SDA
//20210323 - by TSBrownie.  Non-commercial use approved.
#include "Wire.h"                   //Include I2C library
#define DS1307 0x68                 //Default I2C Addr of 1307 0x68 Hex

String DoWList[] = {"Null"," Sun "," Mon "," Tue "," Wed "," Thr "," Fri "," Sat "}; //DOW from 1-7
byte second, minute, hour, DoW, Date, month, year;     //Btye variables for BCD time
int sec, mm, hh, dow, date, mnth, yy;                  //Decimal variables

byte DEC2BCD(byte val){             //Ex: 51 --> 51/10 --> 5 (0101). x16 --> 01010000.  
  return(((val/10)*16)+(val%10));}  //    51%10 --> 0001  5+1 --> 01010001

void PutRTCTime(byte second, byte minute, byte hour,   //Write date/time to RTC
                byte DoW, byte Date, byte month, byte year){
  Wire.beginTransmission(DS1307);             //Open I2C comm
  Wire.write(0);                    //Set to first register
  Wire.write(DEC2BCD(second));      //Seconds (in first register)
  Wire.write(DEC2BCD(minute));      //Minutes
  Wire.write(DEC2BCD(hour));        //Hours
  Wire.write(DEC2BCD(DoW));         //DoW (1=Sun, 7=Sat, 0=NAxx)
  Wire.write(DEC2BCD(Date));        //Day of month date (1-31)
  Wire.write(DEC2BCD(month));       //Month (1-12)
  Wire.write(DEC2BCD(year));        //Year (0-99)
  Wire.endTransmission();           //End I2C comm.  Send above data.
}

byte BCD2DEC(byte val){             //Ex: 51 = 01010001 BCD.  01010001/16-->0101=5 then x10-->50.  
  return(((val/16)*10)+(val%16));}  //         01010001%16-->0001.  50+0001 = 51 DEC

void GetRTCTime(byte *second, byte *minute,byte *hour,byte *DoW,
                byte *Date,byte *month,byte *year){
  Wire.beginTransmission(DS1307);  //Open I2C to RTC
  Wire.write(0x00);                         //Write reg pointer to 0x00 Hex
  Wire.endTransmission();                   //End xmit to I2C.  Send requested data.
  Wire.requestFrom(DS1307, 7);     //Get 7 bytes from RTC buffer
  *second = BCD2DEC(Wire.read() & 0x7f);    //Set pntr to seconds remove hi order bit
  *minute = BCD2DEC(Wire.read());           //Set pointer to minutes
  *hour = BCD2DEC(Wire.read() & 0x3f);      //Set pntr to hour remove 2 hi order bits
  *DoW = BCD2DEC(Wire.read());              //Set pointer to day of week
  *Date = BCD2DEC(Wire.read());             //Set pointer to Date
  *month = BCD2DEC(Wire.read());            //Set pointer to month
  *year = BCD2DEC(Wire.read());             //Set pointer to year
}
void printTime(){                         //Read & print data from RTC
  GetRTCTime(&second, &minute, &hour, &DoW, &Date, &month, &year);  //Get RTC data
  Serial.print(2000+year, DEC);             //Print year 20xx
  Serial.print("/");
  if (month<10){Serial.print("0");}         //Print leading 0 if needed
  Serial.print(month, DEC);                 //Month as decimal
  Serial.print("/");
  if(Date<10){Serial.print("0");}           //Print leading 0 if needed
  Serial.print(Date, DEC);                  //Date (1-30)
  Serial.print(DoWList[DoW]);               //1Sun-7Sat (0=null)
  if (hour<10){Serial.print("0");}          //Print leading 0 if needed
  Serial.print(hour, DEC);                  //HH
  Serial.print(":");        
  if (minute<10){Serial.print("0");}        //Print leading 0 if needed
  Serial.print(minute, DEC);                //MM
  Serial.print(":");
  if (second<10){Serial.print("0");}        //Print leading 0 if needed
  Serial.println(second, DEC);              //SS
}
void setup(){                               //Setup function
  Wire.begin();                             //Join I2C bus as primary
  Serial.begin(74880);                      //Initialize serial com
  Serial.println("Enter 'sec,mm,hh,dow(Sun=1),date(1-31),mnth(1-12),yy(00-99)'  No spaces. No leading 0.");
  while(!Serial.available() ){}             //Wait for Serial Monitor input
  if(Serial.available()){                   //If there's input parse it to get date/time
      sec = Serial.parseInt();              //1st seconds
      mm =  Serial.parseInt();              //2nd minutes
      hh = Serial.parseInt();               //3rd hours
      dow = Serial.parseInt();              //4th Day of Week code (1Sun-7Sat)
      date = Serial.parseInt();             //5th Day of the Month (1-31)
      mnth = Serial.parseInt();             //6th Month code (1Jan-12Dec)
      yy = Serial.parseInt();               //7th Year (2 digits, 00-99)
      PutRTCTime(sec,mm,hh,dow,date,mnth,yy);  //Call function to set RTC 
    }   
  for(int i=1; i<5; i++){               //Read RTC and print date/time for user verify
     printTime();                     //Output date/time to Serial Monitor
     delay(1000);                       //Delay output x seconds
  }
  Serial.println("If the above date and times are correct, the RTC is updated.");
  Serial.println("The RTC can be safely removed.  Reset Arduino to run again.");
}

void loop(){}                           //Just loops
