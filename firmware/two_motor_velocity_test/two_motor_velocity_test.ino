#include <Dynamixel2Arduino.h>

#if !defined(ARDUINO_OpenRB)
#error "Select the OpenRB-150 board in Arduino IDE."
#endif

// OpenRB-150 x1 + XL330-M288-T x2, IDs 1/2, 57600 bps.
// First test: NO wheels/load. Every motion command ends after 2 seconds.
Dynamixel2Arduino dxl(Serial1, -1);
const uint8_t MOTOR_IDS[2] = {1, 2};
const uint16_t EXPECTED_MODEL = 1200;

// 35 x 0.229 = about 8 rpm. These are RAW register units, not rpm.
const int32_t SPEED_RAW = 35;
const int32_t PWM_CAP_RAW = 120;  // About 13.6% output; NOT a current limiter.
const int32_t CURRENT_TRIP_MA = 200;  // Software monitoring, per motor.
const int32_t TOTAL_CURRENT_TRIP_MA = 300;
const uint32_t RUN_MS = 2000;
const uint32_t POLL_MS = 100;
const uint32_t IO_TIMEOUT_MS = 20;
const int32_t WATCHDOG_TICKS = 25;  // 25 x 20 ms = 500 ms without bus packets.

// Car convention: ID1 = left, ID2 = right. Confirm after mounting.
// Bench keys 1/2/3/4 always use the motor's +/- direction, ignoring these signs.
const int8_t LEFT_FORWARD_SIGN = +1;
const int8_t RIGHT_FORWARD_SIGN = -1;

bool running = false;
bool faulted = true;
bool serialWasOpen = false;
bool commandedOn[2] = {false, false};
uint32_t startedAt = 0;
uint32_t lastPollAt = 0;

bool packetOK() {
  return dxl.getLastLibErrCode() == 0 && dxl.getLastStatusPacketError() == 0;
}

bool writeItem(uint8_t item, uint8_t motorId, int32_t value) {
  bool sent = dxl.writeControlTableItem(item, motorId, value, IO_TIMEOUT_MS);
  return sent && packetOK();
}

bool readItem(uint8_t item, uint8_t motorId, int32_t &value) {
  value = dxl.readControlTableItem(item, motorId, IO_TIMEOUT_MS);
  return packetOK();  // A returned zero alone does NOT prove a successful read.
}

// Always attempt BOTH motors, even if one request fails.
// Torque OFF is checked by reading back the setting. It is not a mechanical brake.
bool stopBoth() {
  running = false;
  bool confirmed = true;
  for (uint8_t i = 0; i < 2; ++i) {
    uint8_t motorId = MOTOR_IDS[i];
    writeItem(ControlTableItem::GOAL_VELOCITY, motorId, 0);
    writeItem(ControlTableItem::TORQUE_ENABLE, motorId, 0);
    int32_t torque = -1;
    bool off = readItem(ControlTableItem::TORQUE_ENABLE, motorId, torque)
               && torque == 0;
    if (!off) confirmed = false;
    commandedOn[i] = false;
  }
  if (!confirmed) faulted = true;
  return confirmed;
}

void printStopResult(bool confirmed) {
  if (confirmed) Serial.println("TORQUE_OFF_CONFIRMED: both IDs replied with torque=0.");
  else Serial.println("STOP_UNCONFIRMED: disconnect motor power and check wiring.");
}

void stopAndReport(const char *reason) {
  bool off = stopBoth();
  Serial.println(reason);
  printStopResult(off);
}

bool checked(bool ok, uint8_t motorId, const char *step) {
  if (ok) return true;
  int libError = dxl.getLastLibErrCode();
  int packetError = dxl.getLastStatusPacketError();
  faulted = true;
  bool off = stopBoth();  // Stop before printing diagnostics.
  Serial.print("ERROR ID="); Serial.print(motorId);
  Serial.print(" step="); Serial.print(step);
  Serial.print(" lib="); Serial.print(libError);
  Serial.print(" packet="); Serial.println(packetError);
  printStopResult(off);
  Serial.println("Fix the cause, then send p. Motion is locked until p succeeds.");
  return false;
}

bool prepareBoth() {
  if (!checked(stopBoth(), 0, "initial torque OFF")) return false;
  for (uint8_t i = 0; i < 2; ++i) {
    uint8_t motorId = MOTOR_IDS[i];
    if (!checked(dxl.ping(motorId) && packetOK(), motorId, "ping")) return false;
    if (!checked(dxl.getModelNumber(motorId) == EXPECTED_MODEL,
                 motorId, "expected model 1200")) return false;
    if (!checked(writeItem(ControlTableItem::BUS_WATCHDOG, motorId, 0),
                 motorId, "clear watchdog")) return false;

    // Write EEPROM settings only if they need changing (torque is already OFF).
    int32_t value = 0;
    if (!checked(readItem(ControlTableItem::DRIVE_MODE, motorId, value),
                 motorId, "read drive mode")) return false;
    if (value != 0 && !checked(writeItem(ControlTableItem::DRIVE_MODE, motorId, 0),
                              motorId, "normal drive mode")) return false;
    if (!checked(readItem(ControlTableItem::OPERATING_MODE, motorId, value),
                 motorId, "read operating mode")) return false;
    if (value != 1 && !checked(writeItem(ControlTableItem::OPERATING_MODE, motorId, 1),
                              motorId, "velocity mode")) return false;

    if (!checked(readItem(ControlTableItem::PWM_LIMIT, motorId, value) && value > 0,
                 motorId, "read PWM limit")) return false;
    int32_t outputCap = value < PWM_CAP_RAW ? value : PWM_CAP_RAW;
    if (!checked(writeItem(ControlTableItem::GOAL_PWM, motorId, outputCap),
                 motorId, "output cap")) return false;
    if (!checked(writeItem(ControlTableItem::PROFILE_ACCELERATION, motorId, 5),
                 motorId, "acceleration")) return false;
    if (!checked(writeItem(ControlTableItem::GOAL_VELOCITY, motorId, 0),
                 motorId, "zero initial velocity")) return false;
    if (!checked(writeItem(ControlTableItem::BUS_WATCHDOG, motorId, WATCHDOG_TICKS),
                 motorId, "enable watchdog")) return false;
    Serial.print("ID="); Serial.print(motorId);
    Serial.println(" OK / Model=1200 / velocity mode / torque OFF");
  }
  faulted = false;
  return true;
}

