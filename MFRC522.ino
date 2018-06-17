#include <SPI.h>
#include <MFRC522.h>

/*  Author: Patrick Joseph McGee
 *  SIMSS
 *  RFID_Absent3.ino
 */

#define LED_PIN         A0          // Red LED
#define LED_PIN2        7           // Yellow LED
#define RST_PIN         9           // Reset pin on RFID reader
#define SS_PIN          10          // Slave select pin on RFID reader

MFRC522 rfid522(SS_PIN, RST_PIN);   // Create MFRC522 instance

/* One-time setup upon power-on.
 *  Initialize Serial @9600 baud.
 *  Init SPI for device communication.
 */
void setup() {
  Serial.begin(9600);                                           // Init serial @ 9600 baud
  SPI.begin();                                                  // Init SPI bus
  rfid522.PCD_Init();                                           // Init MFRC522 card
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
}

/* print_hex:
 *  Data in bytes, length -> send hex value to serial.
 */
void print_hex(uint8_t *data, uint8_t length)
{
       char tmp[16];
       for (int i=0; i<length; i++) { 
         sprintf(tmp, "%.2X",data[i]);        // Convert current data buffer to hex, store in tmp
         Serial.print(tmp);                   // Send to serial
         Serial.print(":");
       }
}

MFRC522::Uid id;

/* cpid function:
 *  ID -> copy to RFID structure.
 */
void cpid(MFRC522::Uid *id){
  memset(id, 0, sizeof(MFRC522::Uid));
  memcpy(id->uidByte, rfid522.uid.uidByte, rfid522.uid.size);
  id->size = rfid522.uid.size;
  id->sak = rfid522.uid.sak;
}

/*
 * loop() function ->
 * Loop forever, looking for RFID tags.
 * Output corresponding tag name upon read.
 */
String strID = "";
uint8_t control = 0x00;
int ct = 10;
void loop() {

  /* 
   *  Firstly, initialize the key and status.
   *  Status remains 0 until tag is read.
   */
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status;

  /* 
   *  Proximity Integrated Circuit Card (or, "tag").
   *  PICC_IsNewCardPresent and PICC_ReadCardSerial
   *  return true if a tag is 1) present and 2) readable.
   */
  if ( !rfid522.PICC_IsNewCardPresent()) {
    digitalWrite(LED_PIN2, HIGH);
    return;
  }
  if ( !rfid522.PICC_ReadCardSerial()) {
    digitalWrite(LED_PIN2, HIGH);
    return;
  }

  /*
   * Get the ID of the tag that has been read.
   * Print it in hex. Send alert and tag info.
   */
  cpid(&id);
  Serial.println();
  Serial.println();
  Serial.print("******************\nRFID TAG DETECTED: ");
  print_hex(id.uidByte, id.size);

  digitalWrite(LED_PIN2, LOW);
  digitalWrite(LED_PIN, HIGH);

  /* Run forever, break when tag removed. */
  while(true){
    
    control=0;
    
    for(int i=0; i<3; i++){

      /* A new tag is read. */
      if(!rfid522.PICC_IsNewCardPresent()){

        /* Read the newly found tag, and fill control bits. */
        if(rfid522.PICC_ReadCardSerial()){
          control |= 0x16;
        }

        /* If tag is still present and already read (not new), increment by 1. */
        control += 0x1;
      }
      
      /* If no new card present, increment by four. */
      control += 0x4;
    }

    Serial.println("\n");
    
    /* Tag is present if control value is 13 or 14. 
     * Delay printing for 500ms to reduce headache. */
    if(control == 13 || control == 14){
      print_hex(id.uidByte, id.size);
      Serial.print(" still present.");
      delay(500);
    }
    /* If control is anything else, tag is not present anymore.
     *  Break from loop. */
    else {
      break;
    }
  }

  /* When tag removed, send alert, LED back to default state. 
   * Next, delay another read for 200ms. */
  Serial.println();
  Serial.println("\nRFID TAG REMOVED");
  Serial.println("================");
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_PIN2, HIGH);
  delay(200);

  /* Perform analysis on item 'scalpel' */
  if(ct >= 0){
      Serial.print("Scalpel uses left: ");
      Serial.println(ct);
  }
  if(ct <= 5 && ct != 0)
      Serial.println("WARNING : Item 'scalpel' is near end of lifespan");

  if(ct == 0)
      Serial.println("ALERT : Replace item 'scalpel'");

  if(ct > 0)
      ct--;

  /* Halt the reader until next tag is read. */
  rfid522.PICC_HaltA();
  rfid522.PCD_StopCrypto1();
}


