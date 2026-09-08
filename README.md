# 게임 개발

<aside>
**플레이 영상** : https://youtu.be/093V56PwwEg
**개발 인원**   : 7인개발 
**담당 역할**   : 플레이어·애니메이션·카메라 및 연출  
**플랫폼**      : Windows  
**기간**        : 2026.03 ~ 2026. 06 
**담당한 코드만 첨부했습니다** 


</aside>

## 프로젝트 소개

C++과 DirectX 11을 기반으로 원작 **Hi-Fi RUSH**의 리듬 액션 전투와 게임 시스템을 모작한 프로젝트입니다.

플레이어는 음악의 Beat에 맞춰 일반·강공격을 조합하고, 지상·공중 콤보와 파트너 공격을 연계해 전투를 진행합니다. 입력 타이밍에 따라 공격 결과와 연출이 달라지며, Animation Notify를 통해 공격 판정, VFX, SFX와 카메라 효과를 실제 애니메이션 프레임에 동기화했습니다.

게임 콘텐츠뿐 아니라  Additive Animation과 3×3 Aim Offset,  Camera Effect와 Bone Camera, Dear ImGui 기반 콘텐츠 제작 도구도 함께 구현했습니다.

---

## 개발 환경

| 구분 | 사용 기술 |
| --- | --- |
| Language | C++17 |
| Graphics | DirectX 11 |
| UI / Tool | Dear ImGui |
| Model Import | Assimp |
| Data | JsonCpp |
| Audio | FMOD |
| IDE / Build | Visual Studio 2022 / v143 |
| Platform | Windows |

### 외부 라이브러리 활용

#### Dear ImGui

행동 트리 노드 에디터, Animation Notify 편집, 오브젝트 Inspector와 VFX·Particle Parameter 편집 도구를 구현하는 데 사용했습니다.

#### Assimp

외부 모델의 Mesh, Material, Bone, Skin Weight와 Animation 데이터를 추출하고 엔진 전용 데이터로 변환하는 Model Exporter에 활용했습니다.

#### JsonCpp

애니메이션 상태와 전이, 행동 트리 그래프, 게임 오브젝트 및 콘텐츠 설정값을 데이터화하고 저장·불러오는 데 사용했습니다.

#### FMOD

배경 음악과 효과음을 관리하고 Animation Notify 및 리듬 이벤트와 사운드 재생 시점을 동기화하는 데 사용했습니다.


---

# 담당 기능

## 플레이어 전투 시스템

- 일반·강공격 입력 조합을 이용한 지상·공중 콤보
- 입력 순서를 `vector<ECOMBO>`로 관리
- 콤보 입력 유효 시간과 자동 초기화
- Rhythm 판정과 공격 데미지 연동
- Animation Notify 기반 공격 Collider 활성화
- Combo Index 기반 공격 Animation 전환
- 무기 Animation·Trail·VFX·SFX 동기화
- 파트너 호출 및 합동 공격
- 피격·이동·상태 변경에 따른 콤보 취소

## 플레이어 이동 및 바닥 판정

- 중력과 수직 속도를 이용한 점프·낙하 처리
- 지상·공중 상태에 따른 공격 분기
- 하향 Ray Cast 기반 Ground Check
- 낙하 속도를 반영한 Ray 검사 거리 보정
- 가장 가까운 지면 후보 선택
- 지면 높이 Snap과 착지 이벤트 처리
- 상승 및 Air Combo 상태의 Ground 판정 제외

## Layer Animation System

- 우선순위 기반 Animation State Machine
- Locomotion Base Pose 생성
- Reference Pose 기반 Additive Animation
- 3×3 Aim Offset과 방향 Pose 보간
- Root Motion 처리
- Look Pose 및 IK 후처리
- Animation Notify 기반 Gameplay Event


## Camera System

- 플레이어 추적 Camera와 Damping 기반 Pivot 보간
- Ray Cast 기반 Spring Camera
- Shake·Roll·FOV Kick 후처리
- Animation Notify 기반 Camera 전환
- Bone Combined Transform 추적
- 연출 종료 후 기본 플레이 Camera 복귀
- Camera Keyframe Sequence와 Ease Curve


