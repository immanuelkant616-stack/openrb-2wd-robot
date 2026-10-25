# 환경 구성과 실행

[README로 돌아가기](../README.md)

## 1. 현재 상태부터 확인

이번 장비는 ID `1`, `2`의 응답과 두 모터 구동까지 확인되어 있다. **그 상태를 다시 시험할 때는 ID를 변경하지 않고 주 실행 코드만 사용한다.**

| 코드 | 사용 목적 | 연결 조건 |
| --- | --- | --- |
| [two_motor_usb_test.ino](../firmware/two_motor_usb_test/two_motor_usb_test.ino) | 개별·동시 위치 이동 시험 | ID 1·2인 두 모터가 모두 응답해야 함 |
| [set_second_motor_id.ino](../firmware/set_second_motor_id/set_second_motor_id.ino) | 새 두 번째 모터의 ID를 2로 설정 | 해당 모터 한 개만 물리적으로 연결 |

새 부품을 사용하는 경우에만 [별도 ID 설정 안내](id-setup.md)를 확인한다.

## 2. 연결과 시험 조건

USB 전원을 뺀 상태에서 모터 두 개를 보드의 다이나믹셀 단자 두 곳에 각각 연결한다. 이번 구성은 보드의 +5V 점퍼와 USB 전원으로 시험했으며, 외부 전원 단자에는 전원을 연결하지 않았다. 모터 케이블을 연결하거나 분리할 때는 전원을 먼저 끈다.

이 코드는 바퀴나 하중을 달지 않은 작은 움직임을 시험한다. OpenRB-150 공식 안내는 USB 경로에 500mA 제한이 있다고 설명한다. 코드의 `GOAL_CURRENT = 100`은 전류기반 위치 제어에서 각 모터에 적용하는 약 100mA 설정이다. 보드와 두 모터의 전체 소비 전류를 보장하는 값은 아니다. 현재 설정으로 움직임이 약하거나 연결이 끊기면 반복 구동을 멈추고 연결·전원을 확인한다.

전원 조건의 출처: [OpenRB-150 공식 안내](https://emanual.robotis.com/docs/en/parts/controller/openrb-150/), [XL330 제어 항목](https://emanual.robotis.com/docs/en/dxl/x/xl330-m288/).

## 3. Arduino IDE 환경 설치

이미 업로드에 성공한 컴퓨터는 현재 설정을 유지한다. 새 컴퓨터를 구성한다면 다음 순서를 따른다.

1. Arduino IDE를 설치한다.
2. `파일 → 환경 설정`의 추가 보드 매니저 URL에 아래 주소를 추가한다.

```text
https://raw.githubusercontent.com/ROBOTIS-GIT/OpenRB-150/master/package_openrb_index.json
```

3. 보드 매니저에서 `Arduino SAMD`와 `OpenRB`를 찾아 설치한다.
4. 라이브러리 매니저에서 `Dynamixel2Arduino`를 찾아 설치한다.
5. 보드 목록에서 `OpenRB-150`을 선택한다.

설치 순서의 출처: [ROBOTIS의 보드 매니저·라이브러리 설치 안내](https://emanual.robotis.com/docs/en/parts/controller/openrb-150/#install-board-manager).

과거 제공 코드의 컴파일 기록은 OpenRB 보드 패키지 `0.2.1`, Dynamixel2Arduino `0.8.2` 기준이다. 실제 사용자 PC의 설치 버전은 아직 기록하지 않았다. 새 환경에서 사용한 버전은 [시험 기록](test-results.md)에 남긴다.

## 4. 주 실행 코드 업로드

1. `파일 → 열기`에서 `firmware/two_motor_usb_test/two_motor_usb_test.ino`를 연다. `.ino` 파일과 바로 위 폴더의 이름을 유지한다.
2. USB로 보드를 연결하고, 보드를 OpenRB-150으로 선택한다.
3. 연결된 포트를 선택한다. Ubuntu에서는 `/dev/ttyACM0` 같은 이름이며 숫자는 달라질 수 있다.
4. `✓`로 컴파일을 확인하고, `→`로 보드에 업로드한다. 컴파일 성공만으로는 보드의 프로그램이 바뀌지 않는다.
5. 업로드 성공 여부를 확인하고 시리얼 모니터를 115200 baud로 연다.

```text
READY | p: check IDs | 1: motor 1 | 2: motor 2 | b: both | s: stop
```

이 문구가 아니라면 [오류 해결 기록](troubleshooting.md)의 안내 문구 표를 확인한다.

## 5. 순서대로 시험

1. `p`와 Enter를 보내 두 모터의 응답을 확인한다. 아래는 주 실행 코드의 예상 형식이며, 보관된 실제 ID 검색 로그와는 다른 프로그램의 출력이다.

```text
Motor 1 OK / Model=1200
Motor 2 OK / Model=1200
```

2. `1`과 Enter를 보낸 뒤 동작 종료를 기다린다.
3. `2`와 Enter를 보낸 뒤 동작 종료를 기다린다.
4. `b`와 Enter를 보내 두 모터의 움직임과 자동 종료를 확인한다.
5. 각 시험의 시리얼 출력과 관찰 내용을 [기록 양식](../logs/test-session-template.txt)에 저장한다.

시작 후 3초 대기, 이동 목표 후 4초 대기, 복귀 목표 후 4초 대기가 설정되어 있다. 이 시간은 코드의 대기 시간이며 실제 이동 시간 측정값이 아니다. 정상 종료 시 코드가 출력하는 문구는 `DONE: both motors torque OFF.`이다.

`s` 중단 시험은 별도로 진행해 결과를 기록한다. 이 코드에서 `s`는 시작이 아니라 중단 명령이며, 복귀 절차를 건너뛴다. 통신을 통해 토크 해제를 요청하는 소프트웨어 명령이므로 물리적인 전원 차단 장치와는 다르다.

## 6. 시험을 마친 뒤

정상 종료 메시지와 정지 상태를 확인하고 USB 전원을 분리한다. 코드 파일과 기록은 보관한다. 다음 단계에서 제어 모드를 바꾸기 전, 현재 작동한 파일을 기준 버전으로 남긴다.

