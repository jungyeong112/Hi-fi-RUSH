#pragma once

#include "Component.h"
#include "Asset.h"
#include "Picking.h"
#include "Animation.h"

NS_BEGIN(Engine)
class CBone;
//class CAnimation;

class ENGINE_DLL CModel : public CComponent
{
	friend class CComponent;
public:
	enum EBONEID
	{
		MAIN,

		BLENDA,
		BLENDB,

		REF,
		END
	};
	enum EBLENDINGDELTATYPE {
		CURRENT,
		BLEND,
		ZERO
	};
	enum ESHADOWTYPE
	{
		NONE,
		STATIC,
		DYNAMIC
	};
	enum NotifyType
	{
		NT_ANIM, NT_VFX, NT_SFX , NT_END
	};

	typedef struct tagConstantShaderParameterDesc
	{
		_int		iRendererIdx			= 0;
		_int		iType					= 0; //bool, int, float, float4
		_float4		vData					= {0.f,0.f,0.f,0.f};
		_char		sConstantName[60]		= "";
	} CONSTANTSHADERPARAM_DESC;
	

	typedef struct tagModelDesc {
		_tchar		m_sModelAssetName[60]	= L"";
		_bool		m_bIsCopy				= false;
		_uint		m_uiShadow				= 0;
		_int		m_iParamSize			= 0;
		CONSTANTSHADERPARAM_DESC m_paramDescs[20];
		_int	    m_iRSParamSize			= 0;
		RSPARAM_DESC			 m_rsParamDescs[10];
	} MODEL_DESC;

public:
	enum ENOTIFYCONFIGTYPE
	{
		NOTIFY_CONFIG_BASIC = 0,
		NOTIFY_CONFIG_IMPULSE,
		NOTIFY_CONFIG_SFX,
	};

	struct NOTIFY_CONFIG_ENTRY
	{
		_int		iType = NOTIFY_CONFIG_BASIC;
		_wstring	sAnimTag = L"";
		_wstring	sNotifyTag = L"";

		_float		fStartRatio = 0.f;
		_float		fEndRatio = 0.f;
		_float		fTargetRatio = 0.f;

		_float		fVolume = 1.f;
		_int		iPriority = 0;
		_float		fEndPoint = 50.f;
	};



protected:
	vector<NOTIFY_CONFIG_ENTRY> m_vecNotifyConfigEntries[NT_END];
protected:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& Prototype);
	virtual ~CModel() = default;
public:
	HRESULT	Ready_MeshRenderers();
	virtual HRESULT Load_Asset() override;
public:
	_uint Get_NumMeshes() const { return m_iNumMeshes; }
	const _wstring Get_ModelName() const { return m_desc.m_sModelAssetName; }

public:
	virtual HRESULT Initialize_Prototype(void* pDesc)	override;
	virtual HRESULT Initialize(void* pArg)				override;
	virtual HRESULT Late_Initialize()					override;

	virtual void	Update(_float fDeltaTime)			override;
	void ResetPrevAnim();
	void RenderActive(_bool IsActive);
