#pragma once
#include "Character.h"
#include "Time_Manager_API.h"
#include "Input_Manager_API.h"
#include "Rhythm_Manager_API.h"
#include "QTE_Types.h"

NS_BEGIN(Engine)
class CEffectRoot;
class CCameraHandler;
NS_END


NS_BEGIN(Client)

class CPlayer final : public CCharacter
{
public:
	struct Transit_DATA
	{
		_bool* pIsDead               = nullptr;
		_bool* pRequestDash          = nullptr;
		_bool* pGrounded             = nullptr;
		_bool* pSuccessParry         = nullptr;
		_bool* pInputMove            = nullptr;
		_bool* pIsJump               = nullptr;
		_bool* pIsDash               = nullptr;
		_bool* pIsMagnet             = nullptr;
		_bool* pIsCombo              = nullptr;
		_bool* pIsAirCombo           = nullptr;
		_bool* pIsAttack             = nullptr;


		_uint* pCurState             = nullptr;
		CBlackBoard* pBB             = nullptr;

		CPlayer* pOwner              = nullptr;
		CGameInstance* pGameInstance = nullptr;
	};



public:
	enum  PLAYERSTATE
	{
		IDLE, WALK, RUN, JUMP, DASH, MAGNET, COMBO, PS_END
	};
	enum VFX
	{
		VFX_IDLE,				  
		VFX_REST,					
		VFX_JUMP,
		VFX_JUMPFONT,           
		VFX_DOUBLEJUMP,      
		VFX_RHYTHMDASHSWING,      
		VFX_RHYTHMDASHIMPACT,     
		VFX_LAND,
		VFX_ATTACK_X,
		VFX_ATTACK_XX,
		VFX_ATTACK_XXX,
		VFX_ATTACK_XXXX,

		VFX_ATTACK_X_X,				
		VFX_ATTACK_X_XX,			

		VFX_ATTACK_XX_X,			
		VFX_ATTACK_XX_XX,		
		VFX_ATTACK_XX_XXX,			
		VFX_BEATHIT_XX_XXX,			

		VFX_BEATHIT_XXXX,

		VFX_ATTACK_AIR_X,			
		VFX_ATTACK_AIR_XX,			
		VFX_ATTACK_AIR_XXX,			
		VFX_ATTACK_AIR_XXXX,		
		VFX_BEATHIT_AIR_XXXX,		
		VFX_ATTACK_Y,
		VFX_ATTACK_YY,
		VFX_ATTACK_YYY,
		VFX_BEATHIT_YYY,
		VFX_ATTACK_AIR_Y,			
		VFX_MAGNET,              
		VFX_MAGNET_TRAIL,
		VFX_TRAIL_Y,
		VFX_TRAIL_O,
		VFX_TRAIL_G,
		VFX_TRAIL_B,
		VFX_TRAIL_R,

		VFX_HIT,
		VFX_PARRY,
		VFX_LASER,

		VFX_DANCE_A,
		VFX_DANCE_LB,
		VFX_DANCE_RB,
		VFX_DANCE_FAIL,
		VFX_DANCE_SUCCESS,

		VFX_RAILSPEEDLINE,
		VFX_BATTLE_START,
		VFX_BATTLE_START_GUITAR,
		VFX_SP_808,
		VFX_HIBIKI,
		VFX_HIBIKI_END,
		VFX_HIBIKI_GUITAR,

		VFX_BURN_01,
		VFX_BURN_01_1,
		VFX_BURN_01_2,
		VFX_BURN_01_3,

		VFX_BURN_02,
		VFX_BURN_02_1,
		VFX_BURN_02_2,
		VFX_BURN_02_3,

		VFX_ATTACK_XY,
		VFX_ATTACK_XYmY,
		VFX_ATTACK_XYX,
		VFX_Player_Attack_XYXX_Entry,
		VFX_Player_Attack_XYXX_Mid,
		VFX_ATTACK_XYY,

