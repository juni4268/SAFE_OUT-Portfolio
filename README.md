# SAFE_OUT — 산업 현장 안전사고 대응 게임

<!-- TODO: 대표 이미지를 Docs/images/cover.png로 넣고 아래 주석을 풀어 주세요 -->
<!-- ![SAFE_OUT cover](Docs/images/cover.png) -->

> Unreal Engine 5.7 · C++ · 3인칭 안전 교육 시뮬레이션

공장과 창고에서 무작위로 발생하는 화재, 과열, 압력 이상, 낙하물 같은 **산업 안전사고**를 제한 시간 안에 올바른 방법으로 처리해 탈출하는 게임입니다.
처리 방법이 틀리면(예: 금속 화재에 맞지 않는 소화기를 사용) 사고 원인과 결과를 보여주는 화면과 함께 실패하며, 플레이하면서 안전 수칙을 익히도록 설계했습니다.

<!-- TODO: 플레이 영상 링크로 바꿔 주세요 -->
**▶ 플레이 영상:** [YouTube](https://youtube.com/)
**⬇ 플레이 빌드 (Windows):** [Releases](../../releases)

---

## 주요 구현 내용

### 1. 확장 가능한 사고 이벤트 프레임워크 — [`CPREventBase`](Source/Project_CPR/CPREventBase.h)
- 모든 사고는 하나의 베이스 클래스를 상속하고, 상태 머신(`Idle → Active → Interacting → Resolved / Failed`)으로 관리됩니다.
- 공통 기능을 베이스에 모았습니다: 제한 시간, 자동 시작 지연, 상호작용 범위, 월드 마커 애니메이션, 실패 연출(사망 몽타주, 결과 이미지와 문구).
- 각 사고는 `OnStatusChanged`, `OnTimerExpired`, `ReceivePlayerInteraction` 등을 override해서 고유한 규칙만 구현합니다. 그래서 새 사고 유형을 추가하는 비용이 적습니다.
- `OnEventStatusChanged` 델리게이트로 UI(미션 목록, HUD)와 느슨하게 연결했습니다.

### 2. 사고 유형 (10종)
| 클래스 | 사고 | 핵심 메커니즘 |
|---|---|---|
| [`CPREvent_Overheat`](Source/Project_CPR/CPREvent_Overheat.cpp) | 설비 과열 | 오일, 냉각수, 온도 세 가지 수치를 버튼으로 조절해 균형을 맞추는 미니게임 |
| [`CPREvent_Valve`](Source/Project_CPR/CPREvent_Valve.cpp) | 배관 압력 이상 | 밸브를 돌리는 입력으로 압력을 낮춥니다 |
| [`CPREvent_Generator`](Source/Project_CPR/CPREvent_Generator.cpp) | 발전기 고장 | 문을 열고 레버를 끝까지 당겨야 하며, 절연 장갑 없이 만지면 감전됩니다 |
| [`CPREvent_FireElec`](Source/Project_CPR/CPREvent_FireElec.cpp) | 전기 화재 | 맞는 소화기 태그만 진화되고, 시간이 지나면 불이 번집니다 |
| [`CPREvent_FireLithium`](Source/Project_CPR/CPREvent_FireLithium.cpp) | 리튬(금속) 화재 | D급 금속화재 소화기가 필요합니다 |
| [`CPREvent_AirCon`](Source/Project_CPR/CPREvent_AirCon.cpp) | 유해 연기 | 연기 농도가 점점 짙어지며, 환기 장치로 제거합니다 |
| [`CPREvent_ExplosivesZone`](Source/Project_CPR/CPREvent_ExplosivesZone.cpp) | 폭발물 정리 | 맵에 흩어진 폭발물을 안전 구역으로 옮깁니다 |
| [`CPREvent_HardHat`](Source/Project_CPR/CPREvent_HardHat.cpp) | 낙하물 | 물리 시뮬레이션으로 떨어지는 낙하물에 맞을 때, 안전모를 쓰고 있으면 버티고 아니면 래그돌 처리와 함께 실패합니다 |
| [`CPREvent_Trap`](Source/Project_CPR/CPREvent_Trap.cpp) | 함정 구역 | 플레이어가 진입하면 사고가 발생합니다 |
| [`CPREventTimerActor`](Source/Project_CPR/CPREventTimerActor.cpp) | 시간 제한 이벤트 | 일정 시간 뒤에 시작됩니다 |

- [`ICPRExtinguishable`](Source/Project_CPR/CPRExtinguishable.h) 인터페이스로 "소화기로 끌 수 있는 대상"을 추상화하고, 소화기 종류(태그)에 따라 성공과 실패를 판정합니다.

### 3. 무작위 사고 배치 — [`CPREventSpawner`](Source/Project_CPR/CPREventSpawner.cpp)
- 데이터(`EventRoster`)로 사고 종류, 개수, 스폰 포인트 태그, 지연 시간 범위를 지정합니다.
- 같은 위치가 중복되지 않도록 사용한 위치를 추적하고, 스폰 포인트가 부족하면 가능한 만큼만 생성합니다.
- 플레이할 때마다 사고 구성이 달라집니다.

### 4. 플레이어와 아이템
- [`CPRActionComponent`](Source/Project_CPR/CPRActionComponent.cpp)는 Enhanced Input 처리를 컴포넌트로 분리해 이동, 달리기, 줍기, 사용, 버리기를 담당합니다. 바라보는 아이템은 하이라이트로 표시합니다.
- [`UItemData`](Source/Project_CPR/ItemData.h)(`PrimaryDataAsset`)로 아이템 정의를 데이터화해서, 메시, 아이콘, 콜리전을 코드 수정 없이 추가할 수 있습니다.

### 5. 게임 흐름 / UI
- [`CPRGameFlowManager`](Source/Project_CPR/CPRGameFlowManager.cpp)는 인트로 컷신이 끝난 뒤에 게임플레이와 이벤트 자동 시작을 엽니다.
- [`CPRClearTrigger`](Source/Project_CPR/CPRClearTrigger.cpp)는 클리어를 막는 사고가 남아 있는지 확인한 뒤 탈출을 허용합니다.
- UI 위젯
  - [`CPRMissionWidget`](Source/Project_CPR/CPRMissionWidget.cpp): 미션 목록
  - [`CPRHUDWidget`](Source/Project_CPR/CPRHUDWidget.cpp): HUD
  - [`CPROverheatWidget`](Source/Project_CPR/CPROverheatWidget.cpp): 과열 미니게임 패널
  - [`CPRDieUI`](Source/Project_CPR/CPRDieUI.cpp): 사고 원인과 결과 화면
  - [`CPRCutsceneUI`](Source/Project_CPR/CPRCutsceneUI.cpp), [`CPRTitleUI`](Source/Project_CPR/CPRTitleUI.cpp), [`CPRPauseWidget`](Source/Project_CPR/CPRPauseWidget.cpp)

<!-- TODO: 팀 프로젝트라면 팀 구성과 본인이 맡은 부분을 적어 주세요 -->

---

## 프로젝트 구조
```
Source/Project_CPR/
├─ CPREventBase.*          사고 이벤트 베이스 (상태 머신, 타이머, 마커, 실패 연출)
├─ CPREvent_*.*            사고 유형별 구현
├─ CPREventSpawner.*       무작위 사고 배치
├─ CPRExtinguishable.h     소화 가능 대상 인터페이스
├─ CPRCharacter.* / CPRActionComponent.*   플레이어, 입력
├─ Item.* / ItemData.h     아이템 (DataAsset 기반)
├─ CPRGameFlowManager.* / CPRClearTrigger.* / CPRTodoZoneTrigger.*   게임 흐름
└─ CPR*Widget.* / CPR*UI.*  UI
```

## 참고
- 이 저장소에는 **코드와 설정만** 포함되어 있습니다. 환경, 이펙트 등은 마켓플레이스(Fab) 에셋을 사용해서 라이선스상 포함하지 않았습니다. 그래서 이 저장소만으로는 에디터에서 프로젝트를 열 수 없으며, 플레이는 Releases의 빌드로 해 주세요.
- 엔진: Unreal Engine 5.7
