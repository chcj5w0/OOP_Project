# Petrochemical Factory Simulation

> GIST EECS — EC5209 OOP with C++, Spring 2026 팀 프로젝트
> 석유화학 공정(원료 → 반응 → 분리 → 출하)을 이산 틱(tick) 기반으로 시뮬레이션하고,
> Dear ImGui로 공정 전체를 실시간 시각화하는 데스크톱 애플리케이션.

![Factory Floor UI](captures/factory_floor_ui_2026-06-12.png)

상단 **Factory Floor**에 공정 전체가 그려진다: 기계 카드(상태색 테두리 + 체력/진행 게이지),
파이프라인 속을 흐르는 제품 점, 컨베이어의 슬롯별 운반 상태, 그리고 누적 생산량.
하단에는 실행 제어 / 명령 / 통계 / 이벤트 로그 / 상세 테이블이 배치된다.

---

## 1. 빌드와 실행

```bash
# 의존성: CMake ≥ 3.15, C++17 컴파일러, SDL2, OpenGL (ImGui는 libs/에 동봉)
cmake -B build
cmake --build build -j
./build/imgui_app
```

`CMakeLists.txt`는 `src/**/*.cpp`를 재귀 GLOB으로 수집하므로 **새 소스 파일을 추가해도
CMake 수정이 필요 없다.** 창 배치는 `imgui.ini`에 자동 저장되며, 흐트러졌을 때는
Run Control 창의 **Reset Layout** 버튼이나 `imgui.ini` 삭제로 기본 대시보드로 복귀한다.

---

## 2. 아키텍처 한눈에 보기

코드는 세 개의 층과 하나의 조립 지점으로 나뉜다.

```
┌─────────────────────────────────────────────────────────────────┐
│                          main.cpp                               │
│      (유일하게 core·bridge·ui 를 모두 보는 조립/배선 지점)          │
└────────────┬──────────────────────────────────┬─────────────────┘
             │                                  │
   ┌─────────▼─────────┐   POD 스냅샷/명령    ┌──▼──────────────────┐
   │     src/core/     │ ◀────────────────▶  │      src/ui/        │
   │  시뮬레이션 백엔드  │    src/bridge/      │   ImGui 프런트엔드   │
   │  (ImGui 모름)      │  MachineSnap        │  (도메인 객체 모름)  │
   │                   │  ConnectorSnap      │                     │
   │  Factory, Machine │  MachineCmd         │  FactoryFloorUI,    │
   │  Connector, …     │  FactoryStats       │  InspectorUI, …     │
   └───────────────────┘                     └─────────────────────┘
```

- **core** — 도메인 모델과 시뮬레이션 규칙. 렌더링을 전혀 모른다.
- **bridge** — 양쪽이 공유하는 **POD 자료구조**만 둔다. 메서드도, 외부 의존성도 없다.
  - `MachineSnap` / `ConnectorSnap`: 백엔드 → UI 방향의 읽기 전용 상태 사본
  - `MachineCmd`: UI → 백엔드 방향의 1회성 명령 (`startWork` / `forceBreak` / `instantRepair`)
  - `FactoryStats`: 집계 통계
- **ui** — `UIObject` 인터페이스를 구현하는 창(view)들. 스냅샷을 읽고 명령을 쓸 뿐,
  Machine 같은 도메인 객체를 직접 만지지 않는다.

이 분리 덕에 매 프레임의 데이터 흐름이 단방향 파이프라인이 된다:

```
RunControlManager.update() ── Factory::tick() ×N      (시뮬레이션 진행)
        ↓
snapshotAll() / snapshotConnectors() / stats()        (상태 → POD 사본)
        ↓
각 UIObject::render()                                  (사본을 그리기만)
        ↓
사용자 입력 → MachineCmd 플래그 세팅
        ↓
Factory::applyCmd(cmd) → 플래그 클리어                  (1회성 명령 소비)
```

UI가 보는 것은 항상 "한 틱이 끝난 시점의 일관된 사본"이므로, 그리는 도중 상태가
변하는 문제(반쯤 갱신된 상태를 그리는 것)가 구조적으로 발생하지 않는다.

---

## 3. core — 도메인 모델

### 3.1 클래스 계층

