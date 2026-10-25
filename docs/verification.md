# 코드 보존과 검증 범위

[README로 돌아가기](../README.md)

## 이번 정리에서 확인한 내용

- 기존 실습 파일을 실행 코드와 문서로 나누어 배치했다.
- 두 `.ino` 파일은 기존 파일과 바이트 단위로 일치하도록 보존했다. 이번 묶음을 만들면서 제어 로직과 설정값을 바꾸지 않았다.
- 문서의 상대 경로, 스케치 폴더명과 파일명의 일치, Markdown 코드 블록과 ZIP 압축 구조를 확인했다.
- 이번 정리 환경에서는 새 컴파일이나 하드웨어 동작 시험을 수행하지 않았다.

## 기존 제공 코드의 컴파일 기록

이 표는 이전 실습 코드 묶음의 `VERIFICATION.md`에 보관되어 있던 기록이다. 이번에 새로 측정한 수치가 아니다.

| 항목 | 기록 |
| --- | --- |
| 보드 대상 | `OpenRB-150:samd:OpenRB-150` |
| OpenRB-150 보드 패키지 | `0.2.1` |
| Dynamixel2Arduino | `0.8.2` |
| ARM 컴파일러 | arm-none-eabi-gcc 7-2017q4 / GCC 7.2.1 |
| `set_second_motor_id` | 컴파일·링크 성공 / Flash 21192 bytes / RAM 3916 bytes |
| `two_motor_usb_test` | 컴파일·링크 성공 / Flash 39904 bytes / RAM 3916 bytes |

작성자의 실제 PC 설치 버전과 동일하다고 확인한 것은 아니다. 버전 기록은 다음 시험 때 보완한다. 컴파일 성공과 실제 동작 확인은 서로 다른 검증이며, 동작 관찰은 [시험 결과](test-results.md)에 따로 적었다.

## 포함된 코드의 SHA-256

SHA-256은 파일 내용이 같은지 비교할 때 사용하는 식별값이다. 아래 값은 포함된 두 코드 파일에서 계산했다.

| 파일 | SHA-256 |
| --- | --- |
| `firmware/two_motor_usb_test/two_motor_usb_test.ino` | `d41bd39ff74220a67b152331cd1b89c673ad706e8ae14b0ce55d0dbb9280cbdf` |
| `firmware/set_second_motor_id/set_second_motor_id.ino` | `1042305d0df0d40925b3e422dd53b263622e3094fe0ff5f25122466fdd840c3c` |
