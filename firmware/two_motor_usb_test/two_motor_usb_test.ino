#include <Dynamixel2Arduino.h>

// OpenRB-150 한 개 + XL330-M288-T 두 개. USB로 무부하에서 작게 움직이는 시험.
Dynamixel2Arduino dxl(Serial1, -1);
const uint8_t MOTOR1_ID = 1;
const uint8_t MOTOR2_ID = 2;
const uint16_t EXPECTED_MODEL = 1200;
const int32_t CURRENT_LIMIT_MA = 100;  // 모터마다 적용. 하드웨어 전원 차단기는 아님.
const int32_t MOVE_TICKS = 228;        // 약 20도 (4096 ticks = 360도)
unsigned long lastMessage = 0;

void stopBoth() {
  dxl.torqueOff(MOTOR1_ID);
  dxl.torqueOff(MOTOR2_ID);
}

bool checked(bool success, uint8_t motorId, const char* step) {
  if (success) return true;
  int libraryError = dxl.getLastLibErrCode();
  int packetError = dxl.getLastStatusPacketError();
  stopBoth();
  Serial.print("ERROR: motor ");
  Serial.print(motorId);
  Serial.print(" / ");
  Serial.print(step);
  Serial.print(" / library=");
  Serial.print(libraryError);
  Serial.print(" packet=");
  Serial.println(packetError);
  Serial.println("Stopped. Do not increase the current limit; check the message.");
  return false;
}

void showMotors() {
  const uint8_t motorIds[2] = {MOTOR1_ID, MOTOR2_ID};
  for (uint8_t index = 0; index < 2; ++index) {
    uint8_t motorId = motorIds[index];
    Serial.print("Motor ");
    Serial.print(motorId);
    if (dxl.ping(motorId)) {
      Serial.print(" OK / Model=");
      Serial.println(dxl.getModelNumber(motorId));
    } else {
      Serial.println(" NOT FOUND");
    }
  }
}

bool waitOrStop(unsigned long milliseconds) {
  unsigned long began = millis();
  while (millis() - began < milliseconds) {
    while (Serial.available() > 0) {
      char command = Serial.read();
      if (command == 's' || command == 'S') {
        stopBoth();
        Serial.println("STOP: torque OFF. Test cancelled.");
        return false;
      }
    }
    delay(5);
  }
  return true;
}

bool prepareMotor(uint8_t motorId, int32_t &startPosition) {
  if (!checked(dxl.ping(motorId), motorId, "ping")) return false;
  if (dxl.getModelNumber(motorId) != EXPECTED_MODEL) {
    stopBoth();
    Serial.println("STOP: expected XL330-M288-T, model 1200.");
    return false;
  }
  if (!checked(dxl.torqueOff(motorId), motorId, "torque OFF")) return false;
  if (!checked(dxl.writeControlTableItem(ControlTableItem::BUS_WATCHDOG,
                                        motorId, 0), motorId, "watchdog reset")) return false;
  if (!checked(dxl.writeControlTableItem(ControlTableItem::DRIVE_MODE,
                                        motorId, 0), motorId, "drive mode")) return false;
  if (!checked(dxl.setOperatingMode(motorId, OP_CURRENT_BASED_POSITION),
               motorId, "current-based position mode")) return false;
  if (!checked(dxl.writeControlTableItem(ControlTableItem::GOAL_CURRENT,
                                        motorId, CURRENT_LIMIT_MA),
               motorId, "current setting")) return false;
  if (!checked(dxl.writeControlTableItem(ControlTableItem::PROFILE_ACCELERATION,
                                        motorId, 5), motorId, "acceleration")) return false;
  if (!checked(dxl.writeControlTableItem(ControlTableItem::PROFILE_VELOCITY,
                                        motorId, 5), motorId, "velocity")) return false;
  startPosition = dxl.readControlTableItem(ControlTableItem::PRESENT_POSITION, motorId);
  if (!checked(dxl.getLastLibErrCode() == 0 && dxl.getLastStatusPacketError() == 0,
               motorId, "read position")) return false;
  return checked(dxl.setGoalPosition(motorId, startPosition), motorId, "initial target");
}

bool reportMovement(uint8_t motorId, int32_t startPosition) {
  int32_t present = dxl.readControlTableItem(ControlTableItem::PRESENT_POSITION, motorId);
  if (!checked(dxl.getLastLibErrCode() == 0 && dxl.getLastStatusPacketError() == 0,
               motorId, "read movement")) return false;
  Serial.print("Motor ");
  Serial.print(motorId);
  Serial.print(" actual change: ");
  Serial.print((present - startPosition) * (360.0f / 4096.0f), 1);
  Serial.println(" deg");
  return true;
}

void runTest(bool useMotor1, bool useMotor2) {
  stopBoth();
  if (!checked(dxl.ping(MOTOR1_ID), MOTOR1_ID, "ping")) return;
  if (!checked(dxl.ping(MOTOR2_ID), MOTOR2_ID, "ping")) return;
  Serial.println("START: motion in 3 seconds. Send s to stop.");
  if (!waitOrStop(3000)) return;

  int32_t start1 = 0;
  int32_t start2 = 0;
  if (useMotor1 && !prepareMotor(MOTOR1_ID, start1)) return;
  if (useMotor2 && !prepareMotor(MOTOR2_ID, start2)) return;
  if (useMotor1 && !checked(dxl.torqueOn(MOTOR1_ID), MOTOR1_ID, "torque ON")) return;
  if (useMotor2 && !checked(dxl.torqueOn(MOTOR2_ID), MOTOR2_ID, "torque ON")) return;

  Serial.println("MOVE: about 20 degrees");
  if (useMotor1 && !checked(dxl.setGoalPosition(MOTOR1_ID, start1 + MOVE_TICKS),
                            MOTOR1_ID, "outward target")) return;
  if (useMotor2 && !checked(dxl.setGoalPosition(MOTOR2_ID, start2 + MOVE_TICKS),
                            MOTOR2_ID, "outward target")) return;
  if (!waitOrStop(4000)) return;
  if (useMotor1 && !reportMovement(MOTOR1_ID, start1)) return;
  if (useMotor2 && !reportMovement(MOTOR2_ID, start2)) return;

  Serial.println("RETURN: back to start");
  if (useMotor1 && !checked(dxl.setGoalPosition(MOTOR1_ID, start1),
                            MOTOR1_ID, "return target")) return;
  if (useMotor2 && !checked(dxl.setGoalPosition(MOTOR2_ID, start2),
                            MOTOR2_ID, "return target")) return;
  if (!waitOrStop(4000)) return;
  bool stopped1 = dxl.torqueOff(MOTOR1_ID);
  bool stopped2 = dxl.torqueOff(MOTOR2_ID);
  if (!stopped1 || !stopped2) {
    Serial.println("ERROR: torque OFF not confirmed. Unplug USB.");
    return;
  }
  Serial.println("DONE: both motors torque OFF.");
}

void setup() {
  Serial.begin(115200);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);
  delay(1000);
  stopBoth();
}

void loop() {
  if (millis() - lastMessage >= 3000) {
    Serial.println("READY | p: check IDs | 1: motor 1 | 2: motor 2 | b: both | s: stop");
    lastMessage = millis();
  }
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'p' || command == 'P') showMotors();
    else if (command == '1') runTest(true, false);
    else if (command == '2') runTest(false, true);
    else if (command == 'b' || command == 'B') runTest(true, true);
    else if (command == 's' || command == 'S') {
      stopBoth();
      Serial.println("STOP: torque OFF requested.");
    }
    lastMessage = millis();
  }
}
