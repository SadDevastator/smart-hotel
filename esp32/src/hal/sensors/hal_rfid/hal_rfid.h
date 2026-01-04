#ifndef HAL_RFID_H
#define HAL_RFID_H


#define RFID_SS_PIN 21
#define RFID_RST_PIN 22

// function declarations
bool RFID_INIT(void);

bool RFID_IsNewCardPresent(void);

void RFID_DiagnosticScan(void);
bool RFID_ReadCard(String *uid, byte *rawUID = nullptr, byte *size = nullptr);

bool RFID_CheckAuthorization(const String &uid);
bool RFID_CheckAuthorizationRaw(byte *uid, byte size);
String RFID_GetLastUID(void);
void RFID_Reset(void);
bool RFID_SelfTest(void);
void RFID_GetStatus(void);
bool RFID_WriteRoomNumber(const String &roomNumber, byte *key = nullptr);

bool RFID_ReadRoomNumber(String *roomNumber, byte *key = nullptr);
bool RFID_DeleteRoomNumber(byte *key = nullptr);
bool RFID_WriteBlock(byte blockAddr, byte *data, byte dataSize, byte *key = nullptr);
bool RFID_FormatCard(byte *key = nullptr);

#endif