		VFX_Guitar_Attack_XYY,
		VFX_Guitar_Attack_XYY_1,
		VFX_Guitar_Attack_XYY_2,
		VFX_Guitar_Attack_XYY_3,
		VFX_Guitar_Attack_XYY_4,
		VFX_Guitar_Attack_XYY_5,
		VFX_Guitar_Attack_XYY_6,

		VFX_Player_Attack_XYXX_End,
		VFX_Player_HalftoneStep,
		VFX_Player_BeatHit_XYXX,
		VFX_Player_BeatHit_XYY,
		VFX_Player_BeatHit_XYmY,
		VFX_Player_BeatHit_XYXX_KRASH,

		VFX_RailSuccess,
		//PartnerJam - common
		VFX_PLAYER_ENEMYHIT,
		VFX_PARTNER_ENEMYHIT,

		VFX_MULTI_ENEMYHIT0,
		VFX_MULTI_ENEMYHIT1,
		VFX_MULTI_ENEMYHIT2,

		VFX_MULTI_PEPPHIT0,
		VFX_MULTI_PEPPHIT1,
		VFX_MULTI_PEPPHIT2,

		VFX_PARTNER_JUMPIMPACT,
		VFX_PLAYER_DJ,
		VFX_MACARON_LANDINGSMOKE,
		//PartnerJam - double base drop

		VFX_PLAYER_SHOOT,

		VFX_MACARON_TURN,
		VFX_MACARON_WHIRL,
		VFX_MACARON_FONT,
		VFX_MACARON_WAVE,


		VFX_PEPP_SWING,
		VFX_PEPP_YEAH,
		VFX_PEPP_TURN,

		//VFX_CP_DOUBLEBASEDROP_JUMP,

		VFX_KORSICA_SNAP,
		VFX_KORSICA_SWING,
		VFX_KORSICA_WIND0,
		VFX_KORSICA_WIND1,

		VFX_SLAM,
		VFX_BEAM,
		VFX_GAINTORNADO,
		VFX_GAINTORNADO_SWING,

		VFX_END
	};
	enum ECOMBO
	{
		COMBO_X, COMBO_Y, COMBO_AIR_X, COMBO_AIR_Y, COMBO_REST, COMBO_DASH, BEATHEAT
	};
	enum EBEATHIT 
	{
	   BH_XXXX, BH_XXXX_2,BH_YYY, BH_TUNEUP, BH_AIR_XXXX , BH_ECOSPLASH, BH_BREAKDOWN, BH_SHRED, SP_808, SP_HIBIKI_SWING, SP_HIBIKI_WAVE ,BH_NONE
	};
	enum ETRAILBONE 
	{
		TB_LHAND, TB_RHAND, TB_CHEST, TB_LKNEE, TB_RKNEE, TB_LFOOT, TB_RFOOT, TB_END                                                                                                                                                                                                                            
	};
	enum ERANK 
	{
		RANK_S, RANK_A, RANK_B, RANK_C, RANK_D 
	};

public:
	struct        FPLAYERSTATE 
	{
		_float        m_fMaxHp      =  100;
		_float        m_fCurHp      =  100;

		_int          m_iScore      = {};
		_int          m_iRankScore  = 200;
		_int          m_iGear       = {};
 
		_int          m_iComboCnt   = {};

		_int          m_iMaxEnergy  = {100};
		_int          m_iCurEnergy  = {};

		_float		  m_fTimingRatio = {};

		ERANK         m_eRank       = {};
		ERANK         m_eTimingRank = {};
	};

private:
	_bool         m_bisDie              = false;
	_bool         m_bPerfectTime        = false;
	_bool         m_bGoodTime           = false;

 
	_float        m_fPerfectDamageMul   = 0.5f;
	_float        m_fGoodDamageMul      = 0.3f;
	_float        m_fFalltoDieWaitTime  = 1.f;

	UniqueTimer   m_pFallRespawnTimer   = nullptr; //fall Wait Timer

public:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& Prototype);
 	virtual ~CPlayer() = default;
