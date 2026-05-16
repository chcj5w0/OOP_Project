# Petrochemical Factory Simulation — 프로젝트 골격

> EC5209 / OOP with C++ — Spring 2026, GIST EECS
> 주제: **석유화학 공정(Petrochemical Process) 시뮬레이션** (구체 공정은 추후 확정)
> 1차 목표: 단위 기계(Unit Machine) + 기계 간 연결(Pipeline / Conveyor) 구현

---

## 0. 이번 단계의 범위 (Phase 1)

PDF 요구사항(`factory_project_v3.pdf`) 중 **Phase 1**에서는 아래만 다룬다.

- [x] 단위 기계의 추상 타입 계층 (`SimObject → Machine → 구체 기계`) — `src/core/`
- [x] 기계 간 연결(`Connector → Pipeline / Conveyor`) 1차 버전 — `src/core/Pipeline.*`, `Conveyor.*`
- [x] 시뮬레이션 루프가 분기 없이 모든 객체에 `update(tick)` 호출 — `Factory::tick()`
- [x] UI ↔ Backend 분리 (Cmd / Snapshot 패턴) — `src/bridge/`
- [x] ImGui 5개 윈도우 중 **Simulation Control / Factory Floor / Inspector** 우선 — `src/ui/`
- [x] 시나리오 1종 빌드 (`buildScenarioNormal`) — Source→Pipe→Reactor→Pipe→Separator→Pipe 토폴로지

**Phase 1 빌드/실행 검증 완료** (Ubuntu 22.04, SDL2 2.0.20, 2026-05-16 기준).

Phase 2(이후): Event Log/Statistics 실제 데이터 채우기, 4종 시나리오 셀렉터, breakdown/worker, 저장/복원.

---

## 1. 도메인 모델 (석유화학)

석유화학 공정의 공통 특징을 일반화해서, 도메인이 확정되어도 클래스 이름만 바꾸면 되도록 설계한다.

| 추상 역할 | 의미 | 후보 구체 기계 (도메인 확정 시 결정) |
|---|---|---|
| **Source** | 원료 유입 (탱크/공급 라인) | `CrudeTank`, `NaphthaFeed`, `MonomerFeed` |
| **Reactor** | 화학 반응을 일으키는 장치 | `Cracker`, `Reformer`, `PolymerReactor` |
| **Separator** | 혼합물을 분리 (증류/원심/막) | `DistillationColumn`, `Flash`, `Splitter` |
| **Mixer/Blender** | 혼합/배합 | `Blender`, `Quench` |
| **Sink** | 최종 저장/출하 | `ProductTank`, `Pelletizer` |

> 모든 구체 기계는 `Machine`을 상속하고, `update(tick)`만 가지고 시뮬레이션 루프에서 다형적으로 동작한다. 새 기계 추가 시 루프/UI 어느 쪽도 수정하지 않는다.

### 1.1 제품(Product) 추상화

석유화학 흐름은 "상(phase)"이 바뀌는 게 특징이라, Product 계층도 두 단계 이상.

- `Product` (abstract)
  - `RawFeed` — 원료 (예: 원유, 나프타, 단량체)
  - `Intermediate` — 중간 생성물 (예: 분해유, 반응 생성물)
  - `FinishedProduct` — 최종 (예: 가솔린, 폴리머 펠릿)

각 Machine은 입력 Product 타입과 출력 Product 타입을 명시적으로 가진다(템플릿이 아니라 `std::unique_ptr<Product>` 다형).

---

## 2. 클래스 계층 (UML 초안)

```
                  ┌──────────────────┐
                  │   SimObject      │  <abstract>
                  │  +update(tick)=0 │
                  │  +getInfo()=0    │
                  │  +id() const     │
                  └────────┬─────────┘
                           │
            ┌──────────────┴───────────────┐
            │                              │
       ┌────▼─────┐                  ┌─────▼──────┐
       │ Machine  │ <abstract>       │ Connector  │ <abstract>
       │ -state   │                  │ -capacity  │
       │ -health  │                  │ -load      │
       │ +applyCmd│                  │ +push/pop  │
       │ +snapshot│                  └─────┬──────┘
       └────┬─────┘                        │
            │                       ┌──────┴──────┐
   ┌────────┼────────┐              │             │
   │        │        │           ┌──▼───┐    ┌────▼─────┐
┌──▼──┐ ┌───▼───┐ ┌──▼────┐      │Pipe  │    │Conveyor  │
│Src  │ │Reactor│ │Separa │      │(연속) │    │(이산 큐) │
│Tank │ │       │ │tor    │      └──────┘    └──────────┘
└─────┘ └───────┘ └───────┘
   …      …          …       (구체 클래스들 — 도메인 확정 후 명명)
```

