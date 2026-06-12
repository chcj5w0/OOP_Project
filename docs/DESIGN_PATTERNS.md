# 수업 내용 ↔ 프로젝트 매핑: 현재 상황과 개선 방향

> 교수님 강의 자료(SOLID, Observer, Memento, State 패턴)를 기준으로
> 현재 구현이 무엇을 보여주고 있고, 무엇이 비어 있으며, 어떤 순서로 보강할지 정리한 문서.
> 전체 아키텍처 설명은 [README.md](../README.md) 참조. (작성: 2026-06-12)

---

## 0. 한눈에 보기

| 강의 주제 | 현재 상태 | 판정 | 조치 |
|---|---|---|---|
| SOLID 5원칙 | 코드 전반에 적용되어 있음 | ✅ 양호 | 발표용 사례 정리 (이 문서 §1) |
| Observer 패턴 | **미적용** — 오히려 강의의 "나쁜 예"와 같은 구조가 존재 | 🔴 문제 | 이벤트 시스템 리팩토링 (§2) |
| Memento 패턴 | 미적용 (스냅샷은 있으나 복원 불가) | 🟡 기회 | 저장/복원 기능 추가 (§3) |
| State 패턴 | enum 기반 상태머신 (GoF State 클래스 아님) | 🟡 설명 필요 | 의도적 미적용 — 방어 논리 준비 (§4) |

핵심 결론: **Observer 리팩토링이 최우선**이다. 이미 알려진 설계 결함(core→ui 역방향 의존)의
해법이 강의 자료에 그대로 나와 있어서, "수업 내용을 이해하고 자기 코드의 문제에 적용했다"를
가장 직접적으로 보여줄 수 있는 작업이다.

---

## 1. SOLID — 현재 코드가 이미 보여주는 것

강의 흐름을 따라가면: 절차적 코드는 **제어 흐름과 컴파일 의존성이 같은 방향**이라
상위 정책이 하위 세부사항에 오염된다(fragile/rigid/재사용 불가). OOP의 다형성은
이 의존성 화살표를 **선택적으로 역전**시키는 도구이고, SOLID는 그 활용 원칙이다.

우리 프로젝트에서 이 "화살표 역전"이 실제로 일어나는 곳:

```
제어 흐름:        Factory ──호출──▶ Reactor::update()
컴파일 의존성:    Factory ──▶ SimObject(추상) ◀── Reactor
                            (Factory는 Reactor라는 타입을 모름)
```

### S — 단일 책임 원칙 (SRP)

> 강의: "한 클래스의 책임은 밀접하게 관련되어야 하고, 추가 책임은 추가 클래스로."
> (Memento 강의 Solution I에서 editor가 이력까지 관리하는 것을 SRP 위반으로 지적)

| 우리 코드 | 책임 분리 |
|---|---|
| `Machine` / `MachineSnap` | 공정 실행 ↔ 상태 사본(전달용 데이터) 분리 |
| `StateStyle.h` | "상태가 무슨 색인가"를 뷰들에서 떼어내 한 곳에 |
| `UILayout.h` | "창이 어디 놓이는가"를 각 뷰에서 떼어내 한 곳에 |
| `Factory` vs `RunControlManager` | 공정 조립·진행 ↔ 시간 구동(pause/step/속도) 분리 |

### O — 개방/폐쇄 원칙 (OCP)

> 강의: "새 Tool(Brush) 추가가 mouseDown()/mouseUp() 수정으로 이어지면 안 된다."

우리의 대응: 새 기계를 추가할 때 [Factory.cpp](../src/core/Factory.cpp)의
`tick()` 루프는 한 글자도 바뀌지 않는다 — `for (auto& o : m_objects) o->update(tick);`
새 UI 창을 추가할 때도 main의 `for (UIObject* ui : uiObjects) ui->render();`는 무수정.
**백엔드와 UI 양쪽에 OCP 루프가 대칭으로 존재**하는 것이 이 프로젝트의 특징이다.

### L — 리스코프 치환 원칙 (LSP)

> 강의: "파생 클래스는 기반 클래스의 근본 행동을 바꾸지 않으면서 확장해야 한다."
> (Square/Rectangle 반례, dynamic_cast 사용은 치환 위반의 신호라는 경고)

