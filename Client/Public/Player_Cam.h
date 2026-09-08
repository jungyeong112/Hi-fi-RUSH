
#pragma once

#include "Client_Defines.h"
#include "CameraObject.h"
#include "Time_Manager_API.h"

NS_BEGIN(Client)

class CPlayer_Cam final : public CCamera
{
	friend class CGameObject;
private:
	CPlayer_Cam(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer_Cam(const CPlayer_Cam& Prototype);
	virtual ~CPlayer_Cam() = default;

public:
	virtual HRESULT Initialize_Prototype(void* pArg); 
	virtual HRESULT Initialize(void* pDesc);
	virtual HRESULT Late_Initialize() override;

	void SetCamPos(float dDeltaTime);
	virtual void Priority_Update(_float fTimeDelta);
	void SetCamAngle(const Engine::_float& fTimeDelta);
	virtual void Update(_float fTimeDelta);
	void SetTargetPos();
	virtual void Late_Update(_float fTimeDelta);
	void SetCamOffset();
	_float SetSpringCam(_fvector vPivotPos, _fvector vLookDir);
public:
	CGameObject* Clone(void* pArg) override;
	virtual void	Free() override;
	virtual HRESULT	Reset() override;
public:

	HRESULT Save(void* _pDesc, _uint& _iSize) const override;
	HRESULT Load(void* _pDesc)						override;

	HRESULT Add_InitComponents() override;
	HRESULT Load_Config();
	void	SetPlayer(class CPlayer* pPlayer) { m_pPlayer = pPlayer; }

public:
	void  StartCamShake     (_float fPower, _float fDuration, _float fFrequence = 35.f);
	void  StartCameraRoll   (_float fRollAngle, _float fDuration, _float fInRatio, _float fOutPower);
	void  StartCameraFovKick(_float fFovOffset, _float fDuration, _float fInRatio, _float Outpower);

	void  SetPitchRoll();
	void  SetPitchRoll(_vector vLook);
private: 
	void  ApplyCamShake (_float fTimeDelta);
	void  ApplyCamRoll  (_float fTimeDelta);
	void  ApplyFovKick  (_float fTimeDelta);
	void  ApplyCameraEffect(_float fTimeDelta);

public:
	void	SetTargetBone(class CBone* pTargetBone, float fTransitDuration = 0.2f, float fTargetFOV = XM_PI / 3.f);
	void SetLerpFOV(float fTransitDuration, float fTargetFOV);
private:
	class CBone*	m_pTargetBone		= nullptr;
	class CBone*	m_pPrevBone			= nullptr;
	_float			m_fTargetFOV		= XM_PI / 3.f;
	_float			m_fPrevTargetFOV	= XM_PI / 3.f;

	UniqueTimer		m_upCamTransitTimer = nullptr;

	WPTransform		m_prevTargetTransform;
	WPTransform		m_targetTransform;
	WPTransform		m_defaultCamTransform;
private:
	class CPlayer*	m_pPlayer			= nullptr;
	
	//Common
	_float			m_fYaw				= 0.f;
	_float			m_fTargetYaw		= 0.f;
	_float			m_fPitch			= 0.f;
	_float			m_fTargetPitch		= 0.f;


	_float			m_fPitchLimit		= 0.f;
	_float4			m_vCameraDir		= { 0.f, 0.f, 1.f, 0.f };

	_float			m_fCameraDamp		= 0.f;
	_float			m_fCameraLegth		= 0.f;
	_float			m_fCameraTargetLegth= 0.f;

	_float			m_fCameraMeleeSensitivity	= 0.f;
	_float			m_fCameraAimSensitivity		= 0.f;
	_float			m_fCameraSensitivity		= 0.f;

	_float4			m_vTargetPivotPos	= { 0.f, 0.f, 0.f, 1.f };
	_float4			m_vCurrentPivotPos	= { 0.f, 0.f, 0.f, 1.f };
	_float4			m_vCameraPos		= { 0.f, 0.f, 0.f, 1.f };

	_bool           m_bIsCamShake       = false;
	_float          m_fShakePower       = 0.f;
	_float	        m_fShakeDuration    = 0.f;
	_float	        m_fShakeElapsed     = 0.f;
	_float	        m_fShakeFrequency   = 35.f;
	_float	        m_fShakePhase       = 0.f;

	// Roll
	_bool	        m_bCameraRoll       = false;
	_bool	        m_bRollReturning    = false;
			        				    
	_float	        m_fRollAngle        = 0.f;
	_float	        m_fRollDuration     = 0.f;
	_float	        m_fRollElapsed      = 0.f;
			        				    
	_float	        m_fRollInRatio      = 0.2f;
	_float	        m_fRollOutPower     = 2.5f;
	_float	        m_fRollCurve        = 0.f;
			                				    
	// FOV	                
	_bool	        m_bFovKick          = false;
	_float	        m_fBaseFov          = XMConvertToRadians(60.f);
	_float	        m_fFovOffset        = 0.f;
	_float	        m_fFovDuration      = 0.f;
	_float	        m_fFovElapsed       = 0.f;
    _float          m_fInRatio          = {};
    _float          m_fOutPower         = {};
	_bool	        m_bFovReturning     = false;
	_float	        m_fFovCurve         = 0.f;
};			        

NS_END