## 콘텐츠 제작 도구

- Animation 목록과 Timeline 확인
- Animation·VFX·SFX Notify 생성·수정·삭제
---

# 기술 설명


## 1. 플레이어 콤보 시스템

일반·강공격과 지상·공중 상태를 `ECOMBO` enum으로 구분하고 입력된 순서를 `vector<ECOMBO>`에 저장했습니다.

```cpp
enum ECOMBO
{
    COMBO_X,
    COMBO_Y,
    COMBO_AIR_X,
    COMBO_AIR_Y,
    COMBO_REST,
    COMBO_DASH,
    BEATHEAT
};

vector<ECOMBO> m_vecCombo;
```

```text
공격 입력
    ↓
지상·공중 상태에 맞는 ECOMBO 추가
    ↓
입력 개수와 Index 검사
    ↓
다음 Combo Animation Index 결정
    ↓
Combo Timer 갱신
```

Vector의 길이와 각 Index를 검사해 `[X, X, X]`, `[X, REST, X]`, `[X, Y, X, X]`와 같은 순차·분기형 콤보를 구분했습니다.

### Vector를 사용한 이유

콤보는 입력된 **순서와 현재 입력 개수**가 중요한 가변 길이 데이터입니다.

- `push_back()`으로 입력을 발생 순서대로 추가
- 연속된 메모리에 작은 입력 이력 저장
- `m_vecCombo[index]`를 이용한 직접 패턴 비교
- 전체 이력 순회와 유효 공격 수 계산
- `COMBO_REST`를 제외한 임시 Vector로 특수 패턴 검사
- 콤보 종료 시 `clear()`로 전체 이력 초기화

Queue는 앞쪽 입력을 소비하는 데는 적합하지만 전체 이력의 임의 Index를 반복 비교하기 어렵고, Map이나 Tree는 작은 순차 입력을 관리하기에는 복잡하기 때문에 `vector`를 선택했습니다.

---

## 2. 콤보 유효 시간과 자동 초기화

과거 입력이 다음 공격까지 남아 의도하지 않은 콤보로 연결되는 것을 방지하기 위해 Combo Timer를 구성했습니다.

```cpp
m_pComboTimer = m_pGameInstance->CreateTimerWithDuration(
    m_fComboStayTime,
    false,
    [=]() { ResetCombo(); },
    false
);
```

유효한 공격 입력이 추가될 때마다 Timer를 다시 시작하고, 설정된 시간 동안 다음 입력이 없으면 Callback에서 콤보를 자동 초기화합니다.

```text
Combo 입력 추가
    ↓
Combo Timer Restart
    ↓
다음 입력 도착
├─ 제한 시간 이내 → Combo 연결 및 Timer Restart
└─ 제한 시간 초과 → ResetCombo()
```

`ResetCombo()`는 Vector뿐 아니라 공격 상태, Blackboard 값과 Combo Index를 함께 초기화합니다.

```cpp
void CPlayer::ResetCombo()
{
    if (m_pComboTimer->IsActive())
        m_pComboTimer->Reset();

    m_vecCombo.clear();

    SetBoolElement("AttackX", false);
    SetBoolElement("AttackY", false);
    SetBoolElement("ComboX", false);
    SetBoolElement("ComboY", false);

    m_bComboAble   = false;
    m_iCombo_X_Idx = 0;
    m_iCombo_Y_Idx = 0;
    m_bRest        = false;
    m_bIsAttack    = false;
}
```

Timer 만료 외에도 이동 입력, 최대 유효 입력 수 도달, 판정 실패와 Animation Notify의 `ComboReset`에서 초기화하도록 구성했습니다.

---

## 3. Ray Cast 기반 Ground Check

플레이어 위치보다 조금 높은 지점에서 아래 방향으로 Ray를 발사하여 실제 지면까지의 거리를 검사했습니다.

```text
Player Position + Ray Start Offset
    ↓
Down Ray Cast
    ↓
지면 후보 거리순 정렬
    ↓
가장 가까운 Hit 선택
    ↓
Ground Gap 검사
    ↓
높이 Snap 및 착지 처리
```

