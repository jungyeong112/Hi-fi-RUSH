#include "AnimTransition.h"
#include "BaseModel.h"

CAnimTransition::CAnimTransition(shared_ptr<CModel> spModel)
{
	m_wpModel = spModel;
}

CAnimTransition::CAnimTransition(const CAnimTransition& prototype, shared_ptr<CModel> spModel)
	:	m_desc			( prototype.m_desc )
{
	for (auto& upTransition : prototype.m_vecTransitions)
	{
		m_vecTransitions.emplace_back(move(upTransition->Clone()));
	}

	m_wpModel = spModel;
}

HRESULT CAnimTransition::Initialize(Json::Value jsonValue)
{
	string sTag;
	Json::Value	_jsonDesc	= jsonValue["Desc"];
	m_desc.m_iPriority		= _jsonDesc["Priority"].asInt();
	m_desc.m_bHasExitTIme	= _jsonDesc["HasExitTime"].asBool();
	m_desc.m_fExitRatio		= _jsonDesc["ExitRatio"].asFloat();
	m_desc.m_fBlendTime		= _jsonDesc["BlendTime"].asFloat();
	if (_jsonDesc.isMember("BlendType"))
		m_desc.eBlendType   =  _jsonDesc["BlendType"].asInt();
	else
		m_desc.eBlendType = 0;

	if (_jsonDesc.isMember("BeatTransition"))
		m_desc.iBeatTransit = _jsonDesc["BeatTransition"].asInt();
	else
		m_desc.iBeatTransit = 0;
	


	Json::Value	_jsonCosnditions = jsonValue["Conditions"];
	for (_int i = 0; i < (_int)_jsonCosnditions.size(); i++)
	{
		ITransitionCondition::ECompareOp op = ITransitionCondition::ECompareOp::Equal;
		string sOPTag = _jsonCosnditions[i]["OP"].asString();
		string sCTXTag = _jsonCosnditions[i]["CTX"].asString();

		if		(sOPTag == "Less")			op = ITransitionCondition::ECompareOp::Less;
		else if (sOPTag == "LessEqual")		op = ITransitionCondition::ECompareOp::LessEqual;
		else if (sOPTag == "Greater")		op = ITransitionCondition::ECompareOp::Greater;
		else if (sOPTag == "GreaterEqual")	op = ITransitionCondition::ECompareOp::GreaterEqual;
		else if (sOPTag == "Equal")			op = ITransitionCondition::ECompareOp::Equal;
		else if (sOPTag == "NotEqual")		op = ITransitionCondition::ECompareOp::NotEqual;

		
		ITransitionCondition* pCondition = nullptr;
		if (_jsonCosnditions[i]["Value"].isBool()) {
			auto _value = _jsonCosnditions[i]["Value"].asBool();
			if		(sCTXTag == "IsGround")		pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsGround , _value, op);
			else if (sCTXTag == "IsDead")		pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsDead , _value, op );
			else if (sCTXTag == "IsSoftHit")	pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsSoftHit , _value, op );
			else if (sCTXTag == "IsHardHit")	pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsHardHit , _value, op );
			else if (sCTXTag == "IsGrabbed")	pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsGrabbed, _value, op );
			else if (sCTXTag == "IsMoving")		pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsMoving , _value, op );
			else if (sCTXTag == "IsAnimEnd")	pCondition = new CtxValueCondition( &ANIMCTX_DESC::bIsAnimEnd , _value, op );
			else								pCondition = new CtxBlackBoardCondition<bool>(sCTXTag, _value, op);
		}
		else if (_jsonCosnditions[i]["Value"].isInt()) {
			auto _value = _jsonCosnditions[i]["Value"].asInt();

			pCondition = new CtxBlackBoardCondition<int>(sCTXTag, _value, op);
		}
		else if (_jsonCosnditions[i]["Value"].isDouble()) {
			auto _value = _jsonCosnditions[i]["Value"].asFloat();

			if (sCTXTag == "Speed")				pCondition = new CtxValueCondition( &ANIMCTX_DESC::fSpeed ,		_value, op );
			else if (sCTXTag == "AnimRatio")	pCondition = new CtxValueCondition( &ANIMCTX_DESC::fAnimRatio ,	_value, op );
			else								pCondition = new CtxBlackBoardCondition<float>(sCTXTag, _value, op);
		}
		if (!pCondition) {
			string sMSG = "Fail to Create AnimTransition : Invalid Condition ( " + sCTXTag + " / " + sOPTag + " )";

			MSG_BOXL(s2ws(sMSG).c_str());
			return E_FAIL;
		}
		
		
		m_vecTransitions.emplace_back(unique_ptr<ITransitionCondition>(pCondition));
	}
	
	return S_OK;
}

_bool CAnimTransition::Evaluate(const ANIMCTX_DESC& CTX)
{
	_bool bCanTransit = true;
	for (auto& pTransition : m_vecTransitions) if (!pTransition->Evaluate(CTX)) bCanTransit = false;
	return bCanTransit;
}

unique_ptr<CAnimTransition> CAnimTransition::Clone(shared_ptr<CModel> spModel)
{
	unique_ptr<CAnimTransition> _upAnimTransition = unique_ptr<CAnimTransition>(new CAnimTransition(*this, spModel));

	return _upAnimTransition;
}

unique_ptr<CAnimTransition> CAnimTransition::Create(Json::Value jsonValue, shared_ptr<CModel> spModel)
{
	if (!spModel) {
		MSG_BOX("Fail to create AnimTransition : Empty Model");
		return nullptr;
	}

	unique_ptr<CAnimTransition> upAnimTransition = unique_ptr<CAnimTransition>(new CAnimTransition(spModel));
	if (FAILED(upAnimTransition->Initialize(jsonValue))) {
		MSG_BOX("Fail to create AnimTransition : fail to initialize");
		return nullptr;
	}

	return upAnimTransition;
}