우리의 대응: `Machine`은 `Connector*`만 알고, 그 자리에 Pipeline을 꽂든 Conveyor를
꽂든 동작한다([Factory.cpp](../src/core/Factory.cpp)의 시나리오 조립에서 실제로 혼용).
Conveyor는 push/pop의 **계약**(가득 차면 false, 비었으면 nullptr)을 깨지 않으면서
운반 지연이라는 행동을 확장했다 — Square처럼 호출자의 기대를 배반하는 게 아니라,
계약 안에서의 확장이라는 점이 LSP 준수의 핵심이다.

⚠️ **정직하게 짚을 부분**: [Factory.cpp](../src/core/Factory.cpp)는
`dynamic_cast<Machine*>`로 단일 컨테이너에서 기계만 골라낸다. 강의에서 dynamic_cast는
"치환 위반의 신호"로 소개되는데, 우리 용례는 *행동 분기*가 아니라 *이종 컨테이너 필터링*이다
(기계와 커넥터는 스냅샷 형식 자체가 달라서 역할별 조회가 필요). 질문이 나오면:
"행동은 전부 `update()` 다형성으로 처리하고, dynamic_cast는 역할별 스냅샷 수집에만 쓴다.
대안은 기계/커넥터 별도 벡터 유지나 Visitor인데, 현재 규모에선 필터링이 가장 단순하다"로 답한다.

### I — 인터페이스 분리 원칙 (ISP)

> 강의: "클라이언트가 쓰지 않는 인터페이스에 의존하게 강제하지 마라" (Worker/RobotWorker FAT 인터페이스)

우리의 대응: `UIObject`는 `render()` 하나만 요구한다. 뷰들이 필요 없는
메서드(예: update, handleInput 같은 것)를 억지로 구현하는 일이 없다.
`RunControlManager`를 UIObject에 **넣지 않은 결정**도 ISP 관점으로 설명 가능 —
시그니처가 다른(Factory&가 필요한) 객체를 같은 인터페이스에 욱여넣으면
인터페이스가 비대해지거나 뷰들이 오염된다.

### D — 의존성 역전 원칙 (DIP)

> 강의: "상위 모듈은 하위 모듈에 의존하지 말고, 둘 다 추상에 의존하라" (LightSwitch/Switchable/LightBulb)

우리의 대응:
- `Machine`(상위 정책: 공정 흐름)은 `Connector`(추상)에 의존, 구체 Pipeline/Conveyor를 모름
- UI는 core 구체 클래스가 아니라 `bridge/`의 POD에 의존
- **위반 사례 1건 잔존**: `Factory.cpp` → `ui/EventLogUI.h` 직접 include.
  상위(core)가 하위 세부사항(특정 UI 위젯)에 의존하는, 강의가 경고하는 바로 그 구조.
  → §2의 Observer 리팩토링이 이걸 해소한다.

---

## 2. Observer 패턴 — 🔴 최우선 개선 (이벤트 시스템)

### 강의 내용 요약

dataSource가 formulaSheet·chart를 **직접 참조해 직접 호출**하면:
1. dataSource 작성 시점에 상대 클래스의 갱신 방법을 알아야 하고,
2. 의존 클래스가 늘 때마다 dataSource를 수정해야 한다(OCP 위반).

해법: `Observer` 인터페이스(`update()`)를 도입하고, Subject는 `vector<observer*>`를
순회하며 다형적으로 통지한다. 통지에 값을 실어 보내면 **push 스타일**,
옵저버가 Subject에서 읽어가면 **pull 스타일**.

### 우리 코드의 현재 상태 — 강의의 "나쁜 예"와 동형

```cpp
// Factory.cpp (현재) — dataSource가 chart를 직접 호출하는 것과 같은 구조
#include "ui/EventLogUI.h"          // core가 ui를 안다!
...
if (m_eventLogUI) {
    m_eventLogUI->addEvent(m_tick, msg);   // 구체 클래스 직접 호출
}
```

| 강의의 나쁜 예 | 우리 코드 |
|---|---|
| `dataSource` | `Factory` |
| `formulaSheet& fS` 멤버 | `EventLogUI* m_eventLogUI` 멤버 |
| `fS.setValue(value)` 직접 호출 | `m_eventLogUI->addEvent(...)` 직접 호출 |
| chart 추가 시 dataSource 수정 | 새 이벤트 소비자 추가 시 Factory 수정 |

README §6에 "core→ui 역방향 의존"으로 기록해둔 한계가 정확히 이것이다.

### 개선 설계 (push 스타일)

이벤트가 (tick, message)로 단순하고, 옵저버가 Factory 내부를 되캐물을 이유가 없으므로
**push 스타일**이 적합하다 (pull은 옵저버가 Subject 참조를 들고 있어야 해서 결합이 더 생긴다).