고정된 검사 거리만 사용하면 낙하 속도가 빠를 때 한 프레임 사이에 지면 판정 범위를 통과할 수 있어, 현재 수직 속도와 Delta Time으로 계산한 추가 낙하 거리를 Ray 길이에 포함했습니다.

```cpp
const float extraFallDistance =
    max(0.f, -m_fSpeedY * fTimeDelta);

const float checkDistance =
    rayStartUp
    + footOffset
    + groundSnapDown
    + extraFallDistance;
```

검출된 후보를 거리순으로 정렬하고 가장 가까운 지면의 높이를 선택합니다. 현재 위치와 지면 사이의 간격이 허용 가능한 Snap 거리 안에 있을 때 Player Y 위치를 보정하고 수직 속도를 초기화합니다.

이전 프레임에 공중이었던 경우에만 `OnLanded()`를 호출해 착지 이벤트가 매 프레임 중복 실행되지 않도록 처리했습니다.

---

## 4. Layer 기반 Animation Pipeline

Locomotion으로 Base Skeleton Pose를 생성한 뒤 Aim Offset과 일반 Additive Animation을 동일한 Blend Skeleton에 순차 적용했습니다.

```text
Locomotion Animation
    ↓
Base Skeleton Pose
    ↓
3×3 Aim Offset
    ↓
General Additive Animation
    ↓
IK 및 최종 Bone Matrix
```

기능별 Animation을 독립적으로 관리하여 기본 이동 동작을 유지하면서 조준, 반동과 상체 자세 보정을 추가할 수 있도록 구성했습니다.

---

## 5. Additive Animation

Additive Clip의 Bone Transform을 Reference Pose와 비교하여 본별 Translation과 Quaternion Rotation Delta를 계산했습니다.

```text
Translation Delta
= Additive Translation - Reference Translation

Rotation Delta
= Inverse(Reference Rotation) × Additive Rotation
```

계산된 Delta를 현재 Base Pose에 Weight 기반으로 합성합니다.

```text
Final Translation
= Base Translation + Translation Delta × Weight

Final Rotation
= Slerp(
    Base Rotation,
    Base Rotation × Rotation Delta,
    Weight
)
```

Locomotion 전체를 새로운 Animation으로 교체하지 않고 필요한 변화량만 추가하여 이동을 유지한 상태에서 조준 및 추가 동작을 표현했습니다.

---

## 6. 3×3 Aim Offset

타깃 방향을 정규화된 Look Ratio로 변환하고 수평·수직 축의 Weight를 각각 계산했습니다.

```text
Up-Left       Up       Up-Right

Left         Center       Right

Down-Left    Down     Down-Right
```

두 축의 Weight를 곱해 각 방향 Pose의 최종 가중치를 생성했습니다.

```text
Aim Weight(row, column)
= Vertical Weight(row) × Horizontal Weight(column)
```

각 Aim Pose의 Reference 대비 Delta를 현재 Skeleton에 누적하여 9개의 Pose만으로 연속적인 조준 방향을 표현했습니다.

---

## 7. Animation Notify

Animation의 특정 Frame 또는 재생 구간에 Gameplay Event를 등록했습니다.

- 공격 Collider 활성화
- 콤보 입력 허용과 초기화
- Trail 시작·종료
- VFX·SFX 실행
- Camera Effect
- Bone Camera 전환
- 상태 변경

```text
Animation 재생
    ↓
Notify 구간 진입
    ↓
태그에 등록된 Listener 실행
    ↓
Gameplay System 연동
```

Animation 코드가 Player, VFX와 Camera의 구체 클래스를 직접 참조하지 않고 Notify Tag를 통해 Listener를 실행하도록 구성했습니다.

---


## 8. Camera Effect System

플레이어 추적 Camera의 기본 Transform을 계산한 뒤 Shake, Roll과 FOV Kick을 후처리로 순차 적용했습니다.

```text
Camera Angle
    ↓
Pivot·Position
    ↓
Bone Target Transition
    ↓
Shake → Roll → FOV Kick
    ↓
View·Projection 갱신
```

효과별 수정 Channel을 분리했습니다.

```text
Shake    → Position Offset
Roll     → Right·Up Rotation
FOV Kick → Projection FOV
```