public:
	virtual HRESULT Initialize_Prototype(void* pArg);
	virtual HRESULT Initialize(void* pDesc);
	virtual HRESULT Late_Initialize() override;
	virtual void    Priority_Update(_float fTimeDelta);
	virtual void    Update(_float fTimeDelta);
	virtual void    Late_Update(_float fTimeDelta);
	void            Add_Listener() override;
	void            Set_Timer();
	void            SetBB();
	_bool           GetDie() const { return m_bisDie; }
	void            Restart();
public:
	CGameObject*    Clone(void* pArg) override;
	virtual void	Free() override;
	virtual HRESULT	Reset() override;
public:
	HRESULT         Save(void* _pDesc, _uint& _iSize) const override;
	HRESULT         Load(void* _pDesc) override;
	HRESULT         Add_InitComponents() override;

public:  //RP
	void			SetRhythmParry(class CBone* pBone, _int iDir = 1);

	void            EnterRP_WA(_bool firstRp = true); 
	void            TryRp_WA();
	void            MissRp();
	void            SuccessRP_WA();

	void            Activate_RP_START_UI(_float fDuration = 0.7f);

	void            ResetRp_WA();

	void            EnterRP_Mimosa();
	void            SuccessRP_Mimosa();

	void            EnterRP_Kale(_bool isLast = false);
	void            End_RP_Kale();



	void            Kill_RP_Enemy();
	class CEnemy*   Find_RPEnemy();

	_bool			OnDamage(FDamageInfo _fDamageInfo)      override;
private:

	void			SetRhythmParryPos();
	class CBone*	m_pRPBone           = nullptr;
	UniqueTimer     m_pRP_StartUI_Timer = nullptr;
	_float          m_fRP_StatUI_Time   = 0.7f;
	_bool           m_bisRp             = false;
	_bool           m_bisDodgeRp        = false;  
 
private:   //AnimController
	void            SetPartner();
	void 			ChangePartner();
	void            SetWeapon ();
	HRESULT         Set_VFX();
	HRESULT			LoadEffect(VFX eID, const _wstring& strPrototypeTag);
	void            Set_VFXBone();

	void            RestartVFX(VFX eID, _vector pos, _vector look = XMVectorZero());
	void            RestartVFX_PosBone(VFX eID, _vector look = XMVectorZero());
	void            SetCollider();
	void            VFX_SetPos(VFX eID,_vector pos);
	void            VFX_SetLook(VFX eID, _vector Look);
	void            VFX_SetBone(VFX eID, class CBone* pBone);
	void            VFX_SetBonePos(VFX eID, class CBone* pBone, _vector vOffset = XMVectorSet(0.f, 0.f, 0.f, 0.f));
	void            VFX_SetBoneDir(VFX eID, class CBone* pBone);
	void            Stop_VFX();
	void            DebugPos();

public:
	_vector         GetPos();
	_vector         GetLook();
	_vector         Get_Center();
	_vector			Get_BonePos(const _wstring strBoneName);

public:
	void            WeaponChangeAnim(int NodeIdx, float fBlendTime = 0.2f, float    fExitRatio = 1.f, bool bSync = false);
	void            TrashGuitarChangeAnim(int NodeIdx, float fBlendTime = 0.2f, float    fExitRatio = 1.f, bool bSync = false);
	void            TrashGuitarChangeAnim(wstring wstr, float fBlendTime = 0.2f, float fExitRatio = 1.f, bool bSync = false);


private:
	_float3			GetTrailBoneWorldPos(ETRAILBONE eBone) const;
	void			StartEffectTrail();
	void			UpdateEffectTrail();
	void			StopEffectTrail(_bool bImmediately = false);

private:
	unique_ptr<CAnimController>	     m_upAnimController         = nullptr;
	unique_ptr<CAnimController>	     m_upSkeletonAnimController = nullptr;
	ANIMCTX_DESC				     m_ctxDesc                  = {};

private:
	unique_ptr<class CPlayer_FSM>    m_upPlayerFSM      = nullptr;
	Transit_DATA                     m_TransitData      = {};
	_uint                            m_iCurState        = { PS_END };

public:
	void    Set_TransitData();
	HRESULT Load_Config();
	_bool MakeEcosplashGroundOffset(_float3 vBaseOffset, _float3& vOutOffset);
	void    SetBoolElement(string _tag, _bool _value);
	void    IsAppear(_bool bIsAppear) override;

