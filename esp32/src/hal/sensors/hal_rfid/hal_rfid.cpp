#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "hal_rfid.h"


#if MQ5_1_DEBUG == STD_ON
#define DEBUG_PRINTLN(var) Serial.println(var)
#define DEBUG_PRINT(var) Serial.print(var)
#else
#define DEBUG_PRINTLN(var)
#define DEBUG_PRINT(var)
#endif



static uint8_t lastUID[10];
static uint8_t lastUIDSize = 0;
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);



static String uid_to_string(byte *buffer, byte bufferSize)
{
#if  RFID_ENABLED == STD_ON
    String result = "";
    
    for (byte i = 0; i < bufferSize; i++) {
        if (buffer[i] < 0x10) {
            result += "0";
        }
        result += String(buffer[i], HEX);
        if (i < bufferSize - 1) {
            result += ":";  // Use colon separator for readability
        }
    }
    
    result.toUpperCase();
    return result;
    #endif
}


static bool compare_uid(byte *uid1, byte size1, byte *uid2, byte size2)
{
    #if  RFID_ENABLED == STD_ON
    if (size1 != size2) return false;
    
    for (byte i = 0; i < size1; i++) {
        if (uid1[i] != uid2[i]) return false;
    }
    
    return true;
    #endif
}

/* ==================== Public Functions ==================== */

/*
bool RFID_INIT(void)
{
    Serial.println("[RFID] Starting initialization...");
    
    // Print pin configuration
    Serial.printf("[RFID] SS Pin: %d, RST Pin: %d\n", RFID_SS_PIN, RFID_RST_PIN);
    
    // Initialize SPI with explicit pins (adjust if needed)
    // SPI.begin(SCK, MISO, MOSI, SS);
    SPI.begin();
    Serial.println("[RFID] SPI bus initialized");
    
    // Reset the reader
    digitalWrite(RFID_RST_PIN, LOW);
    delay(10);
    digitalWrite(RFID_RST_PIN, HIGH);
    delay(50);
    
    // Initialize MFRC522
    mfrc522.PCD_Init();
    delay(100);  // Give more time for initialization
    
    Serial.println("[RFID] Attempting communication test...");
    
    // Try multiple reads to verify communication
    byte version = 0;
    for (int i = 0; i < 3; i++) {
        version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
        Serial.printf("[RFID] Read attempt %d: Version = 0x%02X\n", i+1, version);
        
        if (version != 0x00 && version != 0xFF) {
            break;  // Valid version found
        }
        delay(100);
    }
    
    // Check if communication successful
    if (version == 0x00 || version == 0xFF) {
        Serial.println("[RFID] ERROR: Communication failed!");
        Serial.println("[RFID] Troubleshooting:");
        Serial.println("  1. Check wiring (SPI: MOSI, MISO, SCK, SS, RST)");
        Serial.println("  2. Verify 3.3V power supply to MFRC522");
        Serial.println("  3. Check pin definitions match your hardware");
        Serial.println("  4. Ensure SPI pins are not used by other devices");
        Serial.println("  5. Try different GPIO pins for SS and RST");
        return false;
    }
    
    // Print expected version info
    Serial.println("[RFID] ✓ Communication successful!");
    Serial.printf("[RFID] Chip version: 0x%02X ", version);
    
}
    */
///*

bool RFID_INIT(void)
{
    #if  RFID_ENABLED == STD_ON
    SPI.begin();
    mfrc522.PCD_Init();
    delay(50);  // Give reader time to initialize
    
    // Verify communication
    byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    
    if (version == 0x00 || version == 0xFF) {
        Serial.println("[RFID] ERROR: Communication failed");
        return false;
    }
    
    Serial.println("[RFID] Initialized successfully");
    Serial.printf("[RFID] Firmware version: 0x%02X\n", version);
    Serial.println("[RFID] Ready to scan cards...");
    
    return true;
    #endif
}
//*/

bool RFID_IsNewCardPresent(void)
{
    #if  RFID_ENABLED == STD_ON
    return mfrc522.PICC_IsNewCardPresent();
    #endif
}


void RFID_DiagnosticScan(void)
{
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 500) {
        if (mfrc522.PICC_IsNewCardPresent()) {
            Serial.println("[RFID] *** TAG DETECTED ***");
            if (mfrc522.PICC_ReadCardSerial()) {
                Serial.print("[RFID] UID: ");
                for (byte i = 0; i < mfrc522.uid.size; i++) {
                    Serial.printf("%02X ", mfrc522.uid.uidByte[i]);
                }
                Serial.println();
                mfrc522.PICC_HaltA();
            }
        } else {
            Serial.println("[RFID] No tag detected - Keep tag on reader");
        }
        lastCheck = millis();
    }
}


