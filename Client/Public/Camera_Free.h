#pragma once

#include "Client_Defines.h"
#include "CameraObject.h"

NS_BEGIN(Client)

class CCamera_Free final : public CCamera
{
	friend class CGameObject;
public:
	typedef struct tagCameraExporterDesc final : public CCamera::CAMERA_DESC
	{
		_float		fZoomSensor		= {5.f};
		_float		fMoveSensor		= {1.f};
		_float		fRotSensor		= {1.f};
		_float		fFineSensor		= {0.1f};
		_float3		fFocusPos		= {0.f,0.f,0.f};
	} CAMERA_EXP_DESC;
private:
	CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Free(const CCamera& Prototype);
	virtual ~CCamera_Free() = default;

public:
	virtual HRESULT Initialize_Prototype(void* pArg);
	virtual HRESULT Initialize(void* pDesc);

	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);

	void Render_Properties() override;
public:
	CGameObject* Clone(void* pArg) override;
	virtual void	Free() override;
	virtual HRESULT	Reset() override;
public:
	HRESULT Save(void* _pDesc, _uint& _iSize) const override;
	HRESULT Load(void* _pDesc)						override;

	HRESULT Add_InitComponents() override;
	CAMERA_EXP_DESC			m_desc;
	_int					m_iMoveScale = 10;
	_bool					m_bCameraLock = false;
};

NS_END