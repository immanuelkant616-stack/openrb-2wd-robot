# 사진·영상 추가 안내

[프로젝트 README로 돌아가기](../../README.md)

현재 이 묶음에는 사진·영상 원본이 포함되어 있지 않다. 아래 이름으로 자료를 추가하면 기록을 일관되게 관리할 수 있다.

| 저장할 파일명 | 촬영 내용 |
| --- | --- |
| `wiring-two-motors.jpg` | 컴퓨터·보드·두 모터가 연결된 전체 모습 |
| `id-scan.png` | ID 1·2 및 모델 번호가 보이는 실제 검색 결과 |
| `individual-control.mp4` | `1`, `2`를 각각 입력했을 때의 움직임 |
| `both-control.mp4` | `b` 입력부터 두 모터 작동·자동 종료까지 |
| `serial-stop.mp4` | 별도로 진행한 `s` 중단 시험 |

영상은 입력하는 문자와 실제 모터가 함께 보이도록 촬영한다. 목표 각도나 성공률을 측정하지 않았다면 영상 설명에도 측정 성능처럼 적지 않는다.

연결 사진을 이 폴더에 저장한 **뒤에만** 프로젝트 README에 다음 줄을 추가한다.

```markdown
![OpenRB-150과 두 모터의 연결 모습](docs/media/wiring-two-motors.jpg)
```

영상도 실제 파일을 추가한 **뒤에만** 아래 링크를 넣는다.

```markdown
[두 모터 함께 구동한 영상](docs/media/both-control.mp4)
```

브라우저 업로드에는 파일당 25MiB 제한이 있다. 큰 영상은 짧게 편집하거나 외부에 올린 실제 영상 주소를 README에 넣는다. 촬영 자료를 추가하기 전까지 메인 README는 안내 문서로 연결하므로 없는 파일의 이미지가 표시되지 않는다.

출처: [GitHub 파일 업로드 안내](https://docs.github.com/en/repositories/working-with-files/managing-files/adding-a-file-to-a-repository).