bool RFID_ReadCard(String *uid, byte *rawUID, byte *size)
{
    #if  RFID_ENABLED == STD_ON
    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }
    
    // Store UID
    lastUIDSize = mfrc522.uid.size;
    memcpy(lastUID, mfrc522.uid.uidByte, lastUIDSize);
    
    // Convert to string
    *uid = uid_to_string(mfrc522.uid.uidByte, mfrc522.uid.size);
    
    // Optional: Return raw UID bytes
    if (rawUID != nullptr && size != nullptr) {
        memcpy(rawUID, mfrc522.uid.uidByte, mfrc522.uid.size);
        *size = mfrc522.uid.size;
    }
    
    // Get card type
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    String cardType = mfrc522.PICC_GetTypeName(piccType);
    
    Serial.printf("[RFID] Card detected - UID: %s, Type: %s\n", 
                  uid->c_str(), cardType.c_str());
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    
    return true;
    #endif
}

bool RFID_CheckAuthorization(const String &uid)
{
    #if  RFID_ENABLED == STD_ON
    // Authorized UIDs list
    const char *authorizedUIDs[] = {
        "04:86:46:52:71:40:80",  // Card 1
        "59:52:67:D9",            // Card 2
        // Add more authorized UIDs here
    };
    
    const int numAuthorized = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);
    
    for (int i = 0; i < numAuthorized; i++) {
        if (uid == String(authorizedUIDs[i])) {
            Serial.printf("[RFID] Access GRANTED for UID: %s\n", uid.c_str());
            return true;
        }
    }
    
    Serial.printf("[RFID] Access DENIED for UID: %s\n", uid.c_str());
    return false;
    #endif
}

bool RFID_CheckAuthorizationRaw(byte *uid, byte size)
{
    #if  RFID_ENABLED == STD_ON
    // Authorized UIDs (byte arrays)
    const byte authorizedUID1[] = {0x04, 0x86, 0x46, 0x52, 0x71, 0x40, 0x80};
    const byte authorizedUID2[] = {0x59, 0x52, 0x67, 0xD9};
    
    if (compare_uid(uid, size, (byte*)authorizedUID1, sizeof(authorizedUID1))) {
        return true;
    }
    
    if (compare_uid(uid, size, (byte*)authorizedUID2, sizeof(authorizedUID2))) {
        return true;
    }
    
    return false;
    #endif
}


String RFID_GetLastUID(void)
{
    #if  RFID_ENABLED == STD_ON
    return uid_to_string(lastUID, lastUIDSize);
    #endif
}

void RFID_Reset(void)
{
    #if  RFID_ENABLED == STD_ON
    mfrc522.PCD_Reset();
    delay(50);
    mfrc522.PCD_Init();
    Serial.println("[RFID] Reader reset");
    #endif
}


bool RFID_SelfTest(void)
{
    #if  RFID_ENABLED == STD_ON
    Serial.println("[RFID] Running self-test...");
    bool result = mfrc522.PCD_PerformSelfTest();
    
    // Re-initialize after self-test
    mfrc522.PCD_Init();
    
    if (result) {
        Serial.println("[RFID] Self-test PASSED");
    } else {
        Serial.println("[RFID] Self-test FAILED");
    }
    
    return result;
    #endif
}


void RFID_GetStatus(void)
{
    #if  RFID_ENABLED == STD_ON
    Serial.println("\n[RFID] Reader Status:");
    Serial.printf("  Version: 0x%02X\n", 
                  mfrc522.PCD_ReadRegister(mfrc522.VersionReg));
    Serial.printf("  Last UID: %s\n", RFID_GetLastUID().c_str());
    #endif    
}


bool RFID_WriteRoomNumber(const String &roomNumber, byte *key)
{
    #if  RFID_ENABLED == STD_ON
    // Default key if not provided
    byte defaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (key == nullptr) {
        key = defaultKey;
    }
    
    // Prepare data block (16 bytes)
    byte dataBlock[16];
    memset(dataBlock, 0, 16);
    
    // Copy room number to block (max 15 chars, leave 1 for null terminator)
    int len = roomNumber.length();
    if (len > 15) len = 15;
    memcpy(dataBlock, roomNumber.c_str(), len);
    
    // Block address (Sector 1, Block 4)
    byte blockAddr = 4;
    byte trailerBlock = 7;  // Sector 1 trailer
    
    // Create key structure
    MFRC522::MIFARE_Key mifareKey;
    for (byte i = 0; i < 6; i++) {
        mifareKey.keyByte[i] = key[i];
    }
    
    // Authenticate using key A
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
        trailerBlock, 
        &mifareKey, 
        &(mfrc522.uid)
    );
    
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Authentication failed: %s\n", 
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    // Write data to block
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Write failed: %s\n", 
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    Serial.printf("[RFID] Room number '%s' written successfully to block %d\n", 
                  roomNumber.c_str(), blockAddr);
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return true;
    #endif
}