Shake는 서로 다른 주파수의 Sin·Cos 파형과 시간 감쇠를 이용해 Camera Local Right·Up 축에 적용했습니다.

Roll은 Look 축을 기준으로 Right·Up 벡터를 회전하고, FOV Kick은 Base FOV에 Curve가 적용된 Offset을 더하도록 구성했습니다. Roll과 FOV Kick에는 서로 다른 진입·복귀 Curve를 사용했습니다.

연출 시스템이 Camera 제어권을 가진 상태에서도 Camera Effect를 계속 갱신해 플레이 및 연출 Camera 모두에서 효과가 유지되도록 처리했습니다.

---

## 9. Animation 연계 Bone Camera

Animation Notify를 이용해 특정 Action의 Bone Camera 시작과 종료를 요청했습니다.

```text
CamAttachStart Notify
    ↓
Target Bone 설정
    ↓
Bone Combined Transform 추적
    ↓
이전·목표 Transform 보간
    ↓
CamAttachEnd Notify
    ↓
Default Camera 복귀
```

Bone의 Combined Matrix에서 위치와 기저축을 추출해 목표 Camera Transform으로 변환했습니다.

Timer 진행률에 Ease-out Curve를 적용하고 이전 Transform과 목표 Transform을 보간하여 연출 Camera 진입과 플레이 Camera 복귀가 자연스럽게 이어지도록 구성했습니다.

---

# 문제 해결 경험

## 1. Collider 접촉만으로 Ground 상태를 안정적으로 판단하기 어려운 문제

### 문제

Collider 접촉 여부만으로 Ground 상태를 판단하면 경사면, Collider 경계와 빠른 낙하 상황에서 지면 판정이 한 Frame 누락될 수 있었습니다.

Ground 상태가 순간적으로 해제되면 지상 공격이 공중 공격으로 분기되거나 착지 Animation과 Gameplay 상태가 정상적으로 전환되지 않는 문제가 발생했습니다.

### 원인

- Collider의 겹침 여부만으로는 발 아래 지면까지의 거리를 확인하기 어려웠음
- 고정 검사 거리는 빠른 낙하에서 한 Frame 이동 거리를 반영하지 못했음
- 여러 지면 후보 중 실제로 가장 가까운 지면을 선택해야 했음
- Ground 상태와 Player의 실제 Y 위치가 일치하지 않을 수 있었음

### 해결

- Player 위치보다 조금 높은 지점에서 아래 방향으로 Ray Cast를 수행했습니다.
- 현재 수직 속도와 Delta Time으로 한 Frame의 추가 낙하 거리를 계산했습니다.
- Foot Offset, Ground Snap 거리와 추가 낙하 거리를 Ray 길이에 반영했습니다.
- 검출 후보를 거리순으로 정렬하고 가장 가까운 지면을 선택했습니다.
- Ground Gap이 허용 범위 안에 있을 때 Player Y 위치를 지면에 Snap했습니다.
- 착지 시 수직 속도를 0으로 초기화했습니다.
- 이전 Frame이 공중 상태였던 경우에만 `OnLanded()`를 실행했습니다.
- 상승 중이거나 Air Combo 상태에서는 Ground 판정을 조기 종료했습니다.

```text
Player Position
    ↓
낙하 속도를 반영한 Down Ray Cast
    ↓
가장 가까운 지면 선택
    ↓
Ground Gap 검사
    ↓
Position Snap / Grounded / OnLanded
```

### 결과

빠르게 낙하하거나 작은 단차를 통과하는 상황에서도 발 아래 실제 지면 거리를 기준으로 Ground 상태를 판단할 수 있게 됐습니다.

지상·공중 상태의 반복 전환과 착지 누락을 줄이고 지면 높이에 Player 위치를 보정하여 공격 분기와 착지 처리를 안정화했습니다.

---

## 2. 과거 공격 입력이 남아 의도하지 않은 콤보로 연결되는 문제

### 문제

콤보 입력을 계속 저장하면 공격이 끝난 뒤에도 이전 입력 이력이 남아 새로운 공격이 과거 입력과 연결될 수 있었습니다.

