/**
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from more than one PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 *
 * Example sketch/program showing how to read data from more than one PICC (that is: a RFID Tag or Card) using a
 * MFRC522 based RFID Reader on the Arduino SPI interface.
 *
 * Warning: This may not work! Multiple devices at one SPI are difficult and cause many trouble!! Engineering skill
 *          and knowledge are required!
 *
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN             9          // Configurable, see typical pin layout above
#define SS_1_PIN            7         
#define SS_2_PIN            8          
#define SS_3_PIN            10
#define red_LED_PIN         A4
#define green_LED_PIN       A5   

#define NR_OF_READERS   3

#define NUM_ITEMS       3   //set number of tracked items


byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.


//create struct to hold information for individual tracked items
struct itemID {
  byte idArray[4];
  uint8_t itemLocation; // reader number
  String itemName;
};

//set tag UIDs here
//default value for location is set to 255 (indicates not in inventory)
itemID items[NUM_ITEMS] = 
{
  {{0x5A, 0xF0, 0x93, 0xBB}, 255, "Red Lego"},
  {{0x4A, 0xF0, 0x93, 0xBB}, 255, "Pill Bottle"},
  {{0x3A, 0xF0, 0x93, 0xBB}, 255, "Black Box"}
};

//set checking variables
bool tagRead = false;
uint8_t foundReader = 255;
uint8_t foundItem = 255;
bool itemFound = false;
char serialMessage;

/**
 * Initialize.
 */
void setup() {

  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin();        // Init SPI bus

  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {    
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card

    delay(20); // wait to establish connection

    mfrc522[reader].PCD_SetAntennaGain(mfrc522[reader].RxGain_max); // increase antenna gain
    
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }

  //LED setup
  pinMode(red_LED_PIN, OUTPUT);
  pinMode(green_LED_PIN, OUTPUT);
}

/**
 * Main loop.
 */
void loop() {
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
    
    tagRead = true; //track if a tag has been read to compare to foundItem status

    //match read ID with inventory id
    for (uint8_t itemNumber = 0; itemNumber < NUM_ITEMS; itemNumber++)
    {
      for (uint8_t i = 0; i < 4; i++)
      {
        if(items[itemNumber].idArray[i] != mfrc522[reader].uid.uidByte[i])
        {
          break;
        }
        else
        {
          if (i ==  mfrc522[reader].uid.size - 1)
          foundReader = reader;
          foundItem = itemNumber;
          itemFound = true;
        }
      }
    }
      Serial.println();

      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader

  if(tagRead == true && itemFound == false)
  {
    alertUnknownItem();
  }

  if(itemFound)
  {  
    processFoundItem();
  }

  //call serial control function when message is received from serial monitor
  while(Serial.available())
  {
    serialControl();
  }

}

/**
 * function called when item found
 */
void processFoundItem()
{
  itemFound = false;
  tagRead = false;

  //update item location entry
  if(items[foundItem].itemLocation == 255)//update item location if current status is "out"
  {
    items[foundItem].itemLocation = foundReader;    
    Serial.print(items[foundItem].itemName);
    Serial.print(F(" has been successfully tracked in location #"));
    Serial.print(foundReader);
    Serial.print('\n');

    blinkLED(green_LED_PIN, 3, 600, 100);
  } 
  else if (items[foundItem].itemLocation == foundReader) //change item location to "out" if current location matches reader location
  {
    items[foundItem].itemLocation = 255;
    Serial.print(items[foundItem].itemName);
    Serial.print(F(" has been removed from location #"));
    Serial.print(foundReader);
    Serial.print('\n');

    blinkLED(green_LED_PIN, 6, 100, 100);
  }
  else //unexpected location read
  {
    items[foundItem].itemLocation = 255;
    Serial.print("Item tracking error. ");
    Serial.print(items[foundItem].itemName);
    Serial.print(" status reset to 'not in inventory'. ");
    Serial.println();
    Serial.println("Please try again");

    blinkLED(red_LED_PIN, 3, 600, 200);
  }
  
}

/**
  * error notification if unknown tag is scanned
  */
void alertUnknownItem()
{
  Serial.println("Unidentified item, please update inventory database");
  tagRead = false;

  blinkLED(red_LED_PIN, 6, 100, 100);
}

/**
 * print the item inventory struct
 */
void printStruct()
{
  for (uint8_t itemNumber = 0; itemNumber < NUM_ITEMS; itemNumber++)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      Serial.print(items[itemNumber].idArray[i]);
      Serial.print(" ");
    }
    Serial.print(items[itemNumber].itemLocation);
    Serial.println();
  }
}


/**
 * print formatted information about inventory status
 */
void printInventory()
{
  Serial.println();
  for (uint8_t itemNumber = 0; itemNumber < NUM_ITEMS; itemNumber++)
  {
    Serial.print(items[itemNumber].itemName);
    if (items[itemNumber].itemLocation == 255)
    {
      Serial.println(" is not in the container.");
    } else
    {
      Serial.print(" is in container number ");
      Serial.print(items[itemNumber].itemLocation);
      Serial.println();
    }

  }

}

/**
 * resets inventory location to 255
 */
 void resetInventory()
 {
  for (uint8_t itemNumber = 0; itemNumber < NUM_ITEMS; itemNumber++)
  {
    items[itemNumber].itemLocation = 255;
  }
  Serial.print("Inventory reset");
 }


/**
 * process controls sent from serial monitor
 */
 void serialControl()
 {
    serialMessage = Serial.read();

    if(serialMessage == 'p')
    {
      printStruct();
    } 
    
    if (serialMessage == 'i')
    {
      printInventory();
    }

    if (serialMessage == 'r')
    {
      resetInventory();
    }

    if (serialMessage == 'R')
    {
       NVIC_SystemReset(); //system reset
    }
 }

/**
 * blink a selected LED a specified number of times
 */
void blinkLED(uint8_t ledPin, uint8_t times, unsigned long timeOn, unsigned long timeOff)
{
  for(int i = 0; i < times; i++)
  {
    digitalWrite(ledPin, HIGH);
    delay(timeOn);
    digitalWrite(ledPin, LOW);  
    delay(timeOff);  
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    // Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    // Serial.print(buffer[i], HEX);
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
}

