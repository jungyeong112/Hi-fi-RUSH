#include "Character.h"
#include "BaseModel.h"
#include "GameInstance.h"
#include "BlackBoard.h"
#include "Bone.h"
#include "MeshRenderer.h"
#include <CutScene.h>

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CCharacter::Initialize_Prototype(void* pArg)
{
	CHKFAIL(__super::Initialize_Prototype(pArg))

	return S_OK;
}

HRESULT CCharacter::Initialize(void* pDesc)
{
    m_upTransitTimer = m_pGameInstance->CreateTimerWithDuration(0.2f, false, nullptr, false);
	CHKFAIL(__super::Initialize(pDesc)) 
    return S_OK;
}

HRESULT CCharacter::Late_Initialize()
{
    CHKFAIL(__super::Late_Initialize())

    auto wpComp = Get_Component<CModel>();
    m_wpModel = ConvertWPComponent<CModel>(wpComp);
    m_pFaceBone = m_wpModel.lock()->FindBoneWithName(L"face");

    CRenderer::SHADERPARAM_DESC _desc;
    _desc.eParamType    = CRenderer::EPARAMTYPE::PT_RAWDATA;
    _desc.iSize         = sizeof(_float4);
    for (_int i = 0; i < (Engine::_int)m_wpModel.lock()->Get_NumMeshes(); i++)
    {
        _desc.sConstantName = "g_vHeadBoneUp";
        _desc.pData         = &m_vHeadBoneUp;
        m_wpModel.lock()->GetMeshRenderer(i)->Add_Parameters(_desc);
        _desc.sConstantName = "g_vHeadBoneForward";
        _desc.pData         = &m_vHeadBoneForward;
        m_wpModel.lock()->GetMeshRenderer(i)->Add_Parameters(_desc);
        _desc.sConstantName = "g_vHeadBoneRight";
        _desc.pData         = &m_vHeadBoneRight;
        m_wpModel.lock()->GetMeshRenderer(i)->Add_Parameters(_desc);
    }
    
    return S_OK;
}

void CCharacter::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
    //IsAppear(!CCutScene::Get_IsCutScene());
}

void CCharacter::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_bEditorLock && m_pGameInstance->GetCurrentLevelTag() == L"Level_Editor") m_wpMainTransformCom.lock()->Set_LocalState(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));

    Set_FaceBone();
    if (m_upTransitTimer && m_upTransitTimer->IsActive()) {
        _float fRatio = (Engine::_float)m_upTransitTimer->GetElapsedRatio();

        auto _matSrc = XMLoadFloat4x4(&m_vOriginTransform);
        auto _matDst = XMLoadFloat4x4(&m_vTargetTransform);

        _vector _vSrcScale, _vSrcRotation, _vSrcTranslation;
        _vector _vDstScale, _vDstRotation, _vDstTranslation;
        XMMatrixDecompose(&_vSrcScale, &_vSrcRotation, &_vSrcTranslation, _matSrc);
        XMMatrixDecompose(&_vDstScale, &_vDstRotation, &_vDstTranslation, _matDst);

        _float3 _vScale = Get_MainTransform().lock()->Get_Scaled();
        
        _vector vScale = XMVectorSet(_vScale.x, _vScale.y, _vScale.z, 0.f);
        _vector vRotation = XMQuaternionSlerp(
            _vSrcRotation,
            _vDstRotation,
            fRatio
        );
        _vector vTranslation = XMVectorLerp(
            _vSrcTranslation,
            _vDstTranslation,
            fRatio
        );

        //Get_MainTransform().lock()->Copy_Transform();
        auto _localMatrix = (XMMatrixScalingFromVector(vScale) * XMMatrixRotationQuaternion(vRotation) * XMMatrixTranslationFromVector(vTranslation));
        Get_MainTransform().lock()->Copy_LocalMatrix(_localMatrix);
    }
}

void CCharacter::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CCharacter::Load_Asset()
{
	CHKFAIL(__super::Load_Asset())

		auto wpComp = Get_Component<CModel>();
	m_wpModel = ConvertWPComponent<CModel>(wpComp);

	return S_OK;
}

void CCharacter::Add_RhythmTargetListener(_float TargetRatio)
{
    auto pModel = m_wpModel.lock();
    pModel->AddListener(L"RhythmTarget", [=](float) {pModel->AnimSet_RhythmTarget(TargetRatio); }, nullptr, nullptr);
}


HRESULT CCharacter::Add_InitComponents()
{
	CHKFAIL(__super::Add_InitComponents())

	Add_Component(L"Shader_Mesh_Component", L"Com_MeshShader_Main");
	Add_Component(L"Shader_AnimMesh_Component", L"Com_AnimMeshShader_Main");
	Get_Component(L"Com_MeshShader_Main").lock()->SetInitialAdd();
	Get_Component(L"Com_AnimMeshShader_Main").lock()->SetInitialAdd();
	return S_OK;
}

