#include "AnimNode.h"
#include "SingleAnimNode.h"
#include "LayerAnimNode.h"
#include "BaseModel.h"

CAnimNode::CAnimNode(shared_ptr<CModel> spModel)
{
    m_wpModel = spModel;
}

CAnimNode::CAnimNode(const CAnimNode& prototype, shared_ptr<CModel> spModel)
    :   m_sID(prototype.m_sID),
        m_iCurAnimationIdx(-1)
{
    m_wpModel = spModel;
}

HRESULT CAnimNode::Initialize(Json::Value jsonValue)
{
    m_sID = jsonValue["ID"].asString();
    return S_OK;
}

HRESULT CAnimNode::LoadClip(ANIMCLIP_DESC& _desc, Json::Value jsonValue)
{
    string sTag;
    if (jsonValue.isMember("AnimationClipTag")) {
        sTag = jsonValue["AnimationClipTag"].asString();
        _desc.iLocomotionAnimIdx = (sTag.empty()) ? -1 : m_wpModel.lock()->FindAnimationClipIdxWithName(s2ws(sTag));
        _desc.bIsLocomotionAnimLoop = jsonValue["IsLoop"].asBool();
    }
    
    if (jsonValue.isMember("DirLocomtionClipTag")){
        for (int i = 0; i < 4; i++) {
            sTag = jsonValue["DirLocomtionClipTag"][i].asString();
             _desc.iDirLocomotionAnimIdx[i] = (sTag.empty()) ? -1 : m_wpModel.lock()->FindAnimationClipIdxWithName(s2ws(sTag));
        }
    }

    if (jsonValue.isMember("NeutralPoseTag")) {
        sTag = jsonValue["NeutralPoseTag"].asString();
        _desc.iNeutralPoseAnimIdx = (sTag.empty()) ? -1 : m_wpModel.lock()->FindAnimationClipIdxWithName(s2ws(sTag));
    }

    if (jsonValue.isMember("DirectionalPoseRefTag")) {
        sTag = jsonValue["DirectionalPoseRefTag"].asString();
        _desc.iDirectionalRefPoseIdx = (sTag.empty()) ? -1 : m_wpModel.lock()->FindAnimationClipIdxWithName(s2ws(sTag));
    }

    if (jsonValue.isMember("LookingPoseRefTag")) {
        sTag = jsonValue["LookingPoseRefTag"].asString();
        _desc.iLookingRefPoseIdx = (sTag.empty()) ? -1 : m_wpModel.lock()->FindAnimationClipIdxWithName(s2ws(sTag));
    }

    if (jsonValue.isMember("ActionAnimTag")) {
        sTag = jsonValue["ActionAnimTag"].asString();
        _desc.iActionAnimIdx = (sTag.empty()) ? -1 : m_wpModel.lock()->FindAnimationClipIdxWithName(s2ws(sTag));
        _desc.bIsActionAnimLoop = jsonValue["IsActionLoop"].asBool();
    }
    if (jsonValue.isMember("RootMotionScale")) {
        _desc.fRootMotionScale.x = (Engine::_float)jsonValue["RootMotionScale"][0].asDouble();
        _desc.fRootMotionScale.y = (Engine::_float)jsonValue["RootMotionScale"][1].asDouble();
    }
    if (jsonValue.isMember("ForceReset")) {
        _desc.bIsForceReset = true;
    }
    if (jsonValue.isMember("Speed")) {
        _desc.fAnimationSpeed = (Engine::_float)jsonValue["Speed"].asDouble();
    }
    return S_OK;
}

unique_ptr<CAnimNode> CAnimNode::Create(Json::Value jsonValue, shared_ptr<CModel> spModel)
{
    unique_ptr<CAnimNode> upAnimNode;
    string _sType =  jsonValue["Type"].asString();
    if      (_sType == "SINGLE") {
        upAnimNode = move(CSingleAnimNode::Create(jsonValue, spModel));
    }
    else if (_sType == "LAYER") {
        auto upLayerNode = CLayerAnimNode::Create(jsonValue, spModel);
        upAnimNode = move(upLayerNode);
    }
    else {
        MSG_BOXL(s2ws("Fail to Load Anim Node : Invalid type name " + _sType).c_str());
        return upAnimNode;
    }

    return upAnimNode;
}
