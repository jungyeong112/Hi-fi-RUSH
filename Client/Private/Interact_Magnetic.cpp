#include "Interact_Magnetic.h"
#include "BaseModel.h"
#include "AnimController.h"
#include "GameInstance.h"
#include "UC_Interact.h"
#include "MeshRenderer.h"

CGameObject* CInteract_Magnetic::Clone(void* pArg)
{
    return CloneBase<CInteract_Magnetic>(pArg);
}

HRESULT CInteract_Magnetic::Add_InitComponents()
{
    CHKFAIL(__super::Add_InitComponents());
    return S_OK;
}

CInteract_Magnetic::CInteract_Magnetic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CInteractObject(pDevice, pContext)
{
}

CInteract_Magnetic::CInteract_Magnetic(const CInteract_Magnetic& Prototype)
    : CInteractObject(Prototype)
{
}

HRESULT CInteract_Magnetic::Initialize_Prototype(void* pArg)
{
    CHKFAIL(__super::Initialize_Prototype(pArg))
    return S_OK;
}

HRESULT CInteract_Magnetic::Initialize(void* pDesc)
{
    CHKFAIL(__super::Initialize(pDesc))

    //Model
    CModel::MODEL_DESC _modelDesc;
    lstrcpy(_modelDesc.m_sModelAssetName, L"VerticalMagnet");
    _modelDesc.m_bIsCopy = false;
    Add_Component<CModel>(L"Com_Model_Main", &_modelDesc);

    auto wpComp = Get_Component<CModel>();
    m_wpModel = ConvertWPComponent<CModel>(wpComp);

    m_vColor = { 0.f,0.8f,0.f };
   
    return S_OK;
}

HRESULT CInteract_Magnetic::Late_Initialize()
{
    CHKFAIL(__super::Late_Initialize())

    m_desc.m_fDeactiveTime = 0.2f;
    m_upDeactiveTimer->SetDuration(m_desc.m_fDeactiveTime);

    Set_Animator(L"../AnimNodes/Gimmic/Animator_Gimmic_VerticalMagnet.json");
    m_eInteractType = IT_CHAI_MAGNET;
    m_bIsShield = m_desc.m_iAdditionalType;

    if (m_eCurState == ESTATE::ACTIVE && m_bIsShield)
        Deactivate();
    if (m_eCurState == ESTATE::DEACTIVE && !m_bIsShield)
        Activate();


    m_wpMeshRenderer[0] = m_wpModel.lock()->GetMeshRenderer(0);
    m_wpMeshRenderer[1] = m_wpModel.lock()->GetMeshRenderer(3);
    CRenderer::SHADERPARAM_DESC desc = {};
    desc.eParamType = CRenderer::EPARAMTYPE::PT_RAWDATA;
    desc.iSize = sizeof(_float3);
    desc.sConstantName = "g_vEmissiveColor";
    desc.pData = &m_vColor;
    m_wpMeshRenderer[0].lock()->Add_Parameters(desc);
    m_wpMeshRenderer[1].lock()->Add_Parameters(desc);

    desc.eParamType = CRenderer::EPARAMTYPE::PT_RAWDATA;
    desc.iSize = sizeof(_float2);
    desc.sConstantName = "g_vBarrierUVScale";
    desc.pData = &m_vUVScale0;
    m_wpModel.lock()->GetMeshRenderer(6)->Add_Parameters(desc);
    desc.pData = &m_vUVScale1;
    m_wpModel.lock()->GetMeshRenderer(7)->Add_Parameters(desc);


    return S_OK;
}

void CInteract_Magnetic::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CInteract_Magnetic::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CInteract_Magnetic::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

void CInteract_Magnetic::Render_Properties()
{
    __super::Render_Properties();
    if (ImGui::Checkbox("Is Shield", &m_bIsShield)) {
        m_desc.m_iAdditionalType = m_bIsShield;
    }
}

void CInteract_Magnetic::Free()
{
    __super::Free();
}