private: //Move
	_float  m_fRunSpeed              = {15.f};
	_float  m_fAirRunSpeed           = { 10.f };
	_float  m_fSpeed                 = {};
	_float  m_fJumpGravity           = {-30.f};
	_float  m_fJumpForce             = {30.f};
	_float  m_fElapsedDash           = {};
	_float  m_fGravityMul            = {1.f};
								     
								     
	_bool   m_bJumpable              = {true} ;
	_bool   m_bIsJump                = {false};
	_bool   m_bMoveKeyInput          = {false};
	_bool   m_bCanMove               = {true} ;
	_bool   m_bCanDoubleJump         = {true} ;
	_bool   m_bDebugGravity          = {false};
								     
	_vector m_vMoveVelocity          = XMVectorZero();
	_vector m_vLookDir               = XMVectorSet(0.f, 0.f, 1.f, 0.f);
							         
	_float  m_fMoveDamp              = 15.f;
	_float  m_fRotateDamp            = 60.f;
	_int    m_iRunIdx                = {};
							         
	_vector m_vRespawnPos            = { XMVectorZero() };
								     
private:
	//Dash
	_int    m_iDashIdx               = {};
	_bool   m_bisDash                = {false};
	_bool   m_bRhythmDash            = {false};
	_bool   m_bChangeDash            = {false};
	_bool   m_bDashAnimEnd           = {true};
	_vector m_vDashDir               = XMVectorZero();
	_float  m_fDashDuration          = 0.55f;
	_float  m_fDashSpeed             = 60.f;
	_float  m_fDashCoolTime          = 0.5f;
	_bool   m_bNextDashQueued        = false;
							    
	UniqueTimer   m_pDashCoolDownTimer = nullptr;

private:  //Magnetic
	_vector m_vMagnetStartPos        = XMVectorZero();
	_vector m_vMagnetTargetPos       = XMVectorZero();
	_vector m_vLastMagnetVel         = XMVectorZero();
	_bool   m_bIsMagnet              = false;
	_bool   m_bLastMagnet            = false;
	_float  m_fMagnetDuration        = 0.5f;
	_float  m_fMagnetElasedTime      = 0.f;
	_float  m_fMagnetWaitTime        = 0.5f;
	_float  m_fMagnetScanRadius      = 50.f;
	_int    m_iMagnetIdx             = {};
	_int    m_iMagnetDirIdx          = {}; //0_Mid,  1_Up,   2_Down
	_int    m_ibeforeLocoIdx         = {}; //0_Idle, 1_Fall, 2_Jump
	_bool   m_bComboXQueued          = false;

	

	UniqueTimer  m_pMagnetWaitTimer  = nullptr;


	enum  EMAGNET_TARGET_TYPE
	{
		INTERACT,
		ENEMY,
		MAGNETRAIL
	};

	struct MAGNET_TARGET_DESC
	{
		CGameObject* pTarget       = nullptr;
		EMAGNET_TARGET_TYPE eType  = EMAGNET_TARGET_TYPE::INTERACT;
		_bool               bLast  = false;
		_float              fScore = -FLT_MAX;
	};
	MAGNET_TARGET_DESC      m_stBestMagnetTarget = {};