public:
	virtual void Free() override;

	SPComponent Clone(void* pArg) override;
	void Render_Inspector() override;
	HRESULT Save(void* _pDesc, _uint& _iSize) const override;
	HRESULT Load(void* _pDesc) override;

	//Only pick one mesh even if Ray Penetrates multiple Meshs 
	_bool		PickingMesh(RAY InWorldRay, CPicking::PICKING_DESC* pDesc);
	shared_ptr<class CMeshRenderer> GetMeshRenderer(_int idx);
	//
	HRESULT		TransitAnimation(int iNxtAnimIdx, bool bForceTransit = true, float fBlendDuaration = -1, EBLENDINGDELTATYPE eBlendType = EBLENDINGDELTATYPE::BLEND);

	wstring		FindAnimTag(_uint iAnimIdx);
	HRESULT		TransitAnimationClip(const ANIMCLIP_DESC& NxtAnimClip, const CLIPTRANSIT_DESC& NxtAnimTransition);
	_bool		IsAnimationClipEnd(const ANIMCLIP_DESC& animClip);

	_bool IsAnimEnd(_int idx);


	CBone*      FindBoneWithName(const wstring& _boneName);
	CBone*      FindBoneWithIdx(int iBoneIdx);
	_int		FindBoneIdxWithName(const wstring& _boneName);


	CAnimation* FindAnimationClipWithName(const wstring& animationName);
	_int		FindAnimationClipIdxWithName(const wstring& animationName);
	_int        FindAddtiveAnimIdx(_int animIdx);
	_int        FindRefPoseAnimIdx(_int animIdx);
	_int        FindLookAnimIdx(_int animIdx);

	HRESULT		AddMatchingBone(class CBone* pBone, const wstring& PivotSrtBoneName, const wstring& PivotDstBoneName, bool bIsLocationOnly);
	HRESULT		AddMatchingBone(const wstring& targetBoneName, const wstring& PivotSrtBoneName, const wstring& PivotDstBoneName, bool bIsLocationOnly);
	HRESULT		AddBoneScale(const wstring& targetBoneName, float fScalingFactor);
	HRESULT		ActiveMatchingBone(_int idx, bool bIsActive);
	HRESULT		ClearMatchingBone();

	float		GetAnimRatio(int idx);
	float		GetAnimRatio();
	void		AnimReset(int idx);
	void		Render_BoneCombo(int* idx);
	void		SetCurrentDir();
	HRESULT		Snap(class CBone* pBone, int iPivotBoneIdx, _bool bLocationOnly = true);
	void		SetRootMotionScale(float fXScale, float fYScale);

	_vector     GetBoneWorldPos(const vector<CBone*>& vecBones,_int idx);
	_matrix     GetBoneCombineMatrix(const vector<CBone*>& vecBones,_int idx);
	_matrix     GetParentCombinedMatrix(const vector<CBone*>& vecBones,_int idx);

	void        SetBoneLocalMatrix(const vector<CBone*>& vecBones,_int idx, _matrix matLocal);
	void        SetBoneWorldRotation(const vector<CBone*>& vecBones,_int idx, _vector qworldRot);

	_bool       CheckIkChain(const vector<CBone*>& vecBones,_int rootIdx, _int midIdx, _int tipIdx) const;
	void        ReCalcCombined(const vector<CBone*>& vecBones,_int startIdx);
	_bool       SolveIK(const vector<CBone*>& vecBones, _int rootIdx, _int midIdx, _int tipIdx, _vector vTargetPos, _vector vPolePos, _float fDeltaTime);
	void		SetDashAfterimage(Engine::_bool bDash);
	void		SetKaleAfterimage(Engine::_bool bDash);
	void		SetExplosion(Engine::_bool bExplosion);

	void		Render_CameraBone(CBone*& pBone);
protected:
	bool		Check_BeatTransition();
	void		Set_RenderParam();
	void		Render_ModelFilter();
	void		Render_ModelInfo();
	void		Render_AnimationInfo();
	pair<_float4, _float>  UpdateAnimationDeltaMatrix(int iAnimaionIdx, _fmatrix matPrevLocal, _cmatrix matCurLocal, _cmatrix matWorldDir);
	_matrix		BlendMatrix(_fmatrix matPrevLocal, _cmatrix matCurLocal, _float fRatio);
	HRESULT		BoneMatch(int iTargetBoneIdx, int iPivotSrcBoneIdx, int iPivotDstBoneIdx, _bool bLocationOnly = true);
	HRESULT		BoneMatch(class CBone* pBone, int iPivotSrcBoneIdx, int iPivotDstBoneIdx, _bool bLocationOnly = true);
	HRESULT		BoneScale(int iTargetBoneIdx, float fScalingFactor);
	_float4x4	GetModelDir();

	//BoneConstraint
	void		UpdateBoneConstraints();

	void ResetRhythmSyncState(_int iNewAnimIdx);

	//Find
	void Render_CameraBone();

