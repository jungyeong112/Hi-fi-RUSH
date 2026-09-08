#pragma once
#include "Base.h"
#include "BlackBoard.h"

NS_BEGIN(Engine)
class CModel;

struct ITransitionCondition {
	using TYPE = variant<int, float, bool>;
	enum ECompareOp {
		Less			= 0xb001,
		LessEqual		= 0xb011,
		Greater			= 0xb100,
		GreaterEqual	= 0xb110,
		Equal			= 0xb010,
		NotEqual		= 0xb101,
	};

	virtual ~ITransitionCondition() = default;
	virtual unique_ptr<ITransitionCondition> Clone() PURE;
	virtual bool Evaluate(const ANIMCTX_DESC& ctx) const = 0;
};

template<typename T>
struct CtxValueCondition final : ITransitionCondition {
	using MemberPtr = T ANIMCTX_DESC::*;
	CtxValueCondition(MemberPtr member, T         value, ITransitionCondition::ECompareOp op) :
		member(member), value(value), op(op) {
	}
	MemberPtr member;
	T         value;
	ITransitionCondition::ECompareOp op;

	bool Evaluate(const ANIMCTX_DESC& ctx) const override {
		const T& lhs = ctx.*member;
		const T& rhs = value;

		switch (op) {
			case ECompareOp::Less:          return lhs < rhs;
			case ECompareOp::LessEqual:     return lhs <= rhs;
			case ECompareOp::Greater:       return lhs > rhs;
			case ECompareOp::GreaterEqual:  return lhs >= rhs;
			case ECompareOp::Equal:         return lhs == rhs;
			case ECompareOp::NotEqual:      return lhs != rhs;
		}
		return false;
	}
	virtual unique_ptr<ITransitionCondition> Clone() override {
		return unique_ptr<ITransitionCondition>(new CtxValueCondition(member, value, op));
	}
};
template<typename T>
struct CtxBlackBoardCondition final : ITransitionCondition {
	CtxBlackBoardCondition(string memberName, T value, ITransitionCondition::ECompareOp op) :
		memberName(memberName), value(value), op(op) {
	}
	string		memberName;
	T			value;
	ECompareOp	op;

	bool Evaluate(const ANIMCTX_DESC& ctx) const override {
		T lhs;
		ctx.pBlackboard->GetElement<T>(memberName, lhs);
		const T& rhs = value;

		switch (op) {
		case ECompareOp::Less:          return lhs < rhs;
		case ECompareOp::LessEqual:     return lhs <= rhs;
		case ECompareOp::Greater:       return lhs > rhs;
		case ECompareOp::GreaterEqual:  return lhs >= rhs;
		case ECompareOp::Equal:         return lhs == rhs;
		case ECompareOp::NotEqual:      return lhs != rhs;
		}
		return false;
	}
	virtual unique_ptr<ITransitionCondition> Clone() override {
		return unique_ptr<ITransitionCondition>(new CtxBlackBoardCondition(memberName, value, op));
	}
};

class ENGINE_DLL CAnimTransition : public CBase
{
public:
	virtual ~CAnimTransition() = default;
private:
	CAnimTransition(shared_ptr<CModel>	spModel);
	CAnimTransition(const CAnimTransition& prototype, shared_ptr<CModel>	spModel);
	CAnimTransition& operator=(const CAnimTransition&) = delete;
	CAnimTransition(const CAnimTransition& prototype) = delete;
public:
	HRESULT				Initialize(Json::Value jsonValue);

	_bool Evaluate(const ANIMCTX_DESC& CTX);
	const ANIMTRANSITION_DESC& GetDesc() { return m_desc; }
	pair<string, string> GetTransitID() {
		return { m_sSrtAnimNodeID , m_sDstAnimNodeID };
	}
	void SetTransitIdx(_int _iSrtIdx, _int _iDstIdx) { 
		m_desc.m_iSrtAnimNodeIdx = _iSrtIdx;
		m_desc.m_iDstAnimNodeIdx = _iDstIdx;
	}
public:
	unique_ptr<CAnimTransition>			Clone(shared_ptr<CModel>	spModel);
	static  unique_ptr<CAnimTransition>	Create(Json::Value jsonValue, shared_ptr<CModel>	spModel);
private:
	weak_ptr<CModel>							m_wpModel;
	ANIMTRANSITION_DESC							m_desc;
	vector<unique_ptr<ITransitionCondition>>	m_vecTransitions;

	string										m_sSrtAnimNodeID = "";
	string										m_sDstAnimNodeID = "";
};

NS_END