private: //Combo
	vector<ECOMBO> m_vecCombo;
				           
	UniqueTimer             m_pComboTimer      = nullptr;
	UniqueTimer             m_pRestTimer       = nullptr;
	UniqueTimer             m_pBeatHitTimer    = nullptr;
				           
	_float                  m_fOnebeatTime     = 0.5f;
	_float                  m_fComboStayTime   = 1.5f;
	_float                  m_fKeyHoldTime     = 0.f;
				           
	_bool                   m_bSuccessBeatHit  = false;
	_bool                   m_bComboAble       = false;
	_bool                   m_bRest            = false;
	_bool                   m_bIsAttack        = false;
	_bool                   m_bIsAirCombo      = false;
	_bool                   m_bIsLinkCombo     = false;
	_bool                   m_bRestInput       = false;
	_bool                   m_bRestTimerActive = false;
	_bool                   m_bRestAble        = false;
				           
	_bool                   m_bPrevXXXX_Active  = false;
	_bool                   m_bPrevXXXX2_Active = false;
				           
	_bool                   m_bFirstbeatSFX    = false;
	_bool                   m_bFirstbeatSFX2   = false;
	_bool                   m_bKeyLock         = false;
				           
	_int                    m_iCombo_X_Idx     = {};
	_int                    m_iCombo_Y_Idx     = {};

	_int                    m_iCurJustCnt      = {};
	const _int              m_iMaxJustCnt      = {4};		           
	UniqueTimer             m_pJustCntTimer    = nullptr;


	FDamageInfo             m_stDamageInfo     = {};

	void                    AddJustCnt();
	void                    EnemyRot();
	void                    RestartMulAtk(_float _duration = 1.f);


	_bool                   m_bBreakDownPending    = false;

	UniqueTimer             m_pBreakDownDelayTimer = nullptr; 
	UniqueTimer             m_pAirComboDelayTimer  = nullptr;

private: //Parry &  Hit
	_bool                   m_bisParry         = false;
	_bool                   m_bSuccessParry    = false;
	_bool                   m_bisHit           = false;
	_bool                   m_bisElectricShock = false;

	_int                    m_iParryIdx        = {};
	_int                    m_iInputWASD       = {};

	_float                  m_fHitTime         = 0.03f;
	_float                  m_fFlickerTime     = 1.f;

	UniqueTimer             m_pHitTimer          = nullptr;
	UniqueTimer             m_pIgniteTimer       = nullptr;
	UniqueTimer             m_pIgniteDamageTimer = nullptr;
	UniqueTimer             m_pFlickerTimer      = nullptr;  

	weak_ptr<CModel>		m_wpSkeletonModel;

	_bool                   m_bFlickerRenderOn  = true;
	_bool                   m_bKaleDodge        = false;
	_float                  m_fFlickerAcc       = 0.f;


	UniqueTimer            m_pBurn1EffectTimer     = nullptr;
	UniqueTimer            m_pBurn2EffectTimer     = nullptr;
	UniqueTimer            m_pBreakDownEffectTimer = nullptr;

	_int                   m_iBurnIdx1          = {};
	_int                   m_iBurnIdx2          = {};
	const _int             m_iBurnPoolSize      = {4};

	_int                   m_iBreakDownIdx      = {};
	const _int             m_iBreakDownPoolSize = { 7 };

	vector<CEffectRoot*>   m_vecBurn1VFX        = {};
	vector<CEffectRoot*>   m_vecBurn2VFX        = {};
	vector<CEffectRoot*>   m_vecBreakDownVFX    = {};
											    
	_float                 m_fVFXBurn1LoopTime  = 0.25f;
	_float                 m_fVFXBurn2LoopTime  = 0.3f;

	void                    ElectricShock();
	void                    EndElectricShock();

	void                    Ignite();
	void                    CheckInputIngnite();
	void                    EndIgnite();

	void                    Flicker_RenderOn();
	void                    Flicker_RenderOff();
	void                    Update_Flicker(_float fTimeDelta);

	void                    ReflectLaser(FDamageInfo _damageInfo);


private: //Gimmic
	_bool                   m_bisGimmic              = false;
	_float                  m_fDetectInteractRange   = 40.f;
	_float                  m_fNearInteractDist      = -FLT_MAX;
	_float4		            m_fInteractPos		     = {};
	_bool                   m_b808AnimOnce           = false;

	class CInteractObject*  m_pNearInteract = nullptr;

	void         CheckInteract();
	void         CheckQTE();

public:
	void         Add_MaxHP(_float fHP);	         
	void         Add_CurHP(_float fHP);	         
	void         Add_Gear(_int iGear);
	void         Unlock_PartnerJam(_int jamIdx);
	void         SetStore(_bool isStore);
	void         SetStoreAble(_bool isAble) { m_bIsStoreAble = isAble; }
	_bool        GetIsJam()const { return m_bIsJam; }
		         