```
SimObject (abstract)  ── update(tick)=0, getInfo()=0, id()
 ├── Machine (abstract) ── 상태머신·체력·진행도·입출력 커넥터, applyCmd(), snapshot()
 │     ├── SourceTank    : N틱마다 RawFeed 를 방출하는 원료 탱크
 │     ├── Reactor       : RawFeed → Intermediate 변환 (processTicks 소요)
 │     ├── Separator     : Intermediate → FinishedProduct 변환
 │     └── TankTerminal  : 완제품을 소비·집계하는 종착 저장소
 ├── Connector (abstract) ── push/pop/size/capacity=0, loadRatio(), snapshot()
 │     ├── Pipeline      : std::deque FIFO — 즉시 통과하는 버퍼
 │     └── Conveyor      : 고정 슬롯 벨트 — 틱마다 한 칸씩 전진, 운반 지연 존재
 └── (Product 는 SimObject 가 아님 — 아래 3.4)

Product (abstract) ── name()=0, virtual ~Product
 ├── RawFeed / Intermediate / FinishedProduct   (공정 단계별 물질)
```

`Factory`는 이 모두를 `std::vector<std::unique_ptr<SimObject>>` **단일 컨테이너**로
보관하고, 틱마다 분기 없이 `o->update(tick)`만 호출한다. 기계·커넥터가 각자 무엇을
할지는 가상 함수 디스패치가 결정한다 — 새 기계를 추가해도 루프는 한 글자도 안 바뀐다.

### 3.2 Machine — Template Method + Factory Method

`Machine`이 공정의 공통 알고리즘을 소유하고, 하위 클래스는 "차이점"만 채운다.

- **공통 (기반 클래스, `Machine::advanceProgress()`)**
  틱마다: WORKING이면 체력 0.02 감소 → 0이면 BROKEN 전이 → 진행도 증가 →
  `processTicks` 도달 시 `createOutput()` 호출 → 산출물을 출력 커넥터에 push →
  실패하면 BLOCKED(`OUTPUT_FULL`) 상태로 대기, 성공하면 IDLE 복귀.
- **가변 (하위 클래스가 오버라이드)**
  | 훅 | 역할 | 패턴 |
  |---|---|---|
  | `createOutput()` | *무엇을* 생산하는가 (Reactor→Intermediate, …) | Factory Method |
  | `typeName()` | 스냅샷에 실릴 타입 이름 | — |
  | `producedCount()` | 누적 생산량 (TankTerminal만 의미 있음) | 기본값 0 |

`createOutput()`이 `nullptr`을 반환하면 "산출물 없이 작업 완료"로 처리된다.
TankTerminal이 이를 이용해 **제품을 소비하고 카운트만 올리는 종착지**를 오버라이드
세 줄로 구현한다. SourceTank는 반대쪽 특수 케이스로, `advanceProgress()` 대신
자체 `update()`에서 `emitInterval` 틱마다 RawFeed를 직접 push한다.

**상태머신** — 전이는 `update()` 내부와 `applyCmd()`(외부 명령)에서만 일어난다:

```
IDLE ──(입력 도착/startWork)──▶ WORKING ──(완료·push 성공)──▶ IDLE
                                   │ │
              (출력 가득참) ◀──────┘ └──────▶ (체력 0 / forceBreak)
                  BLOCKED                        BROKEN
                   │  push 재시도 성공             │  autoRepairDelay 틱 후
                   └──────▶ IDLE                  └──▶ IDLE (체력 0.5로 자동 수리)
```

BROKEN은 `Machine::autoRepairDelay()`(전역, Run Control 슬라이더로 조절) 틱이 지나면
자동 수리되고, `instantRepair` 명령으로 즉시 복구할 수도 있다.

### 3.3 Connector — 같은 인터페이스, 다른 물류 특성

`push(unique_ptr<Product>) → bool`, `pop() → unique_ptr<Product>`라는 계약은 같지만
두 구현은 **관찰 가능한 행동이 다르다**:

| | Pipeline | Conveyor |
|---|---|---|
| 자료구조 | `std::deque` (가변 크기 FIFO) | `std::vector` 고정 슬롯 (`nullptr` = 빈 칸) |
| 모델 | "쌓이는 통" — 얼마나 찼는지만 중요 | "위치가 있는 벨트" — 어디 있는지가 중요 |
| push | 안 찼으면 즉시 수용 | **입구 슬롯이 비어 있을 때만** 수용 |
| pop | 즉시 꺼냄 | **출구 슬롯에 도착한 제품만** 꺼냄 |
| update | 아무것도 안 함 (즉시 통과 버퍼) | 모든 제품을 출구 쪽으로 한 칸 전진 |
| 효과 | 지연 0 | 용량 C 기준 **C−1틱의 운반 지연** + 입구 백프레셔 |

