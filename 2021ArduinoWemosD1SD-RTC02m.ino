//Arduino, WeMos D1R1 (ESP8266 Boards 2.5.2), upload 921600, 80MHz, COM:7,WeMos D1R1 Boards 2
//Program: Creates SD card file, opens, writes, prints out.  Tested with 64GB SD.
//WeMos Micro SD Shield uses HSPI(12-15) not (5-8), 3V3, G:
//D12 (6) = MISO      master in, slave out (primary in, secondary out)
//D13 (7) = MOSI      master out, slave in (primary out, secondary in)
//D14 (5) = CLK       Clock
//D15 (8) = CS        Chip select
//SD library --> 8.3 filenames (not case-sensitive, ABCDEFGH.txt = abcdefgh.txt).
//RTC DS1307. I2C--> SCL(clk)=D1, SDA(data)=D2 
//20210323 - by TSBrownie.  Non-commercial use approved.
#include <SD.h>               //SD card library
#include <SPI.h>              //Serial Peripheral Interface bus lib for COMM, SD com
#include "Wire.h"             //I2C library
#define DS1307 0x68           //I2C Addr of 1307 (Default=0x68 Hex)
File diskFile;                //Link to file name (file handle)
String SDData;                //Build data to write to SD "disk"
String timeString;            //Build date time data
bool OneTimeFlag = true;      //For demo, execute 1 time (remove for logging)
String DoWList[]={"Null",",Sun,",",Mon,",",Tue,",",Wed,",",Thr,",",Fri,",",Sat,"}; //DOW from 1-7
byte second, minute, hour, DoW, Date, month, year;     //Btye variables for BCD time
String FName = "SDfil01.txt"; //SD card file name to create/write/read
unsigned int interval = 1000; //Time in milliseconds between pin readings 

//RTC FUNCTIONS =====================================
byte BCD2DEC(byte val){             //Ex: 51 = 01010001 BCD.  01010001/16-->0101=5 then x10-->50.  
  return(((val/16)*10)+(val%16));}  //         01010001%16-->0001.  50+0001 = 51 DEC

void GetRTCTime(){                               //Routine read real time clock, format data
  byte second;byte minute;byte hour;byte DoW;byte Date;byte month;byte year;
  Wire.beginTransmission(DS1307);                //Open I2C to RTC DS1307
  Wire.write(0x00);                              //Write reg pointer to 0x00 Hex
  Wire.endTransmission();                        //End xmit to I2C.  Send requested data.
  Wire.requestFrom(DS1307, 7);                   //Get 7 bytes from RTC buffer
  second = BCD2DEC(Wire.read() & 0x7f);          //Seconds.  Remove hi order bit
  minute = BCD2DEC(Wire.read());                 //Minutes
  hour = BCD2DEC(Wire.read() & 0x3f);            //Hour.  Remove 2 hi order bits
  DoW = BCD2DEC(Wire.read());                    //Day of week
  Date = BCD2DEC(Wire.read());                   //Date
  month = BCD2DEC(Wire.read());                  //Month
  year = BCD2DEC(Wire.read());                   //Year
  timeString = 2000+year;                        //Build Date-Time data to write to SD
  if (month<10){timeString = timeString + '0';}  //Pad leading 0 if needed
  timeString = timeString + month;               //Month (1-12)  
  if(Date<10){timeString = timeString + '0';}    //Pad leading 0 if needed
  timeString = timeString + Date;                //Date (1-30)
  timeString = timeString + DoWList[DoW];        //1Sun-7Sat (0=null)
  if (hour<10){timeString = timeString + '0';}   //Pad leading 0 if needed
  timeString = timeString + hour;                //HH (0-24)
  if (minute<10){timeString = timeString + '0';} //Pad leading 0 if needed
  timeString = timeString + minute;              //MM (0-60)
  if (second<10){timeString = timeString + '0';} //Pad leading 0 if needed
  timeString = timeString + second;              //SS (0-60)
}

//SD CARD FUNCTIONS =================================
void openSD() {                                  //Routine to open SD card
  Serial.println(); Serial.println("Open SD card");    //User message
  if (!SD.begin(15)) {                           //If not open, print message.  (CS=pin15)
    Serial.println("Open SD card failed");
    return;}
  Serial.println("SD Card open");
}

char openFile(char RW) {                     //Routine open SD file.  Only 1 open at a time.
  diskFile.close();                          //Ensure file status, before re-opening
  diskFile = SD.open(FName, RW);}            //Open Read at end.  Open at EOF for write/append

String print2File(String tmp1) {             //Routine to print data to SD file
  openFile(FILE_WRITE);                      //Open user SD file for write
  if (diskFile) {                            //If file there & opened --> write
    diskFile.println(tmp1);                  //Print string to file
    diskFile.close();                        //Close file, flush buffer (reliable but slower)
  } else {Serial.println("Error opening file for write");}   //File didn't open
}

void getRecordFile() {                       //Routine to read from SD file
  if (diskFile) {
    Serial.write(diskFile.read());           //Read, then write to COM
  } else {Serial.println("Error opening file for read");}    //File didn't open
}

// SETUP AND LOOP =============================
void setup() {
  Wire.begin();                              //Join I2C bus as primary
  Serial.begin(74880);                       //Open serial com (74880, 38400, 115200)
  pinMode(A0,INPUT);                         //Set unused pins A0, D0, D3, D4 to input
  pinMode(0,INPUT);                          //Default is HIGH
  for (int j=3; j<=4;j++){pinMode(j, INPUT);}  
  openSD();                                  //Call open SD card routine
}

void loop() {                                //Get data, write timestamped records
  if (OneTimeFlag) {                         //Execute once for demo (remove for logging)
    for (uint32_t i = 0; i <= 5L; i++) {     //Max 4294967295? (remove for unlimited logging)
      Serial.println("Taking Reading");
      GetRTCTime();                          //Get time from real time clock
      SDData = timeString+','+i+','+analogRead(A0)+','+digitalRead(D0);
      for(int k=3; k<=4;k++){                //Status of A0, D0, D3, D4 pins
        SDData = SDData+','+digitalRead(k);  //Add unused pin status to output
      }  
      print2File(SDData);                    //Write data to SD file
      delay(interval);                       //Take data reading every x microseconds
    }
    OneTimeFlag = false;                     //End 1 time write to SD card
    Serial.println("File Write Done");
    openFile(FILE_READ);                     //Open SD file at start for read
    while (diskFile.available()) {           //Read SD file until EOF
      getRecordFile();                       //Get 1 record (line) from SD file
    }
    diskFile.close();                        //Close file, flush buffer
    Serial.println("Printout File Done.  Delete File.");
    SD.remove(FName);                        //Delete file.  Comment out to keep file.   
  }
}
