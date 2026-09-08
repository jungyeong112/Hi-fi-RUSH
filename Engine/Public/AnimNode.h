#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CModel;

class ENGINE_DLL CAnimNode abstract: public CBase
{
public:
	enum class EANIMNODETYPE {
		SINGLE,
		BLE,
		TRANSIT,
		LAYER
	};
	virtual ~CAnimNode() = default;
protected:
	CAnimNode(shared_ptr<CModel>	spModel);
	CAnimNode(const CAnimNode& prototype, shared_ptr<CModel>	spModel);
public:
	string		 GetID() const { return m_sID; }
	virtual void Update(float _fDeltaTime)							PURE;
	virtual void UpdateCTX(const ANIMCTX_DESC& ctx)					PURE;
	virtual HRESULT Initialize(Json::Value jsonValue)				PURE;
public:
	virtual bool CanExit(ANIMTRANSITION_DESC _desc)					PURE;
	virtual void Enter(ANIMTRANSITION_DESC _desc)					PURE;
	virtual void Exit(ANIMTRANSITION_DESC _desc)					PURE;
	virtual void Reset()											PURE;
	virtual bool IsEnd() { return false; }
	virtual void SetRootMotionScale(float fXScale, float fYScale, int iLayerIdx = 0)	PURE;
	virtual float GetAnimRatio()									PURE;
protected:
	HRESULT LoadClip(ANIMCLIP_DESC& _desc, Json::Value jsonValue);
public:
	static  unique_ptr<CAnimNode>	Create(Json::Value jsonValue,	shared_ptr<CModel>	spModel);
	virtual unique_ptr<CAnimNode>	Clone(shared_ptr<CModel>	spModel) PURE;
	weak_ptr<CModel>	m_wpModel;
	string				m_sID				= "";
	_int				m_iCurAnimationIdx	= -1;
};

NS_END