자료구조 선택이 의미론을 그대로 반영한다: deque는 양끝 O(1) 연산이 필요한 큐의
정석이고, vector-of-slots는 "벨트 위 물리적 위치"를 인덱스로 표현하기 위함이다.

**백프레셔** — 실패를 예외가 아닌 반환값(`false`/`nullptr`)으로 알리고, 기계가 이를
BLOCKED 상태로 번역한다. 하류가 막히면 커넥터가 차고 → 상류 기계가 BLOCKED되고 →
그 입력 커넥터가 차고 → … 병목이 자연스럽게 상류로 전파된다.

### 3.4 Product — 소유권 파이프라인

Product는 데이터 없는 **마커 계층**이지만, 코드 전체에서 `std::unique_ptr<Product>`
로만 다뤄진다는 점이 핵심이다:

```
SourceTank ─push→ Pipeline ─pop→ Reactor ─push→ Pipeline ─pop→ Separator
                                                    ─push→ Conveyor ─pop→ TankTerminal (소멸)
```

- 생성(`make_unique`) → 이동(`std::move`로 push) → 소비(pop 후 폐기·교체)의 전 과정에서
  생포인터·수동 delete가 없다 — **RAII로 메모리 누수가 구조적으로 불가능**하다.
- Reactor/Separator는 받은 제품을 버리고 **다른 타입의 새 제품을 생성**한다.
  "화학 반응으로 물질이 변한다"를 객체 교체로 모델링한 것.
- `Product::name()`은 커넥터 스냅샷(`slotNames()`)을 통해 UI까지 전달되어,
  Floor 화면에서 제품 종류별 색(갈색 RawFeed / 주황 Intermediate / 초록 완제품)으로 그려진다.

### 3.5 Factory — 조립과 한 틱의 생애

`Factory::buildScenarioNormal()`이 토폴로지를 조립한다:

```
SourceTank[1] →P[2]→ Reactor[3] →P[4]→ Separator[5] →C[6]→ TankTerminal[7]
 (3틱마다 방출)  cap8   (4틱 공정)  cap8    (3틱 공정)  cap8     (1틱 처리)
```

`Factory::tick()` 한 번은: ① 모든 기계의 직전 상태/체력 저장 → ② 등록 순서대로
전 객체 `update()` → ③ 직전/현재 상태를 비교해 **전이가 일어난 순간만** 이벤트 발행
(BROKEN/REPAIRED/BLOCKED/WORKING, 체력 30% 임계, 완제품 증가) → ④ 틱 증가.
객체 등록 순서가 곧 공정 흐름 순서라서, 같은 틱 안에서 "분리기가 push → 벨트 전진 →
터미널이 pop"이 자연스럽게 이어진다.

---

## 4. ui — 시각화 레이어

모든 창은 `UIObject`(순수 가상 `render()`)를 구현하고, main 루프는
`for (UIObject* ui : uiObjects) ui->render();` 한 줄로 전부 그린다 — 백엔드의
`SimObject` 루프와 대칭인 **UI 쪽 다형성**이다.

| 창 | 역할 |
|---|---|
| **Factory Floor** | 핵심 시각화. ImDrawList로 직접 그리는 공정 다이어그램 (아래 상세) |
| **Inspector** | 전 기계의 상태·체력·진행·적재·생산을 한 줄씩 보여주는 테이블 |
| **Simulation Control** | 대상 기계 콤보 선택 + Start Work / Force Break / Instant Repair 버튼 |
| **Run Control** | Pause/Resume/Step, 틱 간격·자동수리 지연 슬라이더, Reset Layout |
| **Statistics** | 현재 틱, 완제품 수, 상태별 기계 수, 총 적재량 |
| **Event Log** | `[Tick N] Machine 3 WORKING` 형식의 상태 전이 이력 (최근 100건) |

### Factory Floor 상세