핵심 설계 원칙
1. **단일 추상 루트(`SimObject`)** — Factory는 `std::vector<std::unique_ptr<SimObject>>` 하나로 모두 보관.
2. **2단계 상속** — `SimObject → Machine → ConcreteMachine`, `SimObject → Connector → Pipe/Conveyor`.
3. **퍼블릭 데이터 멤버 금지** — 모든 필드 `private`/`protected`. 외부는 메서드로만 접근.
4. **루프 분기 없음** — 시뮬레이션 루프는 `for (auto& o : objs) o->update(tick);` 한 줄.

---

## 3. 시뮬레이션 루프 & 상태머신

### 3.1 Machine 상태 (state machine)

```
   ┌────────┐ start  ┌──────────┐ done  ┌────────┐
   │ IDLE   │───────▶│ WORKING  │──────▶│ IDLE   │
   └────┬───┘        └────┬─────┘       └────────┘
        │ break             │ break
        ▼                   ▼
     ┌──────────┐      ┌──────────┐
     │ BROKEN   │◀─────│ BROKEN   │
     └────┬─────┘      └────┬─────┘
          │ repair          │ repair
          ▼                 ▼
        IDLE              IDLE
```

전이는 오직 `Machine::update()` 내부 + `applyCmd()`(외부 명령)에서만 발생.

### 3.2 단일 틱(tick) 처리

```cpp
// main loop, 단일 책임: tick 진행 + cmd 전달 + snapshot 수확
void Factory::tick() {
    for (auto& o : m_objects) o->update(m_currentTick);
    ++m_currentTick;
}
```

기계 간 자재 이동은 `Connector`를 통해 일어난다. `Machine::update()`는 출력 Connector에 `push()`를 시도하고, 입력 Connector에서 `pop()`을 시도한다. Connector가 가득 차면 "blocked" 상태가 되어 백프레셔(back-pressure)가 자연스럽게 생긴다 — 이게 PDF의 *Bottleneck* / *Overflow* 시나리오의 기반.

---

## 4. UI ↔ Backend 분리 (PDF §4.1)

```
 ┌──────────────────────┐     MachineCmd      ┌──────────────────────┐
 │   ImGui UI Layer     │ ──────────────────▶ │  Factory / Machine   │
 │  (Inspector, Floor,  │                     │  (no ImGui include)  │
 │   SimControl)        │ ◀────────────────── │                      │
 └──────────────────────┘    MachineSnap      └──────────────────────┘
            ▲                                          ▲
            └───────────────── main.cpp ───────────────┘
                  (양쪽 모두 보는 유일한 파일)
```

- **`MachineCmd`** — POD struct. `bool startWork; bool forceBreak; bool instantRepair;` 등. 메서드 없음.
- **`MachineSnap`** — POD struct. `State state; float health; int queueDepth; int outputCount;` 등. UI는 이것만 읽는다.
- `main.cpp`만 `factory.applyCmd(cmd); cmd = {};` 호출.
- 백엔드 파일은 `imgui.h`를 절대 include하지 않는다(컴파일 차원에서 강제).

---

## 5. 파일/디렉토리 구조 (현 구현 상태)