반대로 공격이 끝나는 즉시 입력을 지우면 사용자가 다음 버튼을 조금 늦게 눌렀을 때 콤보가 쉽게 끊어져 조작감이 답답해졌습니다.

### 원인

- 콤보 입력을 유지할 명확한 유효 시간이 필요했음
- Animation 종료와 입력 초기화 시점이 항상 일치하지 않았음
- 이동·피격 등 다른 상태로 전환한 뒤에도 입력 이력이 남을 수 있었음
- 입력 수가 계속 증가하면 잘못된 Index 검사와 분기가 발생할 수 있었음

### 해결

- 입력 순서와 개수를 직접 비교할 수 있도록 `vector<ECOMBO>`를 사용했습니다.
- 유효한 공격 입력을 `push_back()`한 뒤 Combo Timer를 다시 시작했습니다.
- 제한 시간 동안 다음 입력이 없으면 Timer Callback에서 `ResetCombo()`를 호출했습니다.
- 이동 입력이 발생하면 기존 공격 이력을 즉시 초기화했습니다.
- 유효 공격 입력이 5개 이상 누적되면 최대 콤보로 판단하고 초기화했습니다.
- 판정 실패 및 상태 변경 시 콤보를 초기화했습니다.
- Animation Notify의 `ComboReset`으로 실제 Animation 종료 시점과 초기화를 동기화했습니다.
- `ResetCombo()`에서 Vector, Timer, Combo Index와 공격 상태를 함께 정리했습니다.

```text
입력 발생
    ↓
vector push_back()
    ↓
Combo Timer Restart
    ↓
다음 입력
├─ 제한 시간 이내 → Combo 연결
└─ 제한 시간 초과 → Vector와 공격 상태 초기화
```

### 결과

허용 시간 안에서는 다음 공격을 자연스럽게 연결하고, 콤보가 종료되면 과거 입력 이력이 자동으로 제거되도록 구성했습니다.

이전 입력으로 잘못된 공격이 실행되는 문제를 방지하면서도 사용자가 다음 입력을 넣을 수 있는 여유 시간을 제공해 콤보 조작감을 개선했습니다.

---

## 3. 여러 Camera Effect가 서로의 결과를 덮어쓰는 문제

### 문제

Shake, Roll과 FOV Kick이 Camera 상태를 각각 직접 제어하면 마지막에 실행된 효과가 이전 효과의 결과를 덮어쓸 수 있었습니다.

연출 Camera가 활성화된 상태에서 일반 추적 Camera의 Update를 중지하면 공격·피격 효과도 함께 멈출 가능성이 있었습니다.

### 해결

- 일반 Camera의 위치와 시선을 먼저 계산했습니다.
- Shake, Roll과 FOV Kick을 후처리 단계에서 정해진 순서로 적용했습니다.
- Shake는 Position, Roll은 Right·Up Axis, FOV Kick은 Projection FOV에 반영했습니다.
- 각 효과에 독립적인 지속 시간과 진입·복귀 Curve를 적용했습니다.
- Camera Handler가 제어권을 가진 상태에서도 Camera Effect는 계속 갱신했습니다.
- 모든 효과 적용 후 View·Projection 정보를 Pipeline에 전달했습니다.

### 결과

세 효과가 서로 다른 Camera Channel에서 동시에 동작하도록 구성했습니다.

일반 플레이 Camera뿐 아니라 Bone 추적 및 연출 Camera에서도 전투 효과가 유지되도록 처리했습니다.

---

## 4. Bone Camera 전환 시 화면이 순간적으로 튀는 문제

### 문제

특정 Action에서 Camera Transform을 캐릭터 Bone으로 즉시 변경하면 기존 플레이 Camera와 위치·회전 차이가 커 화면이 순간적으로 튀었습니다.

연출 종료 후 기본 Camera로 즉시 돌아갈 때도 동일한 불연속이 발생했습니다.

### 해결

- Animation Notify를 Camera 전환의 시작·종료 Trigger로 사용했습니다.
- 전환 시 이전 Camera Transform과 FOV를 저장했습니다.
- Target Bone의 Combined Matrix를 Camera 좌표계로 변환했습니다.
- Timer 진행률에 Ease-out Curve를 적용했습니다.
- 이전 Transform과 목표 Transform을 보간했습니다.
- Target Bone을 `nullptr`로 설정하면 Default Camera를 목표로 사용하도록 구성했습니다.