private: //PlayerState
	_bool                   m_bUnlock_PartnerJam[4]; // 0 - PG, 1 - PA, 2- MG ,3-KA
	_bool                   m_bIsStoreAble  = false;
	_bool                   m_bisStore      = false;
	_bool                   m_bIsJam        = false;
private:		            
	void                    CheckState();
				            
	void                    OnLanded();
				 
	void                    SetPartnerLock(_bool isLock = true);
	void                    CheckParry();
	void                    CheckMagnet();
	void                    CheckCombo();
	void                    CheckPartnerJam();
	void                    CheckGimmic();
	void                    CheckGround(_float fTimeDelta);
				            
				            
	void                    StartComboX();
	void                    StartComboY();
	_float                  Calc_Damage(_float _damage);
	_bool                   CheckRest();
	void                    ActiveGuitar();
private:                    
	void                    Move(_float fTimeDelta);
	void                    ApplyGravity(_float fTimeDelta);
	void                    Dash(_float fTimeDelta);
	void                    StayMagnet(_float fTimeDelta);
				            
	_bool                   UpdateMagnetTargetPos();
		                    
public:	                    
	void                    Reset_Dash();
	void                    ResetCombo();
	 			            
	//Enemy List            
	void                    PushEnemy(class CEnemy* pEnemy);
	void                    ClearEnemy();

	/* Set Savepoint func And Falling sensor func */
	void                    SetRespawnPos(_vector vPos) { m_vRespawnPos = vPos; } // Save
	void                    Fall_To_Die();                    //Fall
				            
private:		            
	class CEnemy*           FindTarget();
	void                    AttackSnap(_float fTimeDelta);
	void                    DistortionComboY();
	void                    PlaySoundXXXX();
public: //Camsera           
	_float4                 GetCameraSocketPos();
	void	                SetCameraDir(_float4 vCamDir)      { m_vCamDir = vCamDir; }
	void                    SetCamera(class CPlayer_Cam* pCam) { m_pPlayerCam = pCam; }
				            
//808			            
	void                    Switch808_Ball();
	void                    Switch808_Cat();
	void                    ChangeAnim_808Cat(int NodeIdx, float fBlendTime= 0.2f, float fExitRatio = 1.0, bool bSync = false, int BlendType = 0, int iBeatTransitionmask = 0);
	_int                    GetPartnerIdx() const { return m_iPartnerIdx; }
				            
//CutScene		            
private:		            
	_bool                   m_bisCutScene = false;


	void                    InCutScene();
	void                    OutCutScene();
				            
public:			            
	void                    GuitarTrickCutScene();
	void                    CutSceneReset();
	_bool                   GetIsCutScene() const { return m_bisCutScene; }

	void                    CombatStartUI(_bool isActive = false);

//Mimosa
private:
	_bool                     m_bMimosaDance                      = false;
	_bool                     m_bDanceInputSuccess                = false;
	class CEnemy_Mimosa*      m_pMimosa                           = nullptr; 
	class CEnemy_Kale*        m_pKale                             = nullptr; 
    class CEnemy_Kale_Phase4* m_pBrotherKale                      = nullptr; 
	_int                      m_iPrevMimosaCamPhase               = -1;
	_int                      m_iPrevMimosaCamTurn                = -1;
	_int                      m_iMimosaInputNum                   = 0;
							  
	void                    Reset_DanceInput();
	_int					m_iRP_Dir							= 1;
public:			            
	void                    Enter_MimosaDance();
	void                    Check_MimosaDance();
	void                    End_MimosaDance();
	void                    Fail_MimosaDance();			  

	void                    Mimosa_Grab(CBone* pBone = nullptr);
	void                    Mimosa_Grab_CamBone(CBone* pCamBone = nullptr);

	void                    MimosaCamPlayer(int iSequence);
	void                    MimosaCamMimosa(int iSequence);
				            
	void                    SetMimosa(class CEnemy_Mimosa* pMimosa);
	void                    SetKale(class CEnemy_Kale* pKale); 
	void                    SetBrotherKale(class CEnemy_Kale_Phase4* pKale);
				            