```
my_imgui_app/
├── CMakeLists.txt                 ← SDL2 + GLOB_RECURSE 자동 수집
├── factory_project_v3.pdf
├── PROJECT_SKELETON.md            ← (이 문서)
├── docs/
│   ├── Machine_MVC.mdj            ← 초기 MVC 토글 데모 스냅샷 (참고용)
│   └── UML.mdj                    ← Phase 1 골격 UML (StarUML)
├── libs/imgui/                    ← ImGui 소스
├── .vscode/                       ← IntelliSense 설정 (compile_commands.json 연동)
└── src/
    ├── main.cpp                   ← 유일한 양쪽-가시 파일 (Factory + UI 와이어링)
    │
    ├── core/                      ← 백엔드 (ImGui 미포함)
    │   ├── SimObject.h            ← abstract root
    │   ├── Machine.h / .cpp       ← abstract (typeName, onProcessComplete 강제)
    │   ├── Connector.h / .cpp     ← abstract (push/pop/size/capacity)
    │   ├── Pipeline.h / .cpp      ← Connector 구체 (std::deque FIFO)
    │   ├── Conveyor.h / .cpp      ← Connector 구체 (Phase 2용 슬롯 배열)
    │   ├── Product.h / .cpp       ← Product/RawFeed/Intermediate/FinishedProduct
    │   ├── Factory.h / .cpp       ← tick() + applyCmd + snapshotAll + buildScenarioNormal
    │   └── machines/
    │       ├── SourceTank.h/.cpp  ← N틱마다 RawFeed 생성
    │       ├── Reactor.h/.cpp     ← RawFeed → Intermediate (processTicks)
    │       └── Separator.h/.cpp   ← Intermediate → FinishedProduct (processTicks)
    │
    ├── bridge/                    ← POD only, 의존성 없음
    │   ├── MachineCmd.h           ← targetId + startWork/forceBreak/instantRepair
    │   └── MachineSnap.h          ← id/typeName/state/health/progress/processTicks/in&out
    │
    └── ui/                        ← ImGui만 의존, 백엔드 헤더 미포함
        ├── SimControlUI.h/.cpp    ← Cmd 작성 위젯
        ├── FactoryFloorUI.h/.cpp  ← 모든 머신 상태 리스트
        ├── InspectorUI.h/.cpp     ← 선택된 머신 1개 상세
        ├── EventLogUI.h/.cpp      ← (Phase 2: 더미)
        └── StatisticsUI.h/.cpp    ← (Phase 2: 더미)
```

**의존성 규칙(현재 디렉토리 분할로 표현)**
- `core/*` → `<vector>`, `<memory>`, `<string>`, `bridge/*` 만. 절대 `imgui.h` 금지.
- `ui/*` → `imgui.h` + `bridge/*` 만. `core/*` 헤더 금지.
- `bridge/*` → 표준 라이브러리만. 양쪽 어디서도 안전하게 include.
- `main.cpp` → 셋 다 include 가능 (유일하게 허용된 다리).

---

## 6. 주요 인터페이스 초안

### 6.1 `SimObject` (추상 루트)

```cpp
class SimObject {
public:
    virtual ~SimObject() = default;
    virtual void        update(int tick) = 0;    // ImGui 호출 금지
    virtual std::string getInfo() const = 0;     // 이벤트 로그용 한 줄
    int                 id() const { return m_id; }
protected:
    explicit SimObject(int id) : m_id(id) {}
private:
    int m_id;
};
```

### 6.2 `Machine` (추상)

```cpp
enum class MachineState { IDLE, WORKING, BLOCKED, BROKEN };

class Machine : public SimObject {
public:
    void          applyCmd(const MachineCmd& cmd);
    MachineSnap   snapshot() const;
    void          attachInput (Connector* c) { m_in  = c; }
    void          attachOutput(Connector* c) { m_out = c; }
protected:
    Machine(int id, int processTicks);
    virtual void onProcessComplete() = 0;     // 자식이 출력 생성
private:
    MachineState  m_state    = MachineState::IDLE;
    float         m_health   = 1.0f;
    int           m_progress = 0;
    int           m_processTicks;
    Connector*    m_in  = nullptr;
    Connector*    m_out = nullptr;
};
```

### 6.3 `Connector` (추상)

```cpp
class Connector : public SimObject {
public:
    virtual bool push(std::unique_ptr<Product> p) = 0;  // 가득 차면 false
    virtual std::unique_ptr<Product> pop()        = 0;  // 비어 있으면 nullptr
    virtual int  size()     const = 0;
    virtual int  capacity() const = 0;
    float        loadRatio() const { return capacity() ? float(size())/capacity() : 0.f; }
};
```

`Pipeline`(연속 흐름; FIFO 큐 + 지연 틱)과 `Conveyor`(이산 슬롯; Phase 2)가 이걸 구현.

### 6.4 `MachineCmd` / `MachineSnap` (브리지)