void showHelp() {
  Serial.println("READY: 1=ID1+  2=ID2+  3=ID1-  4=ID2-");
  Serial.println("CAR: w=forward x=reverse a=left d=right");
  Serial.println("s=stop | p=check/recover | h=help | every run ends after 2 seconds");
}

void startMotion(int32_t motor1Speed, int32_t motor2Speed) {
  if (faulted) {
    Serial.println("LOCKED: fix the error and send p first.");
    return;
  }
  // Recheck/reset before EACH run, including after a motor power cycle.
  if (!prepareBoth()) return;
  int32_t speeds[2] = {motor1Speed, motor2Speed};
  startedAt = millis();
  lastPollAt = startedAt;
  running = true;
  for (uint8_t i = 0; i < 2; ++i) {
    commandedOn[i] = speeds[i] != 0;
    if (!commandedOn[i]) continue;
    uint8_t motorId = MOTOR_IDS[i];
    if (!checked(writeItem(ControlTableItem::TORQUE_ENABLE, motorId, 1),
                 motorId, "torque ON")) return;
    if (!checked(writeItem(ControlTableItem::GOAL_VELOCITY, motorId, speeds[i]),
                 motorId, "target velocity")) return;
  }
  Serial.print("RUN 2s: ID1 raw="); Serial.print(motor1Speed);
  Serial.print(" ID2 raw="); Serial.println(motor2Speed);
}

void serviceMotion() {
  if (!running) return;
  if (!Serial) {
    stopAndReport("STOP: serial connection closed.");
    return;
  }
  uint32_t now = millis();
  if ((uint32_t)(now - startedAt) >= RUN_MS) {
    stopAndReport("AUTO_STOP: 2 seconds elapsed.");
    return;
  }
  if ((uint32_t)(now - lastPollAt) < POLL_MS) return;
  lastPollAt = now;
  int32_t totalCurrent = 0;
  for (uint8_t i = 0; i < 2; ++i) {
    uint8_t motorId = MOTOR_IDS[i];
    int32_t torque = 0, watchdog = 0, current = 0;
    if (!checked(readItem(ControlTableItem::TORQUE_ENABLE, motorId, torque)
                 && torque == (commandedOn[i] ? 1 : 0),
                 motorId, "torque/connection changed")) return;
    if (!checked(readItem(ControlTableItem::BUS_WATCHDOG, motorId, watchdog)
                 && watchdog == WATCHDOG_TICKS,
                 motorId, "watchdog/connection changed")) return;
    if (!checked(readItem(ControlTableItem::PRESENT_CURRENT, motorId, current),
                 motorId, "read current")) return;
    if (current < 0) current = -current;
    if (!checked(current < CURRENT_TRIP_MA, motorId, "current threshold")) return;
    totalCurrent += current;
  }
  checked(totalCurrent < TOTAL_CURRENT_TRIP_MA, 0, "total current threshold");
}

void handleSerial() {
  char command = 0;
  uint8_t count = 0;
  bool stopRequested = false;
  // Bounded input processing prevents a stream of input from starving the timer.
  for (uint8_t n = 0; n < 64 && Serial.available() > 0; ++n) {
    char c = Serial.read();
    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;
    if (c == 's') stopRequested = true;
    else { command = c; ++count; }
  }
  if (stopRequested) { stopAndReport("STOP: user command."); return; }
  if (count == 0) return;
  if (count != 1) { Serial.println("Send ONE command at a time."); return; }
  if (running) { Serial.println("BUSY: wait for AUTO_STOP, or send s."); return; }
  if (command == 'h') { showHelp(); return; }
  if (command == 'p') { if (prepareBoth()) showHelp(); return; }
  const int32_t left = SPEED_RAW * LEFT_FORWARD_SIGN;
  const int32_t right = SPEED_RAW * RIGHT_FORWARD_SIGN;
  switch (command) {
    case '1': startMotion(+SPEED_RAW, 0); break;
    case '2': startMotion(0, +SPEED_RAW); break;
    case '3': startMotion(-SPEED_RAW, 0); break;
    case '4': startMotion(0, -SPEED_RAW); break;
    case 'w': startMotion(left, right); break;
    case 'x': startMotion(-left, -right); break;
    case 'a': startMotion(0, right); break;
    case 'd': startMotion(left, 0); break;
    default: Serial.println("Unknown command. Send h for help."); break;
  }
}

void setup() {
  Serial.begin(115200);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);
  stopBoth();
  uint32_t began = millis();
  while (!Serial && (uint32_t)(millis() - began) < 3000) delay(1);
  // No motion on startup. Discard leftover input from a previous sketch.
  while (Serial.available() > 0) Serial.read();
  Serial.println("OPENRB VELOCITY TEST | no wheels/load | 8 rpm target");
  if (prepareBoth()) showHelp();
  serialWasOpen = bool(Serial);
}

void loop() {
  serviceMotion();
  if (Serial && !serialWasOpen) {
    if (faulted) Serial.println("LOCKED: send p to check both motors.");
    else showHelp();
  }
  serialWasOpen = bool(Serial);
  handleSerial();
  serviceMotion();
}