bool RFID_ReadRoomNumber(String *roomNumber, byte *key)
{
    #if  RFID_ENABLED == STD_ON
    // Default key if not provided
    byte defaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (key == nullptr) {
        key = defaultKey;
    }
    
    byte buffer[18];
    byte size = sizeof(buffer);
    byte blockAddr = 4;
    byte trailerBlock = 7;
    
    // Create key structure
    MFRC522::MIFARE_Key mifareKey;
    for (byte i = 0; i < 6; i++) {
        mifareKey.keyByte[i] = key[i];
    }
    
    // Authenticate
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &mifareKey,
        &(mfrc522.uid)
    );
    
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Authentication failed: %s\n",
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    // Read data from block
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Read failed: %s\n",
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    // Convert to string (stop at null terminator or end of data)
    *roomNumber = "";
    for (int i = 0; i < 16 && buffer[i] != 0; i++) {
        *roomNumber += (char)buffer[i];
    }
    
    Serial.printf("[RFID] Room number read: '%s'\n", roomNumber->c_str());
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return true;
    #endif
}


bool RFID_DeleteRoomNumber(byte *key)
{
    #if  RFID_ENABLED == STD_ON
    // Default key if not provided
    byte defaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (key == nullptr) {
        key = defaultKey;
    }
    
    // Prepare empty data block (all zeros)
    byte dataBlock[16];
    memset(dataBlock, 0, 16);
    
    byte blockAddr = 4;
    byte trailerBlock = 7;
    
    // Create key structure
    MFRC522::MIFARE_Key mifareKey;
    for (byte i = 0; i < 6; i++) {
        mifareKey.keyByte[i] = key[i];
    }
    
    // Authenticate
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &mifareKey,
        &(mfrc522.uid)
    );
    
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Authentication failed: %s\n",
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    // Write zeros to block
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Delete failed: %s\n",
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    Serial.printf("[RFID] Room number deleted from block %d\n", blockAddr);
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return true;
    #endif
}


bool RFID_WriteBlock(byte blockAddr, byte *data, byte dataSize, byte *key)
{
    #if  RFID_ENABLED == STD_ON
    if (dataSize > 16) {
        Serial.println("[RFID] ERROR: Data size exceeds 16 bytes");
        return false;
    }
    
    // Default key if not provided
    byte defaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (key == nullptr) {
        key = defaultKey;
    }
    
    // Prepare data block (pad with zeros if needed)
    byte dataBlock[16];
    memset(dataBlock, 0, 16);
    memcpy(dataBlock, data, dataSize);
    
    // Calculate trailer block for this sector
    byte trailerBlock = (blockAddr / 4) * 4 + 3;
    
    // Create key structure
    MFRC522::MIFARE_Key mifareKey;
    for (byte i = 0; i < 6; i++) {
        mifareKey.keyByte[i] = key[i];
    }
    
    // Authenticate
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &mifareKey,
        &(mfrc522.uid)
    );
    
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Authentication failed: %s\n",
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    // Write data
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.printf("[RFID] Write failed: %s\n",
                     mfrc522.GetStatusCodeName(status));
        return false;
    }
    
    Serial.printf("[RFID] Data written successfully to block %d\n", blockAddr);
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return true;
    #endif
}


bool RFID_FormatCard(byte *key)
{
    #if  RFID_ENABLED == STD_ON
    Serial.println("[RFID] WARNING: Formatting card - all data will be erased!");
    
    // Default key if not provided
    byte defaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (key == nullptr) {
        key = defaultKey;
    }
    
    byte emptyBlock[16];
    memset(emptyBlock, 0, 16);
    
    // Format sectors 1-15 (sector 0 contains manufacturer data, skip it)
    for (byte sector = 1; sector < 16; sector++) {
        byte trailerBlock = sector * 4 + 3;
        
        // Create key structure
        MFRC522::MIFARE_Key mifareKey;
        for (byte i = 0; i < 6; i++) {
            mifareKey.keyByte[i] = key[i];
        }
        
        // Authenticate
        MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A,
            trailerBlock,
            &mifareKey,
            &(mfrc522.uid)
        );
        
        if (status != MFRC522::STATUS_OK) {
            Serial.printf("[RFID] Auth failed for sector %d\n", sector);
            continue;
        }
        
        // Clear blocks in this sector (skip trailer block)
        for (byte blockOffset = 0; blockOffset < 3; blockOffset++) {
            byte blockAddr = sector * 4 + blockOffset;
            
            status = mfrc522.MIFARE_Write(blockAddr, emptyBlock, 16);
            if (status != MFRC522::STATUS_OK) {
                Serial.printf("[RFID] Failed to clear block %d\n", blockAddr);
            }
        }
        
        Serial.printf("[RFID] Sector %d formatted\n", sector);
    }
    
    Serial.println("[RFID] Card format complete");
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return true;
    #endif
}