```
src/bridge/IEventObserver.h   ← 추상 인터페이스 (POD는 아니지만 순수 추상 = 의존성 없음)
   class IEventObserver {
   public:
       virtual ~IEventObserver() = default;
       virtual void onEvent(int tick, const std::string& message) = 0;
   };

src/core/Factory   (Subject 역할)
   - EventLogUI* m_eventLogUI            → std::vector<IEventObserver*> m_observers
   - setEventLogUI(...)                  → addObserver(IEventObserver*)
   - m_eventLogUI->addEvent(t, msg)      → notify(t, msg)  { for (auto* o : m_observers) o->onEvent(t, msg); }
   - #include "ui/EventLogUI.h" 삭제     → #include "bridge/IEventObserver.h"

src/ui/EventLogUI   (ConcreteObserver 역할)
   - class EventLogUI : public UIObject, public IEventObserver
   - onEvent(tick, msg) { addEvent(tick, msg); }

src/main.cpp
   - factory.setEventLogUI(&eventLogUI)  → factory.addObserver(&eventLogUI)
```

bridge에 "행동 있는 클래스를 두면 안 되지 않나?"라는 의문이 나올 수 있는데,
IEventObserver는 **구현이 전혀 없는 순수 추상 인터페이스**라 어느 쪽에도 의존성을
끌고 들어오지 않는다. bridge의 본질(양쪽이 안전하게 공유하는 계약)에 부합한다.
RunControlManager를 bridge로 옮기는 것이 안 됐던 이유(core 의존 행동 클래스)와 대비해서
설명하면 이해가 쉽다.

### 기대 효과

1. core→ui include가 사라져 레이어 규칙이 예외 없이 성립 (DIP 위반 해소)
2. 새 이벤트 소비자(파일 로거, 통계 수집기, 사운드 알림 등)를 Factory 무수정으로 추가 (OCP)
3. "강의에서 배운 패턴을, 강의가 경고한 바로 그 문제에 적용했다"는 발표 서사

검증 방법: 빌드 + Xvfb 실행 스크린샷으로 Event Log 창에 이벤트가 계속 흐르는지 확인.

---

## 3. Memento 패턴 — 🟡 저장/복원 기능 (추가 시 강력)

### 강의 내용 요약 (해법의 진화 과정이 중요)

undo()를 구현하는 세 가지 시도:
- **Solution I**: editor가 직접 prevContent 벡터 유지 → 동작은 하지만 **SRP 위반**
  (편집 책임 + 이력 관리 책임이 한 클래스에)
- **Solution II**: editor 객체 전체를 history에 복사 → **불필요한 상태까지 저장**되어 비대해짐
- **Solution III (정답)**: 복원에 필요한 상태만 담는 **EditorState(Memento)** 를 분리하고,
  **History(Caretaker)** 가 보관. Originator ─(점선 의존)→ Memento ←(컴포지션)─ Caretaker

### 우리 코드의 현재 상태

`MachineSnap`/`ConnectorSnap`이 있어서 "이미 Memento 아닌가?" 싶지만 **다르다**:

| | 우리 Snap | Memento |
|---|---|---|
| 방향 | 읽기 전용, UI 표시용 | 복원 가능, 상태 되돌리기용 |
| 내용 | 표시에 필요한 것 (typeName, blockedReason…) | 복원에 필요한 것만 |
| 복원 경로 | 없음 — Factory에 setState 류 API 없음 | Originator가 memento로 자신을 되돌림 |

즉 스냅샷 인프라는 Memento의 절반(상태 추출)만 한다. 복원이 없다.

### 개선 설계

```
src/core/FactoryMemento.h   (Memento — 강의 Solution II의 교훈대로 "복원에 필요한 것만")
   struct FactoryMemento {            // Factory만 내용을 채우고 꺼냄
       int tick;
       struct MachineState_ { MachineState state; float health; int progress; ... };
       std::vector<MachineState_>            machines;     // id 순
       std::vector<std::vector<std::string>> connectors;   // 슬롯의 제품 이름들
   };

src/core/Factory   (Originator)
   FactoryMemento createMemento() const;
   void restore(const FactoryMemento& m);   // 제품은 이름→make_unique 재생성

src/app or main   (Caretaker — 강의의 History)
   class SimHistory { push(m); pop(); };    // Factory를 모르고 memento만 보관

UI: Run Control에 [Save] [Restore] 버튼 (또는 자동: N틱마다 push → Undo)
```

