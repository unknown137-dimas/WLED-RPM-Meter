

/*
 12-12-2016
 Final Final Final Tested again on 12-12-2016 Miata Cold With ELM327 Just Connected and Connected all the time. Works Both ways.
 
 
 Added 4000 delay after 0100
 * 
Added if statement to check if over 100 to flash display but it doesn't work correctly. Coomented out
Final Final Code. Works on Miata.
Does a check first to see if it is connected to ELM327 by looking for the ATZ..... received signature
after issuing an ATZ command. Once bluetooth is connected than we get back     ATZ[[[ELM327 v1.5
To parse the ATZ needed to look for only TZ by using substring(1,3)=="TZ". Both Serial Monitor and ELM327 work without a space in the beginning.



1.f

if (BuildINString.substring(1,3)=="TZ")  ENTER WITH A SPACE 1ST CHAR DOESN'T MATTER

Add a check if ELM327 returned
(1,4)=="ATZ")   Serial Monitor  put a space in front  _ATZ?????  Test Works OK but not in car
// MIATA RESPONSE TO ATZ IS "ATZ[[[ELM327 V1.5"  OR AT LEAST THAT IS WHAT ARDUINO HAS IN THE BUFFER  

12-8-2016
Result =  A++  Works Great  got back correct reading 
Miata Final 1.c
ATZ ATZ 0100 105, increased message to 3 seconds

12-7-2016
Back to original code still wrong display bach -35 C
Commented last code not to blink
Original Code Got back correct value with Blinking

Updated 12-7-2016
tests OK in Serial Monitor
Test OK in Miata
WorkingString = BuildINString.substring(11,14);   //0105[41 05 32   // Correct value reading back  10 degrees.
32(h) is 50(d) -40 is 10

WITH THIS SET UP
wait 5 seconds between ignition otherwise received 0105[Lines] STOPPED[Lines][Lines]>
After 5 seconds ON Received 0105[Lines]41 05 32 [Lines][Lines]
*/

// include the library code:
#include <LiquidCrystal_I2C.h>  //NEW!!!!!!!!!
#include <Wire.h>  //NEW!!!!!!!!!!!!
// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // NEW!!!!!!!!!!!!!  Set the LCD I2C address



byte inData;
char inChar;
String BuildINString="";
String DisplayString="";
String WorkingString="";
long DisplayValue;
long A;



void setup() {
  // put your setup code here, to run once:

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
 
  //Display Welcome Message
  lcd.setCursor(0, 0);
  lcd.print("2004 Mazda Miata");
  lcd.setCursor(0, 1);
  lcd.print("Welcome, Sofia ");
  delay(3000);
  lcd.clear();

//********************************************************************
//Added new code to check if ELM327 responded
  Retry:
  lcd.setCursor(0, 0);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print("Connecting......    ");



  
  //Set up Serial communication at 9600 baud
  Serial.begin(9600);   //initialize Serial  

  Serial.println("ATZ");
  lcd.setCursor(0, 0);
  lcd.print("ELM327 TZ    ");
  delay(2000);
   ReadData();
   

                                             // If used substring(1,4)=="ATZ" needed a space before ATZ in Serial Monitor and it did not work
  if (BuildINString.substring(1,3)=="TZ")    // MIATA RESPONSE TO ATZ IS ATZ[[[ELM327 V1.5  OR AT LEAST THAT IS WHAT ARDUINO HAS IN THE BUFFER
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome");
      lcd.setCursor(9, 0);
      //lcd.print(BuildINString);   //Echo response to screen "Welcome  ELM327"
      lcd.setCursor(0, 1);
      lcd.print("Connection OK         ");
      delay(1500);
      lcd.clear();
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print("Error             ");
      lcd.setCursor(0, 1);
      lcd.print("No Connection!         ");
      delay(1500);
      lcd.clear();
      goto Retry;
    }

//*****************************************************************  

 //Added this from the test Code

 

  Serial.println("0100");          // Works with This  only
  lcd.setCursor(0, 0);
  lcd.print("Initialzing.....");          //Initialize BUS  //lcd.print("0100 Sent"); 
  delay(4000);
   ReadData();
   lcd.setCursor(0, 0);            //Added 12-10-2016
   lcd.print("Initialized.....");  //Added 12-10-2016
   delay(1000);
   lcd.clear();

 

}

void loop() {

// Added to display Coolant Temp
  lcd.setCursor(0, 0);
  lcd.print("Coolant Temp    "); 


  //resets the received string to NULL  Without it it repeated last string.
  BuildINString = "";  

  Serial.println("0105");  // Send Coolant PID request 0105
  //Serial.flush();         //Not sure if it's needed*********************************************
  delay(1000);

  // Receive complete string from the serial buffer
  ReadData();  //replaced below code


// Parse the received string, retrieve only the hex value for temperature  Example: 32 is the 11 and 12th character
// 0105[41 05 32   Correct value reading back  10 degrees. (11,13 or even 11,14 works)
WorkingString = BuildINString.substring(11,13);   

    A = strtol(WorkingString.c_str(),NULL,16);  //convert hex to decimnal

   DisplayValue = A;
   DisplayString = String(DisplayValue - 40) + " C            ";  // Subtract 40 from decimal to get the right temperature
   lcd.setCursor(0, 1);
   lcd.print(DisplayString); 
   delay(500);

//Check if if over 100 C ******************************************************
int B;
B = DisplayString.toInt();  //Convert String to Integer  .toInt()


   if (B >= 100){     //Check if if over 100 C
    
// ------- Quick 3 blinks of backlight  -------------
  for(int i = 0; i< 3; i++)
  {
    lcd.display();   //For I2C use lcd.backlight
    delay(250);
    lcd.noDisplay(); //For I2C use lcd.noBacklight
    delay(250);
  }
  lcd.display(); // finish with backlight on   //For I2C use lcd.backlight
   }
  
}

//Read Data Function ***********************************************************
void ReadData()
{
BuildINString="";  
  while(Serial.available() > 0)
  {
    inData=0;
    inChar=0;
    inData = Serial.read();
    inChar=char(inData);
    BuildINString = BuildINString + inChar;
  }
}








