# 실습 사진과 영상 기록

[프로젝트 README로 돌아가기](../../README.md)

## 등록된 사진

| 파일 | 내용 |
| --- | --- |
| [wiring-two-motors-1.jpg](wiring-two-motors-1.jpg) | USB로 연결된 OpenRB-150과 모터 두 개의 실제 연결 모습 |
| [wiring-two-motors-2.jpg](wiring-two-motors-2.jpg) | 시리얼 모니터의 두 모터 응답과 모델 번호 1200 |

사진 두 장은 프로젝트 첫 화면의 README에도 표시한다. 두 번째 사진은 통신 확인 자료이며, 움직임이나 위치 제어 정확도를 측정한 자료는 아니다.

### 하드웨어 연결

<img src="wiring-two-motors-1.jpg" alt="OpenRB-150과 XL330-M288-T 모터 두 개의 연결 모습" width="640">

### 두 모터 통신 응답

<img src="wiring-two-motors-2.jpg" alt="시리얼 모니터의 Motor 1 OK와 Motor 2 OK 및 모델 번호 1200" width="640">

## 추가할 영상

현재 이 폴더에는 영상 파일이 없다. 아래는 앞으로 촬영해 추가할 자료다.

| 저장할 파일명 | 촬영 내용 |
| --- | --- |
| `individual-control.mp4` | `1`, `2`를 각각 입력했을 때의 움직임 |
| `both-control.mp4` | `b` 입력부터 두 모터 작동·자동 종료까지 |
| `serial-stop.mp4` | 별도로 진행한 `s` 중단 시험 |

영상은 입력하는 문자와 실제 모터가 함께 보이도록 촬영한다. 목표 각도나 성공률을 측정하지 않았다면 영상 설명에도 측정 성능처럼 적지 않는다.

`both-control.mp4`를 이 폴더에 실제로 업로드한 **뒤에** 최상위 README에 아래 링크를 추가한다.

```markdown
[두 모터 함께 구동한 영상](docs/media/both-control.mp4)
```

브라우저 업로드에는 파일당 25MiB 제한이 있다. 큰 영상은 짧게 편집하거나 외부에 올린 실제 영상 주소를 README에 넣는다.

출처: [GitHub 파일 업로드 안내](https://docs.github.com/en/repositories/working-with-files/managing-files/adding-a-file-to-a-repository).
