#include "Camera_Free.h"
#include "GameInstance.h"
#include "Bone.h"

CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCamera(pDevice, pContext)
{
}

CCamera_Free::CCamera_Free(const CCamera& Prototype)
    : CCamera(Prototype)
{
}

HRESULT CCamera_Free::Initialize_Prototype(void* pArg)
{
    CHKFAIL(__super::Initialize_Prototype(pArg))
    return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pDesc)
{
    CHKFAIL(__super::Initialize(pDesc))
    __super::m_desc.fNear= 0.1875f;
    __super::m_desc.fFar = 5000.f;
    return S_OK;
}

void CCamera_Free::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);

    fTimeDelta = 0.016f;

    if (m_pGameInstance->IsKeyState(DIK_LBRACKET, EKEYACTIONSTATE::STAY)) m_iMoveScale = max(m_iMoveScale - 1, 1);
    if (m_pGameInstance->IsKeyState(DIK_RBRACKET, EKEYACTIONSTATE::STAY)) m_iMoveScale = min(m_iMoveScale + 1, 3000);
    if (m_pGameInstance->IsKeyState(DIK_L, EKEYACTIONSTATE::ENTER)) m_bCameraLock^=true;
    
    auto spTransformCom = m_wpMainTransformCom.lock();
    if (spTransformCom && !m_bCameraLock) {
        auto _localPos = spTransformCom->Get_LocalState(STATE::POSITION);
        
        
        if (m_pGameInstance->IsKeyState(DIK_W, EKEYACTIONSTATE::STAY))
            _localPos = XMVectorAdd(_localPos, 
                XMVectorScale(spTransformCom->Get_LocalState(STATE::LOOK), +1.f * fTimeDelta * m_desc.fMoveSensor * m_iMoveScale));
        if (m_pGameInstance->IsKeyState(DIK_S, EKEYACTIONSTATE::STAY))
            _localPos = XMVectorAdd(_localPos,
                XMVectorScale(spTransformCom->Get_LocalState(STATE::LOOK), -1.f * fTimeDelta * m_desc.fMoveSensor * m_iMoveScale));
        
        if (m_pGameInstance->IsKeyState(DIK_E, EKEYACTIONSTATE::STAY))
            _localPos = XMVectorAdd(_localPos,
                XMVectorScale(spTransformCom->Get_LocalState(STATE::UP), +1.f * fTimeDelta * m_desc.fMoveSensor * m_iMoveScale));
        if (m_pGameInstance->IsKeyState(DIK_Q, EKEYACTIONSTATE::STAY))
            _localPos = XMVectorAdd(_localPos,
                XMVectorScale(spTransformCom->Get_LocalState(STATE::UP), -1.f * fTimeDelta * m_desc.fMoveSensor * m_iMoveScale));
        
        if (m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::STAY))
            _localPos = XMVectorAdd(_localPos,
                XMVectorScale(spTransformCom->Get_LocalState(STATE::RIGHT), +1.f * fTimeDelta * m_desc.fMoveSensor * m_iMoveScale));
        if (m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::STAY))
            _localPos = XMVectorAdd(_localPos,
                XMVectorScale(spTransformCom->Get_LocalState(STATE::RIGHT), -1.f * fTimeDelta * m_desc.fMoveSensor * m_iMoveScale));
        spTransformCom->Set_LocalState(STATE::POSITION, _localPos);

        if (m_pGameInstance->IsKeyState(DIK_LCONTROL, EKEYACTIONSTATE::STAY)) {
            if (auto dx = m_pGameInstance->GetMouseMoveState(EMOUSEMOVE::X))
                spTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), dx * fTimeDelta * m_desc.fRotSensor);
            if (auto dy = m_pGameInstance->GetMouseMoveState(EMOUSEMOVE::Y))
                spTransformCom->Turn(spTransformCom->Get_LocalState(STATE::RIGHT), dy * fTimeDelta * m_desc.fRotSensor);
        }
    }

    if (m_pMinwhanBone) {
        auto _desc = m_pMinwhanBone->GetDesc();
        if (auto spTransformCom = Get_MainTransform().lock()) {
            spTransformCom->Aim(-XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[0])));
            spTransformCom->Set_LocalState(STATE(3),
                XMLoadFloat4(
                    reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[3])
                )
            );
            spTransformCom->Set_Scale(1.f, 1.f, 1.f);
        }
    }

    Update_PipeLines();
}

void CCamera_Free::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCamera_Free::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

void CCamera_Free::Render_Properties()
{
    __super::Render_Properties();
    ImGui::DragInt("Speed", &m_iMoveScale);
    
}

CGameObject* CCamera_Free::Clone(void* pArg)
{
    return CloneBase<CCamera_Free>(pArg);
}

void CCamera_Free::Free()
{
    __super::Free();
}

HRESULT CCamera_Free::Reset()
{
    return S_OK;
}

HRESULT CCamera_Free::Save(void* _pDesc, _uint& _iSize) const
{
    _iSize = sizeof(CAMERA_EXP_DESC);
    memcpy(_pDesc, &m_desc, _iSize);

    return S_OK;
}

HRESULT CCamera_Free::Load(void* _pDesc)
{
    static_assert(is_trivially_copyable_v<CAMERA_EXP_DESC>);
    memcpy(&m_desc, _pDesc, sizeof(CAMERA_EXP_DESC));

    CHKFAIL(__super::Load(&m_desc));

    return S_OK;
}

HRESULT CCamera_Free::Add_InitComponents()
{
    CHKFAIL(__super::Add_InitComponents())
    return S_OK;
}