설계 포인트 두 가지:
- **강의 Solution II의 비판을 그대로 적용**: Factory 전체(UI 포인터, 옵저버 목록 포함)를
  복사하지 않고 시뮬레이션 상태만 담는다. "왜 Factory를 통째로 저장 안 했나"의 답이
  강의 슬라이드에 있다.
- 제품 객체는 포인터라 복사가 안 되므로, **이름으로 직렬화 → 복원 시 재생성**.
  `Product::name()`의 세 번째 쓰임새가 생긴다 (UI 색상, 이벤트 로그에 이어).

데모 시나리오: Save → Force Break로 공정 망가뜨리기 → Restore → 원상 복구.
평가자에게 패턴+기능을 동시에 보여주는 그림이 된다.

### 비용

중간 규모 (새 파일 2개 + Factory에 두 메서드 + UI 버튼). 커넥터 내용물 복원을 위해
Connector에 `clear()`/`pushByName()` 같은 보조가 필요해 코어를 약간 건드린다.
시간이 빠듯하면 1순위(Observer)만 하고 이건 "향후 확장"으로 발표에 언급만 해도 된다.

---

## 4. State 패턴 — 🟡 의도적 미적용 (방어 논리)

### 강의 내용 요약

Canvas가 `enum toolType` + if-else로 도구별 동작을 분기하면, 도구 추가 시
mouseDown/mouseUp을 모두 수정해야 한다. 해법: `Tool` 추상 클래스를 두고
Canvas는 `Tool* currentTool`에 위임 → 분기 제거 + OCP.

### 우리는 왜 enum인가 — 예상 질문과 답

`MachineState`는 enum이고, GoF State처럼 IdleState/WorkingState 클래스를 만들지 않았다.
이는 **의도적 선택**이며 근거는:

1. **분기 제거라는 패턴의 동기가 이미 다른 수단으로 충족됐다.**
   강의 Canvas의 문제는 "도구 종류별 행동 분기"였고, 우리의 "기계 종류별 행동 분기"는
   Machine 상속 + `update()`/`createOutput()` 다형성(Template Method + Factory Method)이
   이미 제거했다. 남은 상태 검사는 한 알고리즘 내부의 진행 단계 판정이다.
2. **상태 전이가 공유 알고리즘 하나에 응집되어 있다.**
   `Machine::advanceProgress()`는 체력 감소→고장 판정→진행→배출→차단이 서로 얽힌
   **하나의 흐름**이다. 4개 상태를 클래스로 쪼개면 이 흐름이 네 파일로 파편화되고,
   상태 객체들이 체력·진행도·커넥터에 모두 접근해야 해서 결합이 오히려 늘어난다.
3. **상태 가짓수가 고정적이고 적다 (4개).** State 패턴의 이득(상태 추가의 OCP)이
   발생할 가능성이 낮다. 기계 *종류*는 늘어나지만(→ 그쪽은 상속으로 열려 있음)
   기계 *상태*는 도메인상 안정적이다.

남아 있는 상태 switch는 [StateStyle.h](../src/ui/StateStyle.h)의 색/라벨 매핑인데,
이는 행동이 아니라 **표현 계층의 조회 테이블**이라 State 패턴의 대상이 아니다.

> 한 줄 요약: "State 패턴의 목적(상태별 행동 분기 제거)은 이해하고 있고, 우리 코드에서
> 그 목적은 상속 다형성이 담당한다. 패턴을 아는 것과 모든 곳에 적용하는 것은 다르다 —
> 적용하지 않을 이유를 아는 것까지가 패턴 이해라고 판단했다."

---

## 5. 실행 계획

| 순위 | 작업 | 규모 | 효과 |
|---|---|---|---|
| 1 | 이벤트 시스템 Observer 리팩토링 (§2) | 소 (파일 3~4개) | DIP 위반 해소 + 강의 패턴 직접 시연 |
| 2 | Memento 저장/복원 + UI 버튼 (§3) | 중 | 기능 추가 + 패턴 시연 + 데모 시나리오 |
| 3 | README §5 표를 SOLID 원칙별로 재정렬 | 소 (문서) | 평가 기준 용어와 정렬 |
| 4 | UML에 Observer/Memento 반영 후 PNG 재추출 | 소 (모델링) | 산출물 일관성 |

State 패턴은 코드 변경 없이 §4의 방어 논리만 발표 준비물로 가져간다.