- **기계 카드**: 테두리 = 상태색(IDLE 회색·WORKING 파랑·BLOCKED 주황·BROKEN 빨강),
  체력 게이지(초록→빨강 그라데이션), 작업 진행 게이지, 하단에 고장 카운트다운·차단
  사유·누적 생산량. 호버 시 상세 툴팁.
- **Pipeline**: 둥근 관 안에 대기 제품이 색 점으로 쌓인다 (출구 쪽이 pop 순서 선두).
- **Conveyor**: 슬롯이 개별 칸으로 그려져 제품이 왼쪽(입구)→오른쪽(출구)으로
  **매 틱 한 칸씩 이동하는 것이 보인다.**
- **상호작용**: 좌클릭 = 명령 대상 선택(흰 테두리 강조, Simulation Control과 동기화),
  우클릭 = 명령 컨텍스트 메뉴. 그리기 순서는 하드코딩이 아니라 `MachineSnap`의
  `outputId`로 커넥터를 찾아가므로 토폴로지가 바뀌어도 자동 대응된다.

### 스타일과 배치의 단일 출처

- **`StateStyle.h`** — 상태→색/라벨, 제품→색, 체력→색 매핑을 전부 모은 헤더.
  어떤 창도 "BROKEN이 무슨 색인지" 독자적으로 정하지 않는다.
- **`UILayout.h`** — 여섯 창의 기본 배치 좌표를 한 곳에 정의. 평소엔
  `ImGuiCond_FirstUseEver`(저장된 `imgui.ini` 존중)로 적용하고, **Reset Layout** 버튼이
  눌리면 다음 한 프레임 동안 `ImGuiCond_Always`로 전 창을 강제 복귀시킨다.
  (버튼은 프레임 중간에 눌리므로 즉시 적용하면 이미 그려진 창이 빠진다 —
  요청은 이번 프레임, 적용은 다음 프레임 전체라는 2단계 플래그로 해결.)

---

## 5. 적용된 OOP 개념·디자인 패턴 정리

| 개념/패턴 | 적용 위치 | 효과 |
|---|---|---|
| 추상 클래스 + 순수 가상 | `SimObject`, `Machine`, `Connector`, `Product`, `UIObject` | 인스턴스화 차단, 계약 강제 |
| 다형성 (서브타입) | `Factory::tick()`·main 루프의 분기 없는 단일 for문 | 새 타입 추가 시 루프 무수정 |
| **Template Method** | `Machine::advanceProgress()`, `Connector::snapshot()` | 공통 알고리즘 1회 작성, 하위는 훅만 구현 |
| **Factory Method** | `Machine::createOutput()` | 생산물 결정을 하위 클래스에 위임 |
| **Bridge (Cmd/Snapshot)** | `src/bridge/`의 POD struct들 | UI↔백엔드 컴파일 의존성 절단 |
| 캡슐화 | 전 클래스 데이터 멤버 private (+protected 세터) | 상태 전이 경로를 메서드로 한정 |
| RAII / 소유권 이동 | `unique_ptr<Product>`의 push/pop 체인 | 누수 불가능, 소유권이 시그니처에 명시 |
| LSP (치환 가능성) | `Connector*`만 아는 Machine에 Pipeline/Conveyor 교체 장착 | 시나리오 조립의 자유 |
| 단일 출처 원칙 | `StateStyle.h`(색), `UILayout.h`(배치) | 뷰 간 불일치 방지 |
| 상태 패턴 (경량) | `MachineState` enum + 전이 규칙의 메서드 한정 | 상태 전이 추적 용이 |

---

## 6. 설계 평가 — 잘된 점과 알려진 한계

> 강의 자료(SOLID, Observer/Memento/State 패턴)와의 상세 매핑 및 개선 로드맵은
> [docs/DESIGN_PATTERNS.md](docs/DESIGN_PATTERNS.md) 참조.

### 잘된 점

1. **두 개의 대칭적 다형성 루프** — 백엔드(`SimObject`)와 UI(`UIObject`)가 같은 구조.
   기계든 창이든, 추가할 때 기존 루프·분기를 건드리지 않는다.
2. **스냅샷 경계의 일관성** — UI는 어떤 경로로도 도메인 객체에 닿을 수 없고,
   백엔드는 어떤 ImGui 타입도 모른다. 한쪽을 통째로 교체(예: 텍스트 UI, 테스트 하네스)
   해도 다른 쪽이 안전하다.
