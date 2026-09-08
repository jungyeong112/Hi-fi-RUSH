#pragma once
#include "Client_Defines.h"
#include "InteractObject.h"
NS_BEGIN(Engine)
class CMeshRenderer;
NS_END
NS_BEGIN(Client)
class CInteract_Magnetic : public CInteractObject
{
public:
	CInteract_Magnetic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInteract_Magnetic(const CInteract_Magnetic& Prototype);
	virtual ~CInteract_Magnetic() = default;
public:
	virtual HRESULT Initialize_Prototype(void* pArg);
	virtual HRESULT Initialize(void* pDesc);
	
	virtual HRESULT Late_Initialize() override;

	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual void Render_Properties() override;

public:
	_float  Get_Range()const { return m_fRange; }
private:
	_float	m_fRange = 10.f;
	_bool	m_bIsShield = false;
public:
	virtual void	Free() override;

	CGameObject* Clone(void* pArg) override;
	HRESULT Add_InitComponents() override;

private:
	class CUC_Interact* m_pUCInteract = { nullptr };
	_int iUCIndex = 0;

	weak_ptr<CMeshRenderer>m_wpMeshRenderer[2];
	_float3 m_vColor = {};
	_float2	m_vUVScale0 = { 1.f, 1.f };
	_float2	m_vUVScale1 = { 8.f, 8.f };
};

NS_END