public:
const char* NotifyTypeToString(NotifyType eType) const;
	
public:
	void        AnimStart_RhythmTarget(_float fTargetRatio);
	void        AnimSet_RhythmTarget(_float fTargetRatio);
	void        AnimRefresh_RhythmAnimChange();
	HRESULT     AddListener(wstring _tag, function<void(_float)> funcUpdate,
		function<void(void)> funcEnter, function<void(void)> funcExit, _bool isOnce = false);

	HRESULT     HandleNotifies(list<ANIMNOTIFY_DESC> _listNotifies);
	void        Update_Listener(_float fTimeDelta);

	HRESULT     AddNotify(_wstring _animName, ANIMNOTIFY_DESC& _desc);
	vector<ANIMNOTIFY_DESC>& Get_Notifies(_int iAnimIdx) { return m_vecAnimations[iAnimIdx]->Get_Notifies(); }
	//notify helper
	void        Add_Notify(_float fInitframe, _float fEndFrame, _float fTotalFrame, _wstring AnimTag, _wstring NotifyTag);
	void        Add_ImpulseNotify(_float fTargetFrame, _float fTotalFrame, _wstring AnimTag, _wstring NotifyTag);
	void        Add_SFXNotify(_float fTargetFrame, _float fTotalframe, _wstring AnimTag, _wstring SFXTag,
		_float fVolume, _int iPriority, _float EndPoint);
	void        Add_VFXNotify(_float fTargetFrame, _float fTotalFrame, _wstring AnimTag, _wstring VFXTag);
//editor
public:
	bool		Render_AnimationCombo(int* idx, string sName, bool bExcludePose = true);

public:
	_wstring    GetNotifyJsonPath(NotifyType eType) const;
	void        SortNotifyConfigEntries(NotifyType eType);
	void        SortAllNotifyConfigEntries();
	void		ClearRuntimeNotifyData();
	void        ApplyAnimNotifyEntry(const NOTIFY_CONFIG_ENTRY& entry);
	void        ApplyVFXNotifyEntry(const NOTIFY_CONFIG_ENTRY& entry);
	void        ApplySFXNotifyEntry(const NOTIFY_CONFIG_ENTRY& entry);
	void        ApplyAllNotifyConfigEntriesToRuntime();
	void        LoadNotifyConfigFromJson(NotifyType eType);
	void        LoadAllNotifyConfigFromJson();
	void        SaveNotifyConfigToJson(NotifyType eType) const;
	void        SaveAllNotifyConfigToJson() const;
	void        SyncNotifyConfigEntriesFromRuntime(NotifyType eType);
	void		SetModelAnimationSpeed(_float fSpeed) { m_fAnimationSpeed = clamp(fSpeed, 0.f, 1.f); }
public: //Addtive
	void        Activate_Aditive(_int _animIdx = -1);
    void        DeActivate_Aditive();
	void		SetIdentityRefBones();

	void        SetLookRatio(_float2 _lookRatio) { m_fLookRatio = _lookRatio; }
	void        SetLookUse(_bool bUseLook) { m_bUseLook = bUseLook; }
protected:
	_matrix		ApplyLocomotionAnim(float fDeltaTime, const vector<CBone*>& vecBones, const ANIMCLIP_DESC& animClipDesc);
	void        ApplyLookPose(const vector<CBone*>& vecBones);
	void        ApplyLocomotionAddtiveAnim(float fDeltaTime, const vector<CBone*>& vecBones, const ANIMCLIP_DESC& animClipDesc);
	void        ApplyIk(const vector<CBone*>& vecBones,_float fTimeDelta);