### 결과

Camera 전환 시점을 실제 Animation 구간과 동기화하고 Bone Camera와 플레이 Camera 사이를 자연스럽게 연결했습니다.

특정 Action에서는 Bone 움직임을 따라가며 동작을 강조하고 연출 종료 후 기존 플레이 시점으로 부드럽게 복귀하도록 구성했습니다.

---

# 향후 개선 방향

## 콤보 시스템 데이터화

현재 콤보 패턴은 Vector의 길이와 Index를 직접 비교해 분기합니다. 패턴 수가 증가하면 조건문도 길어질 수 있으므로 다음 구조로 개선하고 싶습니다.

- Trie 또는 Combo Graph 기반 입력 Pattern
- JSON 기반 Combo Table
- 입력 허용 시간 데이터화
- Animation·Damage·Effect 정보 분리
- Ground·Air 조건을 Node 속성으로 관리
- Combo Editor

```text
현재 Combo Node + 새 입력
    ↓
다음 Combo Node 검색
    ↓
Animation·Damage·Effect 실행
```

새로운 콤보를 추가할 때 Player 코드의 조건문을 직접 수정하지 않고 데이터 편집만으로 확장할 수 있도록 개선하고 싶습니다.

---

## Ground Check 개선

현재 Ground Check는 단일 하향 Ray를 사용합니다. 캐릭터의 발 폭과 복잡한 경사를 더 안정적으로 반영하기 위해 다음 기능을 추가하고 싶습니다.

- Sphere Cast 또는 Capsule Cast
- 지면 Normal 기반 경사 제한
- 여러 지점 Ray Cast
- Step Height 판정
- 이동 가능한 경사와 벽 구분
- Ground Collider Type 정리

---

## Camera Effect 확장

현재 Shake, Roll과 FOV Kick은 효과 종류별로 하나의 상태를 관리합니다. 동일 종류의 여러 효과를 조합하기 위해 다음과 같이 확장하고 싶습니다.

- 공통 Camera Effect 인터페이스
- 효과별 Duration·Curve·Priority
- 동일 종류 효과의 중첩 정책
- Effect Instance Pool
- Data Asset 기반 Parameter
- Debug Timeline

---

# 프로젝트를 통해 배운 점

- 입력 순서를 Vector로 관리하면서 자료구조는 단순한 저장 수단이 아니라 실제 게임 규칙과 조작감을 결정하는 요소임을 경험했습니다.
- Combo Timer, Animation Notify와 상태 전환을 함께 관리하며 입력 허용 시간과 초기화 시점이 액션 게임의 조작감에 큰 영향을 준다는 점을 배웠습니다.
- Collider 접촉 판정만으로 부족한 부분을 Ray Cast와 수직 속도 기반 검사 거리로 보완하며 캐릭터 이동과 충돌 판정의 안정성을 개선했습니다.
- Layer 기반 Animation Pipeline을 구현하며 Base Pose와 추가 동작의 역할을 분리하고 Animation 조합을 재사용하는 방법을 경험했습니다.
- Reference Pose 기반 Additive Animation과 Aim Offset을 구현하며 Quaternion Delta와 Weight 기반 Pose 합성을 학습했습니다.
- Animation Notify를 Collider, VFX, SFX, Camera와 연결하면서 화면에 보이는 Animation과 실제 Gameplay Event의 동기화가 중요하다는 점을 경험했습니다.
- Camera Effect와 Bone Camera를 구현하며 기본 카메라 이동, 연출 효과 및 카메라 제어권을 분리하는 방법을 경험했습니다.
- 플레이어 기능을 담당하며 애니메이션·충돌·UI·사운드 등 여러 시스템을 연결하는 과정에서, 함수 호출 시점과 기능의 사용 방법을 명확히 문서화하는 것이 협업에 중요하다는 점을 배웠습니다. 또한 유지보수와 이후 작업자를 고려해 의도가 쉽게 드러나는 코드와 인터페이스를 작성하는 습관의 중요성을 느꼈습니다.

---