void CCharacter::SetBBFromAnimJson(const Json::Value& root, ANIMCTX_DESC& m_ctxDesc)
{
    if (!m_ctxDesc.pBlackboard)
        m_ctxDesc.pBlackboard = CBlackBoard::Create();

    if (!root.isObject())
        return;

    if (!root.isMember("AnimTransitions") || !root["AnimTransitions"].isArray())
        return;

    std::unordered_set<std::string> setCtx;

    const Json::Value& transitions = root["AnimTransitions"];
    for (Json::ArrayIndex i = 0; i < transitions.size(); ++i)
    {
        const Json::Value& trans = transitions[i];
        if (!trans.isObject())
            continue;

        if (!trans.isMember("Conditions") || !trans["Conditions"].isArray())
            continue;

        const Json::Value& conditions = trans["Conditions"];
        for (Json::ArrayIndex j = 0; j < conditions.size(); ++j)
        {
            const Json::Value& cond = conditions[j];
            if (!cond.isObject())
                continue;

            if (!cond.isMember("CTX"))
                continue;

            std::string ctx = cond["CTX"].asString();
            if (ctx.empty())
                continue;

            // 중복 제거
            if (!setCtx.insert(ctx).second)
                continue;

            // Value 타입 기준으로 기본값 세팅
            if (!cond.isMember("Value"))
            {
                bool bDefault = false;
                m_ctxDesc.pBlackboard->AddElement(ctx.c_str(), bDefault);
                continue;
            }

            const Json::Value& value = cond["Value"];

            if (value.isBool())
            {
                bool bValue = value.asBool();
                m_ctxDesc.pBlackboard->AddElement(ctx.c_str(), bValue);
            }
            else if (value.isInt())
            {
                int iValue = value.asInt();
                m_ctxDesc.pBlackboard->AddElement(ctx.c_str(), iValue);
            }
            else if (value.isDouble())
            {
                float fValue = value.asFloat();
                m_ctxDesc.pBlackboard->AddElement(ctx.c_str(), fValue);
            }
            else
            {
                bool bDefault = false;
                m_ctxDesc.pBlackboard->AddElement(ctx.c_str(), bDefault);
            }
        }
    }
}

void CCharacter::IsAppear(bool bIsAppear)
{
    m_wpModel.lock()->RenderActive(bIsAppear);
}

void CCharacter::SetPos(_vector vPos)
{
    Get_MainTransform().lock()->Set_LocalState(STATE::POSITION, vPos);
}

void CCharacter::Render_Properties()
{
    __super::Render_Properties();
    ImGui::Checkbox("LockPos", &m_bEditorLock);
}

HRESULT CCharacter::Save(void* _pDesc, _uint& _iSize) const
{
	_iSize = 0;
	return S_OK;
}

HRESULT CCharacter::Load(void* _pDesc)
{
	return S_OK;
}

void CCharacter::Free()
{
	__super::Free();
}

HRESULT CCharacter::Reset()
{
	return S_OK;
}

void CCharacter::Move_TargetPoint(_fmatrix vTargetMatrix)
{
    auto _bpm = m_pGameInstance->GetBPM();
    auto _offset = _bpm / 96;
    m_upTransitTimer->SetDuration(15.f/24.f / _offset);
    m_upTransitTimer->Restart();
    XMStoreFloat4x4(
        &m_vOriginTransform, 
        Get_MainTransform().lock()->GetWorldMatrix());
    XMStoreFloat4x4(
        &m_vTargetTransform,
        vTargetMatrix);

}

void CCharacter::Set_FaceBone()
{
    if (m_pFaceBone) {
        auto _desc = m_pFaceBone->GetDesc();
        XMStoreFloat4(
            &m_vHeadBoneForward,
            XMVector3Normalize(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(_desc.m_CombinedTransformationMatrix.m[0])))
        );
        XMStoreFloat4(
            &m_vHeadBoneUp,
            XMVector3Normalize(-XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(_desc.m_CombinedTransformationMatrix.m[2])))
        );
        XMStoreFloat4(
            &m_vHeadBoneRight,
            XMVector3Normalize(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(_desc.m_CombinedTransformationMatrix.m[1])))
        );

        //Debug
        /*if (true) {
            DEBUGRAY_DESC   _rayDesc;
            RAY             _ray;
            _rayDesc.bOverlay = true;
            _rayDesc.bNormalize = false;
            memcpy(&_ray.vRayOrigin, _desc.m_CombinedTransformationMatrix.m[3], sizeof _float4);

            _rayDesc.eColor[0] = Colors::Black;
            _ray.vRayDir = m_vHeadBoneForward;
            _ray.vRayDir.x *= 0.08f;
            _ray.vRayDir.y *= 0.08f;
            _ray.vRayDir.z *= 0.08f;
            _rayDesc.ray = _ray;
            m_pGameInstance->AddDebugRenderCall(&_rayDesc);

            _rayDesc.eColor[0] = Colors::GreenYellow;
            _ray.vRayDir = m_vHeadBoneUp;
            _ray.vRayDir.x *= 0.08f;
            _ray.vRayDir.y *= 0.08f;
            _ray.vRayDir.z *= 0.08f;
            _rayDesc.ray = _ray;
            m_pGameInstance->AddDebugRenderCall(&_rayDesc);
        }*/
    }
}

void CCharacter::PlayRandomSound(_wstring _SfxTag, _int iMin, _int iMax)
{
    _int Sound = Random(iMin, iMax);
    wstring soundName = _SfxTag + to_wstring(Sound);
    m_pGameInstance->Play_SFX(soundName.c_str());
}


void CCharacter::ApplyGravity(_float fTimeDelta)
{
    auto pTransform = m_wpMainTransformCom.lock();
    if (!pTransform || !m_bUseGravity)
        return;


    if (!m_bGrounded)
    {
        _vector vPos = pTransform->Get_LocalState(STATE::POSITION);
        _float fPosY = XMVectorGetY(vPos);

        _float fCurGravity =  m_fGravity;
        _float fNextPosY = fPosY + m_fSpeedY * fTimeDelta + 0.5f * fCurGravity * fTimeDelta * fTimeDelta;
        //PrintDebug(fNextPosY);

        m_fSpeedY += fCurGravity * fTimeDelta;
        m_fSpeedY = max(m_fSpeedY, m_fTerminalvelocity);
        
        vPos = XMVectorSetY(vPos, fNextPosY);
        pTransform->Set_LocalState(STATE::POSITION, vPos);
    }
    else {
        m_fSpeedY = max(m_fSpeedY, 0.f);
    }

}
