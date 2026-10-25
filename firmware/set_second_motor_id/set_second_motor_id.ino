#include <Dynamixel2Arduino.h>

// 이 파일은 '두 번째 모터 한 개만' 연결해서 사용합니다.
// OpenRB의 프로그램을 바꿔도 모터에 저장된 ID는 유지됩니다.
Dynamixel2Arduino dxl(Serial1, -1);
const uint8_t TARGET_MOTOR_ID = 2;
const uint16_t EXPECTED_MODEL = 1200;  // XL330-M288-T
unsigned long lastMessage = 0;

uint16_t findRespondingIds(uint8_t &foundMotorId) {
  uint16_t count = 0;
  Serial.println("Scanning IDs 0..252 at 57600 bps. Please wait...");
  for (uint16_t address = 0; address <= 252; ++address) {
    uint8_t motorId = static_cast<uint8_t>(address);
    if (dxl.ping(motorId)) {
      foundMotorId = motorId;
      ++count;
      Serial.print("FOUND: ID=");
      Serial.print(motorId);
      Serial.print(" Model=");
      Serial.println(dxl.getModelNumber(motorId));
    }
  }
  if (count == 0) {
    Serial.println("NO REPLY: check cable, +5V jumper and 57600 baud setting.");
  }
  return count;
}

void changeSecondMotorId() {
  // 같은 ID를 가진 모터 수는 검색으로 구별할 수 없습니다. 반드시 한 개만 연결!
  uint8_t currentMotorId = 0;
  uint16_t count = findRespondingIds(currentMotorId);
  if (count != 1) {
    Serial.println("STOP: leave ONLY the second motor connected, then retry.");
    return;
  }
  if (dxl.getModelNumber(currentMotorId) != EXPECTED_MODEL) {
    Serial.println("STOP: expected an XL330-M288-T (model 1200).");
    return;
  }
  if (!dxl.torqueOff(currentMotorId)) {
    Serial.println("ERROR: torque OFF failed. ID was not changed.");
    return;
  }
  if (currentMotorId == TARGET_MOTOR_ID) {
    Serial.println("OK: this motor already has ID 2.");
    return;
  }

  Serial.print("Changing ID ");
  Serial.print(currentMotorId);
  Serial.println(" -> 2");
  bool acknowledged = dxl.setID(currentMotorId, TARGET_MOTOR_ID);
  delay(200);
  if (dxl.ping(TARGET_MOTOR_ID) &&
      dxl.getModelNumber(TARGET_MOTOR_ID) == EXPECTED_MODEL) {
    Serial.println("SUCCESS: ID=2 verified. This setting stays after power OFF.");
    Serial.println("Unplug USB, then label this motor as 2.");
  } else {
    Serial.print("ERROR: new ID could not be verified. Write acknowledged: ");
    Serial.println(acknowledged ? "yes" : "no");
    Serial.println("Keep ONLY this motor connected. Send p to scan again.");
  }
}

void setup() {
  Serial.begin(115200);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);
  delay(1000);
}

void loop() {
  if (millis() - lastMessage >= 3000) {
    Serial.println("ID SETUP: second motor ONLY | p: find ID | c: set ID to 2");
    lastMessage = millis();
  }
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'p' || command == 'P') {
      uint8_t motorId = 0;
      findRespondingIds(motorId);
    } else if (command == 'c' || command == 'C') {
      changeSecondMotorId();
    }
    lastMessage = millis();
  }
}