//PartnerJam	            
private:		            
	void                    Pepp_GroundJam();
	void                    Pepp_AirJam();
	void                    Kor_AirJam();
	void                    Maca_GroundJam();

				            
	_bool                   m_bInputPartner = false;

//SpecialAttack
private:		           
	void                    StartHibiki();
	void                    CheckHibiKi();
	void                    Success_Hibiki();
	void                    Fail_Hibiki();
	void                    EndHibiki();
				           
	void                    StartSP808();
	void                    EndSP808();
				           
				           
	void                    ChangeSP();
				           
	_bool                   m_bisHibiki = false;
	_int                    m_iCurSPIdx = {};
	_bool                   m_bHibikiOnce = false;
	UniqueTimer             m_pColorReserveTimer = nullptr;

	_vector                 m_vEcosplashOffset; 
public:			            
	list<CEnemy*>           Get_EnemyList()   const { return m_listEnemy; }
	CEnemy*                 Get_EnemyTarget() const {return  m_pTarget;}
// Score & Chorus           
public:			            
	void                    HitEnemy();
	void                    EnemyKill(_int _score, _int _energy, EAttackType eKillType);
	void                    AddScore(_int iScore);
	void                    OnBeatHitSuccess();
	void                    OnDamagePenalty();
	void                    AddEnergy(_int iEnergy);          
	void                    AddRankScore(_int iValue);	          
	void                    UpdateBattleRank();	          
	void                    UpdateTimingRank();
	void                    BattleResultUI();
				                    
// DSP
private:
	void Notice_DSP_Perfect();
	void SetEmissive();
	void UpdateEmissive(Engine::_float fTimeDelta);

private:		            
				            				    
	_float                  m_fScoreMul           = 1.f;
	_float                  m_fScoreMulTime       = 1.f;
	_float                  m_fAtkHoldTime        = 3.f;
	_float                  m_fJustTimeRatio      = {};
	_float                  m_fElapsedAtkTime     = {};
	_float                  m_fElasedBattleTime   = {};
	_float                  m_fMulAtkTime         = { 0.2f}; //0.2f
	_float                  m_fMulAtk_StayTime    = { 1.0f }; //0.2f

	_int                    m_iInputAttack        = {};
	_int                    m_iJustTimming        = {};
	_int                    m_iMultiHItIdx        = 0;
				            				      
	UniqueTimer             m_pScoreMulTimer      = nullptr;
	UniqueTimer             m_pAtkCntTimer        = nullptr;
	UniqueTimer             m_pMulAtkTimer        = nullptr;
	UniqueTimer             m_pMulAtk_StayTimer   = nullptr;
				            					  
	void                    CalcScore();
	void                    SetScoreMul(_float _ScoreMul, _int iType = 0);

// MagnetRail
private:
	_bool                   m_bIsMagnetRail              = false;
	_bool                   m_bSkipUpdate                = false;
	_bool                   m_bSuccessRail               = false;

	void                    StartMagnetRail();
	void                    StayMagnetRail(_float fTimeDelta);
	void                    EndMangetRail();
	void                    ChangeAnimRail(int NodeIdx, float fBlendTime = 0.2f, float fExitRatio = 1.0, bool bSync = false, int BlendType = 0, int iBeatTransitionmask = 0);
	
	class  CMagnetRail*      m_pRail = nullptr;
	
// QTE

public:
	void Start_QTE(const QTEDESC& tQTEDesc, function<void()> fnSuccess, function<void()> fnFailed);
private:
	void Update_QTE(const _float fTimeDelta);
	QTEINPUTDESC Make_QTEInputDesc();

	class CQTEManager*		m_pQTEManager = { nullptr };
	QTEDESC m_tQTEDesc{};

	_bool                  m_bQtePlay = false;

	void                   StartQTE();
	void                   EndQTE();
	void                   SuccessQTE();
	void                   FailQTE();

						    