3. **하위 클래스가 얇다** — 구체 기계가 각각 ~30줄. 공통 로직이 잘 끌어올려졌다는 증거.
4. **Conveyor의 행동적 차별화** — 서브타입이 이름만 다른 게 아니라 운반 지연·입구
   백프레셔라는 관찰 가능한 차이를 가진다. 상속 계층의 존재 이유가 동작으로 증명된다.

### 알려진 한계 (개선 후보)

1. **core → ui 역방향 의존** — `Factory.cpp`가 이벤트 발행을 위해 `ui/EventLogUI.h`를
   직접 include한다. 레이어 규칙("core는 ImGui/ui를 모른다")의 유일한 위반.
   *개선안*: bridge에 `IEventSink` 인터페이스(또는 이벤트 POD 큐)를 두고 Factory는
   그것만 알게 하기. `RunControlManager`가 ui에 있으면서 `core/Factory.h`를 직접
   조작하는 것도 같은 맥락 — 컨트롤러로서 의도된 예외라면 문서화가 필요하다.
2. **Reactor/Separator/TankTerminal의 `update()`가 글자 단위로 동일** —
   "수리 틱 → BROKEN 체크 → IDLE이면 입력 pop → advanceProgress" 흐름을
   `Machine::update()` 기본 구현으로 끌어올리고 SourceTank만 오버라이드하면 중복 제거.
3. **입력 제품을 검증 없이 폐기** — pop한 제품을 트리거로만 쓰고 버린다. 토폴로지가
   자유로워지면 `accepts(const Product&)` 같은 타입 검증 훅이 필요하다.
4. **매직 넘버** — 체력 감소량 0.02, 자동수리 후 체력 0.5 등이 코드에 박혀 있다.
   StateStyle/UILayout처럼 상수 헤더로 모으면 시나리오 튜닝이 쉬워진다.
5. **시나리오 1종** — PDF의 Bottleneck/Random Breakdown 시나리오용
   `buildScenarioXxx()` 추가가 다음 단계.

---

## 7. 파일 맵

```
my_imgui_app/
├── CMakeLists.txt              # SDL2+OpenGL, src/**.cpp 자동 수집
├── README.md                   # (이 문서) 프로젝트 전체 설명
├── docs/                       # StarUML 다이어그램 (.mdj/.png) + DESIGN_PATTERNS.md (수업 내용 매핑)
├── captures/                   # 실행 스크린샷
├── libs/imgui/                 # Dear ImGui 소스 (동봉)
└── src/
    ├── main.cpp                # SDL/GL/ImGui 초기화 + 3층 조립 + 프레임 루프
    ├── core/
    │   ├── SimObject.h         # 추상 루트 (update/getInfo/id)
    │   ├── Machine.h/.cpp      # 추상 기계: 상태머신, advanceProgress, applyCmd, snapshot
    │   ├── machines/           # SourceTank, Reactor, Separator, TankTerminal
    │   ├── Connector.h/.cpp    # 추상 커넥터: push/pop 계약 + snapshot 조립
    │   ├── Pipeline.h/.cpp     # deque FIFO 버퍼
    │   ├── Conveyor.h/.cpp     # 고정 슬롯 벨트 (틱당 한 칸 전진)
    │   ├── Product.h/.cpp      # Product/RawFeed/Intermediate/FinishedProduct
    │   └── Factory.h/.cpp      # 조립(buildScenarioNormal), tick, 이벤트 감지, 스냅샷 수집
    ├── bridge/                 # POD만: MachineSnap, ConnectorSnap, MachineCmd, FactoryStats
    └── ui/
        ├── UIObject.h          # 추상 뷰 (render()=0)
        ├── FactoryFloorUI.*    # ImDrawList 공정 다이어그램 (시각화 핵심)
        ├── InspectorUI.*       # 전 기계 상세 테이블
        ├── SimControlUI.*      # 대상 선택 + 1회성 명령 버튼
        ├── RunControlManager.* # 틱 드라이버 (Pause/Step/속도) + Reset Layout
        ├── StatisticsUI.*      # 집계 통계
        ├── EventLogUI.*        # 상태 전이 이력
        ├── StateStyle.h        # 상태/제품/체력 → 색·라벨 단일 출처
        └── UILayout.h          # 창 기본 배치 단일 출처 + 리셋 메커니즘
```