```cpp
struct MachineCmd {
    bool startWork     = false;
    bool forceBreak    = false;
    bool instantRepair = false;
    int  targetId      = -1;   // 어느 머신에 적용할지
};

struct MachineSnap {
    int          id;
    const char*  typeName;     // "Reactor", "Separator", …
    MachineState state;
    float        health;       // 0.0 ~ 1.0
    int          progress;     // 0 ~ processTicks
    int          processTicks;
    int          outputCount;
    int          inputLoad;    // 입력 connector의 현재 적재량
};
```

### 6.5 `Factory`

```cpp
class Factory {
public:
    void  tick();
    void  applyCmd(const MachineCmd& cmd);
    std::vector<MachineSnap> snapshotAll() const;
    int   currentTick() const { return m_tick; }

    // 빌더 — 시나리오별로 다른 토폴로지 구성
    void  buildScenarioNormal();
    void  buildScenarioBottleneck();   // Phase 2
private:
    std::vector<std::unique_ptr<SimObject>> m_objects;  // 단일 컨테이너
    int m_tick = 0;
};
```

---

## 7. PDF 요구사항 매핑 체크리스트

| PDF 요구 | 대응 |
|---|---|
| 추상 루트 + 다형적 루프 | `SimObject::update()`, `Factory::tick()`의 단일 for문 |
| 2단계 상속 | `SimObject → Machine → ConcreteMachine` |
| 2종 이상 구체 머신 | `Reactor`, `Separator`, `SourceTank` 등 |
| 2단계 Product | `RawFeed` / `Intermediate` / `FinishedProduct` |
| public 데이터 멤버 없음 | 모든 필드 `private` (Machine은 `protected` 일부) |
| UI/Backend 분리 | `core/` ↔ `ui/`, 다리는 `bridge/` POD struct |
| `virtual void update(int tick) = 0` | `SimObject::update` |
| `virtual std::string getInfo() const = 0` | `SimObject::getInfo` |
| ImGui 위젯 | SliderInt, ProgressBar, Combo, TextColored, Selectable 등 |

---

## 8. 마일스톤 (PDF Timeline에 맞춤)

| 주차 | 기간 | 목표 |
|---|---|---|
| W1 | ~5/9 | 팀/주제 확정, 본 골격 문서 확정 |
| **현재** | 5/16 | core/* 헤더 골격, main.cpp 컴파일되는 빈 Factory |
| W3 | 5/19–23 | **M1**: UML 손그림 + 핵심 클래스 구현 + ImGui 3개 윈도우 |
| W4 | ~5/30 | 시나리오 1종(Normal) 풀 사이클 동작, Inspector/Snapshot 완성 |
| W5 | 6/2–6 | 시나리오 2종 이상, Event Log, Statistics, 최종 제출 |

---

## 9. 즉시 다음 액션 (Phase 1 시작 시)

1. `src/` 하위에 `core/`, `bridge/`, `ui/` 디렉토리 생성, 기존 `Machine.*`은 `legacy/`로 이동.
2. `SimObject.h` + `Machine.h`(추상) + `Connector.h`(추상) + `Product.h` 헤더만 먼저 작성, 비어 있어도 컴파일 통과시키기.
3. `MachineCmd` / `MachineSnap` POD 정의 → 이게 UI와 백엔드의 계약(contract).
4. `Factory`에 더미 `SourceTank` 한 개 + `Pipeline` 한 개 + `Reactor` 한 개를 하드코딩으로 붙여서 tick 루프 굴려보기.
5. ImGui 쪽에서 `MachineSnap` 리스트만 받아 Inspector 윈도우에 표시 — backend 헤더 한 줄도 include하지 않고.

---

## 10. 열린 결정사항 (팀과 논의 필요)

- **구체 도메인**: 정유 / 나프타 크래커 / 폴리머 / 암모니아 — 머신 이름·process time·breakdown율이 이 결정에 묶임.
- **Conveyor vs Pipeline**: 석유화학은 본질적으로 "관(pipe)" 흐름 → Pipeline을 1차로, Conveyor는 폴리머 펠릿 단계에만 한정해도 됨. (PDF는 Conveyor 생략 허용)
- **Product 표현 단위**: 한 "단위"가 1리터인지 1배치인지. 이산 시뮬에 맞게 "1 batch = 1 Product object"로 가는 게 가장 단순.
- **Worker/Technician 클래스 도입 시점**: PDF의 *Random Breakdowns* 시나리오를 어느 주차에 넣을지에 따라 Phase 2/3 결정.
