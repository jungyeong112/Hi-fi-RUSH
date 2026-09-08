#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Input_Manager_API.h"
#include "Time_Manager_API.h"

NS_BEGIN(Engine)
class CModel;
class CBone;
class CAnimController;
class CBlackBoard;
NS_END

NS_BEGIN(Client)
class CCharacter abstract : public CGameObject
{
public:
	enum class EAttackType {
		// Player
		NONE,
		NORMAL,
		AIRBORNE,
		AIRATK,
		BEATHIT,
		MULTIHITATK,
		FINISHER,
		AIRBONE_FINISHER,

		// Partner
		PEPPERMINT_BARRIER_BREAK,
		KORSICA_WIND,
		KORSICA_WHIRL_ATK,
		MACARON_SHIELD_BREAK,
		MACARON_HOLD,

		// Enemy
		RP_PARRY,
		RP_DODGE,

		FIRE,
		LASER,
		LAST_LASER,
		KALE_LASER,
		GRAB,
		KNOCKBACK,
		RP_KNOCKBACK,

		// Interact Object
		ELECTRIC,
	};

	struct FDamageInfo {
		CGameObject* pSourceObj = nullptr;
		WPCollidor* wpCollidor;
				
		_float fDamage = 0.f;
		EAttackType eAttackType = EAttackType::NONE;

		_float4 vPosition = { 0.f, 0.f, 0.f, 0.f };
		_float4 vNormal = { 0.f, 0.f, 0.f, 0.f };

		//_bool bIsFire = false;
		//_bool bIsBeatHit = false;
		//_bool bIsAirborne = false;
		//_bool bIsMultiHitATK = false;
	};

public:
	CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCharacter(const CCharacter& Prototype);
	virtual ~CCharacter() = default;
public:
	virtual HRESULT         Initialize_Prototype(void* pArg);
	virtual HRESULT         Initialize(void* pDesc);
	virtual HRESULT         Late_Initialize() override;
					        
	virtual void            Priority_Update(_float fTimeDelta);
	virtual void            Update(_float fTimeDelta);
	virtual void            Late_Update(_float fTimeDelta);
	HRESULT                 Load_Asset() override;
	virtual void            Add_Listener() {};
	virtual void            Add_RhythmTargetListener(_float TargetRatio);
					        
	virtual _bool           OnDamage(FDamageInfo _fDamageInfo) PURE;
	void                    SetUseGravity(_bool bUseGravity) { m_bUseGravity = bUseGravity; }
	void                    SetPos(_vector vPos);
	void					Render_Properties() override;
public:		                
	HRESULT                 Save(void* _pDesc, _uint& _iSize) const override;
	HRESULT                 Load(void* _pDesc) override;
	HRESULT                 Add_InitComponents() override;
			                
	void	                SetBBFromAnimJson(const Json::Value& root, ANIMCTX_DESC& m_ctxDesc);
public:
	virtual void	        Free() override;
	virtual HRESULT	        Reset() override;

	void                    Move_TargetPoint	(_fmatrix vTargetMatrix);
	void                    Set_FaceBone		();
	void                    PlayRandomSound(_wstring _SfxTag, _int iMin, _int iMax);

	virtual void            IsAppear(_bool bIsAppear);

protected:
	weak_ptr<CModel>		m_wpModel;

protected: //Gravity
	_float                  m_fGravity    = {};
	_float                  m_fSpeedY     = { -1.f};
	_float                  m_fTerminalvelocity = { -50.f };

    _bool                   m_bUseGravity = false;
	_bool                   m_bGrounded   = false;
	_bool                   m_bEditorLock = false;

	void                    ApplyGravity(_float fTimeDelta);

protected:
	UniqueTimer				m_upTransitTimer	= nullptr;
	_float4x4				m_vOriginTransform	= IDENTITYMATRIX;
	_float4x4				m_vTargetTransform	= IDENTITYMATRIX;

protected:
	class CBone*			m_pFaceBone			= nullptr;
	_float4					m_vHeadBoneUp		= { 0.f, 0.f, 1.f, 0.f };
	_float4					m_vHeadBoneForward	= { 0.f, 1.f, 0.f, 0.f };
	_float4					m_vHeadBoneRight	= { 1.f, 0.f, 0.f, 0.f };
};

NS_END