private:				    
	_float4				    m_vCamDir                    = { 0.f, 0.f, 1.f, 1.f };
	_float3				    m_vSpawnPos                  = { 0.f, 0.f, 0.f };
	_float3				    m_vSpawnAim                  = { 0.f, 0.f, 1.f };
						    			       	         
	CBone*                  m_pCameraBone                = nullptr;
	CBone*				    m_pVFX_TrailBone[TB_END];
	CBone*                  m_pMimosaGrabBone            = nullptr;
						    
	class CWeapon*          m_pGuitar                    = nullptr;
	class CWeapon*          m_pTrashGuitar               = nullptr;
	class CWeapon*          m_pWeaponPartner[3]          = {};

	class CWeapon*          m_pWeaponPartnerWeaponKorsica_L					= nullptr;
	class CWeapon*          m_pWeaponPartnerWeaponKorsica_R					= nullptr;
	class CWeapon*          m_pWeaponPartnerWeaponPeppermint_MasterBlaster	= nullptr;


	class C808_Ball*        m_p808_Ball                  = nullptr;
	class C808_Cat*         m_p808_Cat                   = nullptr;
	class CPlayer_Cam*      m_pPlayerCam                 = nullptr;
	//Partner:              1 - peppermint, 2 - Macaron, 3 - Korsica
	class CPartner*         pPartners[3]		         = { nullptr , nullptr , nullptr };
	_int				    m_iPartnerIdx		         = 0;
						    
	//Trail				    
	class CVFX_Trail*       m_pMagnetTrail		         = { nullptr };
	class CVFX_Trail*	    m_pEffectTrail[TB_END]       = {};
	class CEffectRoot*	    m_pEffectTrailRoot[TB_END]   = {};
	_float3				    m_vEffectTrailStart[TB_END]  = {};
	_bool				    m_bEffectTrailPlay[TB_END]   = {};
	_bool				    m_bEffectTrailDash           = false;
						    
	CEffectRoot*            m_pEffect[VFX_END]           = {};
	WPCollidor              m_wpBodyCollider;
	WPCollidor              m_wpAttackCollider;
	WPCollidor              m_wpBeatHitCollider[BH_NONE] = {};


												   
	UniqueBeatListener      _upBeatListener          = nullptr;
	UniqueBeatListener      _upPerfectTime           = nullptr;
	UniqueBeatListener      _upGoodTime              = nullptr;
											         
												     
	//Test										     
	_float                  m_fTestTime              = {};
	_float                  m_fTimeGap               = {};
		                    			             
	_bool					m_bFirstRhythmRes	     = true;
	_bool                   m_bInvincible            = false;
												     
	class CUC_GamePlay*		m_pUCGamePlay		     = { nullptr };
	class CUC_Beathit*		m_pUCBeathit		     = { nullptr };
	class CUC_PlayerStatus*	m_pUCPlayerStatus	     = { nullptr };
	class CUC_Interact*		m_pUCInteract		     = { nullptr };
	class CUC_UIEffect*		m_pUCEffect 		     = { nullptr };
	class CUC_Score*		m_pUCScore			     = { nullptr };
	class CUC_RhythmNote*	m_pUCRhythmNote		     = { nullptr };
	class CUC_Widget*		m_pUCWidget				 = { nullptr };
	class CCameraHandler*   m_pMimosaDanceCamHandler = { nullptr };
	class CLevel_GamePlay*	m_pGamePlay				 = { nullptr };
	class CUC_Message*      m_PUCMessage             = { nullptr };
	class CUC_Store*        m_pUCStore               = { nullptr };

	ERhythmResult			m_eRhythmResult		     = { ERhythmResult::None };
	EBEATHIT				m_eBeatHitType		     = { EBEATHIT::BH_NONE   };
												     
    list<class CEnemy*>     m_listEnemy              = {};
	class      CEnemy*      m_pTarget                = nullptr;
	vector<class CEnemy*>   m_vecRotEnemy;

	_bool                   m_bisEnemyRot            = false;
	_float                  m_fDetectEnemyRange      = 30.f;
	_float                  m_fAttackSnapRange       = 10.f;
												     
	FPLAYERSTATE            m_PlayerState            = {};

	Engine::_float3			m_f3EmissiveColor		= {};
};												     
NS_END