protected:
	MODEL_DESC								m_desc = {};

	SP_ModelAsset							m_spModelAsset = nullptr;
	//Mesh
	_uint									m_iNumMeshes = { 0 };
	vector<shared_ptr<class CMeshRenderer>>	m_vecSPMeshRenderers;
	//Bone
	_uint									m_iRootBoneIdx = {};
	vector<CBone*>							m_vecBones[EBONEID::END];

	vector<vector<int>>						m_vecBoneChildrens;
	vector<int>								m_vecBoneLastChildrenIdx;
	function<CBone* (int)>					m_funcBoneFinder = nullptr;

	//Animation
	_uint									m_iNumAnimations = {};
	vector<class CAnimation*>				m_vecAnimations;

	_float4x4								m_iPrevAnimationDir = {};
	_float4x4								m_iCurAnimationDir = {};
	_float									m_fAnimationSpeed = 1.f;

	//Root Motion
	_float4x4								m_matPrevLocalRoot = IDENTITYMATRIX;
	_float4x4								m_matPrevLocalRootA = IDENTITYMATRIX;
	_float4x4								m_matPrevLocalRootB = IDENTITYMATRIX;

	//Animation Clip
	ANIMCLIP_DESC							m_prevAnimClip;
	ANIMCLIP_DESC							m_curAnimClip;
	ANIMCLIP_DESC							m_nextAnimClip;

	CLIPTRANSIT_DESC						m_ClipTransitionDesc;
	float									m_fBlendTime = 0.f;

	//Bone Match
	vector<BONECONSTRAINT_DESC>				m_vecBoneMatchIndices;

	bool m_bListOpen = false;
	bool m_bIsResetPrevAnim = false;

	//Shadow
	Engine::_uint m_uiOldShadow;

	//Notify
	map<_wstring, list<class CListener*>>   m_mapListener;
	// Addtive
	_bool                                   m_bIsOnAddtiveAnim  = false;
	_bool                                   m_bIsAddtiveAnimFirstFrame = false;
	_int                                    m_iAddtiveAnimIdx   = -1;
	_int                                    m_iRefPoseAnimIdx   = -1;
	vector<_int>                            m_vecAddtiveAnimIdx {};

	//Look
	_float2                                 m_fLookRatio                  = {};
	vector<_int>                            m_vecLookAnimIdx              = {};
	_bool                                   m_bUseLook                    = false;
	_float									m_fLookingWeights[3][3]       = { {0.f,0.f,0.f} , {0.f,0.f,0.f}, {0.f,0.f,0.f} };
	_int                                    m_arrLookingPoseIndices[3][3] = { {-1,-1,-1} , {-1,-1,-1}, {-1,-1,-1} };

	//IK
	_bool                                   m_bUseIK         = false;      
	_vector                                 m_vIKTargetPos   = XMVectorSet(0,0,0,1);
	_vector                                 m_vIKPolePos     = XMVectorSet(0, 0, 0, 1);
	_int                                    m_iIkTipBoneIdx  = {};
	_int                                    m_iIkMidBoneIdx  = {};
	_int                                    m_iIkRootBoneIdx = {};
	_float                                  m_fIKWeight      = {};
	_vector                                 m_vIKSmoothedTargetPos  = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	_bool                                   m_bInitIKSmoothedTarget = false;

	_vector                                 m_vLastIKBendAxis        = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_bool                                   m_bHasLastIKBendAxis     = false;

	_float                                  m_fIKTargetFollowSpeed  = 8.f;     
	_float                                  m_fIKMaxRootTurnSpeed   = XMConvertToRadians(170.f); 
	_float                                  m_fIKMaxMidTurnSpeed    = XMConvertToRadians(260.f); 
	_float                                  m_fIKStraightMargin     = 0.15f;

	_bool                                   m_bRhythmSpeedAdjusting = false;      

	_int                                    m_iRhythmCalculatedAnimIdx = -1;

	_bool  m_bRhythmSyncActive = false;        // ���� ���� ��
	_bool  m_bRhythmSyncedThisAnim = false;    // �� �ִϸ��̼ǿ��� �̹� ���� ����


	_float m_fRhythmTargetRatio = 0.f;
	_float m_fRhythmSpeedCoeff = 1.f;

	_float m_fPrevRhythmAnimRatio = 0.f;

	NotifyType                             m_eNotifyType = { NT_ANIM };
};		                                                                     
		                                    
NS_END	                                    