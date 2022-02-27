//Arduino Wemos D1 mini (ESP8266 Boards 2.7.4, "D1 R2 & mini") 
//Demo of using the same functions to access multiple SD card files
//and passing parameters.
#include <SD.h>                //SD card library
#include <SPI.h>               //Serial Peripheral Interface bus lib for COMM, SD com
#define S_prt Serial.print     //Short name for Serial.print
#define S_pln Serial.println   //Short name for Serial.println
File dataFile;                 //SD type File variable. Keep current file handle 
char *FName1 = "fname1.txt";   //SD card file name 1 to create/write/read
char *FName2 = "fname2.txt";   //SD card file name 2 to create/write/read
String SDData;                 //Build data to write to SD card
unsigned int x = 0;            //Variable for testing

//SD CARD FUNCTIONS =================================
void openSD(){                               //Routine to open SD card
  S_pln(); S_pln("Open SD card");            //User message.
  if (!SD.begin(15)) {                       //If not open, print message. (CS=pin15)
    S_pln("Open SD card failed");
    return;}
  S_pln("SD Card open");
}

void openFile(char *FName, char RW) {        //Open 1 SD file at a time. Shared func.
  dataFile.close();                          //Ensure file status, before re-opening
  dataFile = SD.open(FName, RW);}            //Open Read at start.  Open at EOF for write/append

void print2File(char *FN, String tmp1){      //Print data to SD file. Shared func.
  openFile(FN, FILE_WRITE);                  //Open user SD file for write
  if (dataFile) {                            //If file there & opened --> write
    dataFile.println(tmp1);                  //Print string to file
    dataFile.close();                        //Close file, flush buffer (reliable but slower)
//    dataFile.flush();                        //Flush file buffer
  } else {S_pln("Error opening file for write");}   //File didn't open
}

void getRecordFile(){                        //Read SD file (must open first)
  if (dataFile) {                            //If file is there
    Serial.write(dataFile.read());           //Read datafile, then write to COM.
  } else {S_pln("Error opening file for read");}    //File didn't open
}

//END SD CARD FUNCTIONS =============================
void setup() {                               //Setup function
  Serial.begin(115200);                      //Open Com
  delay(500);                                //Allow serial to open
  openSD();                                  //Open SD card
}

void loop(){                                 //Loop function
  for(int i=0;i<25;i++){                     //Loop so won't get infinite loop errors
    SDData = "Test file 1, Cycle " + String(x);  //Prepare test 1 output string
    print2File(FName1, SDData);              //Write string to SD file 1
    S_pln(SDData);                           //Write status to COM
    SDData = "This is a test of file2";      //Prepare test 2 output string
    print2File(FName2, SDData);              //Write string to SD file 2
    S_pln(SDData);                           //Write status to COM
    delay(100);                              //Delay for visual on COM
  }
  x++;                                       //To show test 1 data is changing
  openFile(FName1, FILE_READ);               //Open file 1 & print to show it's working
  while (dataFile.available()) {             //Read SD file until EOF
    getRecordFile();}                        //Get 1 line from SD file
  delay(3000);                               //Wait x milliseconds
  dataFile.close();                          //Close file, flush buffer.
}
