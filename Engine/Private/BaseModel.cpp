#include "BaseModel.h"
#include "Mesh.h"
#include "Bone.h"
#include "Animation.h"
#include "MeshRenderer.h"
#include "GameInstance.h"
#include "Listener.h"
#include "CameraObject.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
	: CComponent(Prototype),
	m_desc(Prototype.m_desc)
{
}

HRESULT CModel::Ready_MeshRenderers()
{
	if (!m_spModelAsset) return S_OK;

	m_vecSPMeshRenderers.clear();
	m_vecSPMeshRenderers.resize(m_iNumMeshes, nullptr);
	for (size_t i = 0; i < m_iNumMeshes; i++) {
		auto _pMesh = (*m_spModelAsset)[i];
		auto _pMeshRenderer = CComponent::Create<CMeshRenderer>(m_pDevice, m_pContext);
		shared_ptr<CMeshRenderer> _spMeshRenderer(
			_pMeshRenderer,
			[](CMeshRenderer* p) { Safe_Release(p); }
		);

		if (m_desc.m_bIsCopy)	_pMeshRenderer->SetCopyMesh(_pMesh);
		else					_pMeshRenderer->SetRefMesh(_pMesh);



		_spMeshRenderer->SetWPComponent(ConvertWPComponent<CComponent>(_spMeshRenderer));
		_spMeshRenderer->SetBoneFinder(m_funcBoneFinder);
		_spMeshRenderer->SetGameObject(m_pOwnerObj);
		_pMeshRenderer->Initialize(nullptr);
		m_vecSPMeshRenderers[i] = _spMeshRenderer;
	}
	return S_OK;
}

HRESULT CModel::Load_Asset()
{
	if (!m_spModelAsset) {
		SP_Asset _spAsset;
		CHKFAIL(m_pGameInstance->LoadAsset(_spAsset, m_desc.m_sModelAssetName))
			if (_spAsset) m_spModelAsset = dynamic_pointer_cast<CModelAsset>(_spAsset);
	}
	auto _desc = m_spModelAsset->GetInfo();
	m_iNumMeshes = _desc.iNumMeshs;
	m_iRootBoneIdx = _desc.iRootBoneIdx;

	//GetBone
	for (int i = 0; i < EBONEID::END; i++) {
		for (auto _pBone : m_vecBones[i]) Safe_Release(_pBone);
		m_vecBones[i].clear();
	}

	auto _vecBoneDescs = m_spModelAsset->GetBoneDescs();
	for (const auto& _boneDescs : _vecBoneDescs) {
		for (int i = 0; i < EBONEID::END; i++) {
			m_vecBones[i].push_back(CBone::Create(_boneDescs));
		}
	}

	//GetAnimation
	for (auto _pAnimation : m_vecAnimations) Safe_Release(_pAnimation);
	m_vecAnimations.clear();
	m_vecAnimations = m_spModelAsset->GetAnimations();
	m_iNumAnimations = (Engine::_uint)m_vecAnimations.size();

	return S_OK;
}

HRESULT CModel::Initialize_Prototype(void* pDesc)
{
	CHKFAIL(__super::Initialize_Prototype(pDesc))

		return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	CHKFAIL(__super::Initialize(pArg))
		m_funcBoneFinder = [this](int i)->CBone* {
		if (i < m_vecBones[MAIN].size())
			return m_vecBones[MAIN][i];
		else
			return nullptr;
		};

	m_bIsUpdateComponent = true;

	return S_OK;
}

HRESULT CModel::Late_Initialize()
{
	CHKFAIL(__super::Late_Initialize())
		Ready_MeshRenderers();
	m_vecBoneChildrens.resize(m_vecBones[MAIN].size(), vector<int>());
	m_vecBoneLastChildrenIdx.resize(m_vecBones[MAIN].size(), 0);
	for (auto pBone : m_vecBones[MAIN])
	{
		auto _desc = pBone->GetDesc();
		if (_desc.m_iParentBoneIndex != -1)
			m_vecBoneChildrens[_desc.m_iParentBoneIndex].push_back(_desc.m_iBoneIndex);
	}

	stack<int> st;
	for (size_t i = 0; i < m_vecBones[MAIN].size(); i++)
	{
		st.push((Engine::_int)i);
		while (st.size()) {
			int iCurIdx = st.top();
			if (m_vecBoneChildrens[iCurIdx].empty() || i >= (*max_element(m_vecBoneChildrens[iCurIdx].begin(), m_vecBoneChildrens[iCurIdx].end()))) {
				m_vecBoneLastChildrenIdx[iCurIdx] = (Engine::_int)i;
				st.pop();
			}
			else {
				break;
			}

		}
	}
	for (auto _pMeshRenderer : m_vecSPMeshRenderers) {
		_pMeshRenderer->CheckLateInitialize();
	}

	if (m_vecBones[MAIN].size()) m_vecBones[MAIN][m_iRootBoneIdx]->SetAnimMatrix(XMMatrixIdentity());
	m_curAnimClip.iLocomotionAnimIdx = 0;
	if (m_iRootBoneIdx != 0) m_iCurAnimationDir = GetModelDir();

	Set_RenderParam();
	if (!m_vecAnimations.empty())
	{
		LoadAllNotifyConfigFromJson();
		ApplyAllNotifyConfigEntriesToRuntime();
	}

	for (_int i = 0; i < m_vecAnimations.size(); i++)
	{
		auto _idx = FindAddtiveAnimIdx(i);
		if (_idx != -1)
			m_vecAddtiveAnimIdx.push_back(_idx);
		auto _refIdx = FindRefPoseAnimIdx(i);
		if (_refIdx != -1)
			m_iRefPoseAnimIdx = _refIdx;
		auto _lookIdx = FindLookAnimIdx(i);
		if (_lookIdx != -1 && m_vecLookAnimIdx.size() < 8)
			m_vecLookAnimIdx.push_back(_lookIdx);
	}
	if (m_vecLookAnimIdx.size() == 8)
	{
		m_arrLookingPoseIndices[0][0] = m_vecLookAnimIdx[4];
		m_arrLookingPoseIndices[0][1] = m_vecLookAnimIdx[0];
		m_arrLookingPoseIndices[0][2] = m_vecLookAnimIdx[5];

		m_arrLookingPoseIndices[1][0] = m_vecLookAnimIdx[2];
		m_arrLookingPoseIndices[1][1] = -1;  //Center - 예외
		m_arrLookingPoseIndices[1][2] = m_vecLookAnimIdx[3];

		m_arrLookingPoseIndices[2][0] = m_vecLookAnimIdx[6];
		m_arrLookingPoseIndices[2][1] = m_vecLookAnimIdx[1];
		m_arrLookingPoseIndices[2][2] = m_vecLookAnimIdx[7];
	}




	if (m_iRefPoseAnimIdx != -1)
		m_vecAnimations[m_iRefPoseAnimIdx]->Update_Addtive_FirstFrame(m_vecBones[REF]);



	return S_OK;
}

void CModel::Update(_float fDeltaTime)
{
	__super::Update(fDeltaTime);

	if (m_vecAnimations.empty()) return;

	//Transit
	if (m_nextAnimClip.iLocomotionAnimIdx != -1 &&
		(m_ClipTransitionDesc.bForceTransit || IsAnimationClipEnd(m_curAnimClip))
		&& Check_BeatTransition()
		) {
		m_bIsResetPrevAnim = false;
		m_prevAnimClip = m_curAnimClip;
		m_curAnimClip = m_nextAnimClip;
		ANIMCLIP_DESC _desc;
		m_nextAnimClip = _desc;

		m_iRhythmCalculatedAnimIdx = -1;

		m_iPrevAnimationDir = m_iCurAnimationDir;
		m_matPrevLocalRootA = m_matPrevLocalRootB;
		if (m_curAnimClip.iLocomotionAnimIdx != m_prevAnimClip.iLocomotionAnimIdx) {
			if (m_curAnimClip.iLocomotionAnimIdx != -1) m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->Reset();
			if (m_prevAnimClip.iLocomotionAnimIdx != -1) HandleNotifies(m_vecAnimations[m_prevAnimClip.iLocomotionAnimIdx]->Reset_Notifies());
			m_iCurAnimationDir = GetModelDir();
			m_matPrevLocalRootB = IDENTITYMATRIX;
		}
		else if (m_curAnimClip.bIsForceReset) {
			if (m_prevAnimClip.iLocomotionAnimIdx != -1) HandleNotifies(m_vecAnimations[m_prevAnimClip.iLocomotionAnimIdx]->Reset_Notifies());
			if (m_curAnimClip.iLocomotionAnimIdx != -1) m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->Reset();
			m_iCurAnimationDir = GetModelDir();
			m_matPrevLocalRootB = IDENTITYMATRIX;
		}

		if (m_curAnimClip.iActionAnimIdx != m_prevAnimClip.iActionAnimIdx) {
			if (m_curAnimClip.iActionAnimIdx != -1) m_vecAnimations[m_curAnimClip.iActionAnimIdx]->Reset();
		}



		for (size_t i = 0; i < 4; i++) {
			if (m_curAnimClip.iDirLocomotionAnimIdx[i] != -1) m_vecAnimations[m_curAnimClip.iDirLocomotionAnimIdx[i]]->SetLoop(true);
		}
		if (m_curAnimClip.iLocomotionAnimIdx != -1) {
			m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->SetLoop(m_curAnimClip.bIsLocomotionAnimLoop);
			if (m_ClipTransitionDesc.m_bSync && m_prevAnimClip.iLocomotionAnimIdx != -1) {
				_float fTrackPosition = m_vecAnimations[m_prevAnimClip.iLocomotionAnimIdx]->GetTrackPostion();
				m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->SetTrackPostion(fTrackPosition);
			}
		}
		if (m_curAnimClip.iActionAnimIdx != -1) {
			m_vecAnimations[m_curAnimClip.iActionAnimIdx]->SetLoop(m_curAnimClip.bIsActionAnimLoop);
		}

		m_fBlendTime = 0.f;
	}

	//Blending
	auto fAnimationTime = fDeltaTime * m_fAnimationSpeed;
	m_fBlendTime += fAnimationTime;
	m_fBlendTime = min(m_ClipTransitionDesc.fBlendDuration, m_fBlendTime);
	_float fRatio = m_fBlendTime / m_ClipTransitionDesc.fBlendDuration;
	fRatio = -2 * fRatio * fRatio * fRatio + 3 * fRatio * fRatio;

	list<ANIMNOTIFY_DESC> _listNotifies;
	pair<_float4, _float> deltaV;
	if (m_prevAnimClip.iLocomotionAnimIdx != -1 && fRatio > 0.f && fRatio < 1.f) {
		_matrix _curLocalA = ApplyLocomotionAnim(fAnimationTime, m_vecBones[BLENDA], m_prevAnimClip);
		ApplyLookPose(m_vecBones[BLENDA]);
		ApplyLocomotionAddtiveAnim(fAnimationTime, m_vecBones[BLENDA], m_curAnimClip);
		ApplyIk(m_vecBones[BLENDA], fDeltaTime);

		_matrix _curLocalB;
		if (m_curAnimClip.iLocomotionAnimIdx == m_prevAnimClip.iLocomotionAnimIdx)
		{
			_curLocalB = ApplyLocomotionAnim(0.f, m_vecBones[BLENDB], m_curAnimClip);
			ApplyLookPose(m_vecBones[BLENDB]);
			ApplyLocomotionAddtiveAnim(0.f, m_vecBones[BLENDB], m_curAnimClip);
			ApplyIk(m_vecBones[BLENDB], fDeltaTime);
		}

		else
		{
			_curLocalB = ApplyLocomotionAnim(fAnimationTime, m_vecBones[BLENDB], m_curAnimClip);
			ApplyLookPose(m_vecBones[BLENDB]);
			ApplyLocomotionAddtiveAnim(fAnimationTime, m_vecBones[BLENDB], m_curAnimClip);
			ApplyIk(m_vecBones[BLENDB], fDeltaTime);
		}

		_matrix _prevLocalA = XMLoadFloat4x4(&m_matPrevLocalRootA);
		_matrix _prevLocalB = XMLoadFloat4x4(&m_matPrevLocalRootB);

		_matrix _initDirA = XMLoadFloat4x4(&m_iPrevAnimationDir);
		_matrix _initDirB = XMLoadFloat4x4(&m_iCurAnimationDir);

		if (m_vecAnimations[m_prevAnimClip.iLocomotionAnimIdx]->IsEnd())	m_iPrevAnimationDir = GetModelDir();
		if (m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->IsEnd())		m_iCurAnimationDir = GetModelDir();


		pair<_float4, _float> deltaA = UpdateAnimationDeltaMatrix(m_prevAnimClip.iLocomotionAnimIdx, _prevLocalA, _curLocalA, _initDirA);
		pair<_float4, _float> deltaB = UpdateAnimationDeltaMatrix(m_curAnimClip.iLocomotionAnimIdx, _prevLocalB, _curLocalB, _initDirB);

		XMStoreFloat4x4(&m_matPrevLocalRootA, _curLocalA);
		XMStoreFloat4x4(&m_matPrevLocalRootB, _curLocalB);

		_listNotifies = m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->Update_Notifies();

		for (size_t i = 0; i < m_vecBones[MAIN].size(); i++) {
			if (m_iRootBoneIdx == i) continue;
			m_vecBones[MAIN][i]->BlendBone(m_vecBones[BLENDA][i], m_vecBones[BLENDB][i], fRatio);
		}

		switch (m_ClipTransitionDesc.eBlendType)
		{
		case Engine::CModel::CURRENT:
			deltaV = deltaB;
			break;

		case Engine::CModel::BLEND:

			deltaV.second = lerp(deltaA.second, deltaB.second, fRatio);
			deltaV.first.x = lerp(deltaA.first.x, deltaB.first.x, fRatio);
			deltaV.first.y = lerp(deltaA.first.y, deltaB.first.y, fRatio);
			deltaV.first.z = lerp(deltaA.first.z, deltaB.first.z, fRatio);
			deltaV.first.w = 1.f;
			break;
		case Engine::CModel::ZERO:
			deltaV.first = { 0.f,0.f,0.f,1.f };
			deltaV.second = 0.f;
			break;
		default:
			break;
		}
	}
	else {
		ResetPrevAnim();
		//Base Locomotion
		_matrix _curLocal;
		_curLocal = ApplyLocomotionAnim(fAnimationTime, m_vecBones[MAIN], m_curAnimClip);
		ApplyLookPose(m_vecBones[MAIN]);
		ApplyLocomotionAddtiveAnim(fAnimationTime, m_vecBones[MAIN], m_curAnimClip);
		ApplyIk(m_vecBones[MAIN], fDeltaTime);

		_matrix _prevLocal = XMLoadFloat4x4(&m_matPrevLocalRootB);

		_matrix _initDir = XMLoadFloat4x4(&m_iCurAnimationDir);
		if (m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->IsEnd()) m_iCurAnimationDir = GetModelDir();

		deltaV = UpdateAnimationDeltaMatrix(m_curAnimClip.iLocomotionAnimIdx, _prevLocal, _curLocal, _initDir);

		XMStoreFloat4x4(&m_matPrevLocalRootB, _curLocal);
		_listNotifies = m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->Update_Notifies();
	}

	//Calc RootMotion	
	_vector vTranslation = XMLoadFloat4(&deltaV.first);

	vTranslation = XMVectorMultiply(vTranslation, XMVectorSet(m_curAnimClip.fRootMotionScale.x, m_curAnimClip.fRootMotionScale.y, m_curAnimClip.fRootMotionScale.x, 1.f));
	m_pOwnerObj->Get_MainTransform().lock()->Translate(vTranslation, false);
	m_pOwnerObj->Get_MainTransform().lock()->Turn(XMVectorSet(0.f, -1.f, 0.f, 0.f), deltaV.second);

	for (auto pBone : m_vecBones[MAIN]) {
		pBone->Update_CombinedTransformationMatrix(
			m_vecBones[MAIN],
			m_pOwnerObj->Get_MainTransform().lock()->GetWorldMatrix()
		);
	}

	DEBUGRAY_DESC _desc;
	_desc.bNormalize = true;
	XMStoreFloat4(&_desc.ray.vRayDir, XMVector3TransformCoord(XMVectorSet(1.f, 0.f, 0.f, 1.f), XMLoadFloat4x4(&m_iCurAnimationDir)));
	XMStoreFloat4(&_desc.ray.vRayOrigin, m_pOwnerObj->Get_MainTransform().lock()->Get_LocalState(STATE::POSITION));
	m_pGameInstance->AddDebugRenderCall(&_desc);

	//AdjustBone
	//UpdateBoneConstraints();

	//Notify
	HandleNotifies(_listNotifies);
	Update_Listener(fDeltaTime);
}

void CModel::ResetPrevAnim()
{
	if (m_bIsResetPrevAnim) return;
	if (m_prevAnimClip.iLocomotionAnimIdx != -1 && m_prevAnimClip.iLocomotionAnimIdx != m_curAnimClip.iLocomotionAnimIdx) {
		m_vecAnimations[m_prevAnimClip.iLocomotionAnimIdx]->Reset();
	}
	if (m_prevAnimClip.iActionAnimIdx != -1 && m_prevAnimClip.iActionAnimIdx != m_curAnimClip.iActionAnimIdx) {
		m_vecAnimations[m_prevAnimClip.iActionAnimIdx]->Reset();
	}
	m_bIsResetPrevAnim = true;
}

void CModel::RenderActive(_bool IsActive)
{
	auto iNumMesh = Get_NumMeshes();
	for (size_t i = 0; i < iNumMesh; i++)
	{
		GetMeshRenderer((Engine::_int)i)->SetActive(IsActive);
	}
}

void CModel::Free()
{
	__super::Free();
	for (int i = 0; i < EBONEID::END; i++) {
		for (auto _pBone : m_vecBones[i]) Safe_Release(_pBone);
		m_vecBones[i].clear();
	}

	for (auto& [_, listener] : m_mapListener)
	{
		for (auto plistener : listener) Safe_Release(plistener);
		listener.clear();
	}
	m_mapListener.clear();

	for (auto _pAnimation : m_vecAnimations) Safe_Release(_pAnimation);
	m_vecAnimations.clear();
	m_vecAddtiveAnimIdx.clear();
}

SPComponent CModel::Clone(void* pArg)
{
	return CloneBase<CModel>(pArg);
}

void CModel::Render_Inspector()
{
	if (ImGui::Button("Refresh Notify"))
	{
		if (!m_vecAnimations.empty())
		{
			LoadAllNotifyConfigFromJson();
			ApplyAllNotifyConfigEntriesToRuntime();
		}
	}
	_bool bIsChange = false;
	Render_CameraBone();
	Render_ModelFilter();
	Render_ModelInfo();
	Render_AnimationInfo();
	int iIndex(0);
	for (auto _spMeshRenderer : m_vecSPMeshRenderers) {
		ImGui::PushID(iIndex++);
		ImGui::Text((to_string(iIndex - 1) + " : ").c_str());
		ImGui::SameLine();
		_spMeshRenderer->Render_Inspector();
		ImGui::PopID();
	}
	ImGui::RadioButton("None", reinterpret_cast<int*>(&m_desc.m_uiShadow), ENUM_TO_UINT(NONE));
	ImGui::SameLine();
	ImGui::RadioButton("Static", reinterpret_cast<int*>(&m_desc.m_uiShadow), ENUM_TO_UINT(STATIC));
	ImGui::SameLine();
	ImGui::RadioButton("Dynamic", reinterpret_cast<int*>(&m_desc.m_uiShadow), ENUM_TO_UINT(DYNAMIC));

	if (ImGui::CollapsingHeader("Constant Shader Param")) {
		_int iDeleteIdx = -1;
		if (ImGui::Button("Add")) {
			if (m_desc.m_iParamSize != 20) {
				m_desc.m_paramDescs[m_desc.m_iParamSize] = {};
				memcpy(m_desc.m_paramDescs[m_desc.m_iParamSize].sConstantName, ("Empty" + to_string(m_desc.m_iParamSize)).c_str(), 60);
				m_desc.m_iParamSize++;
			}
		}
		for (_int i = 0; i < m_desc.m_iParamSize; i++) {
			ImGui::PushID(i);
			auto _sName = "Param" + to_string(i);
			if (ImGui::CollapsingHeader(_sName.c_str())) {
				if (ImGui::DragInt("RendererIdx", &m_desc.m_paramDescs[i].iRendererIdx, 1, 0, m_iNumMeshes - 1))
					bIsChange = true;

				if (ImGui::RadioButton("bool", &m_desc.m_paramDescs[i].iType, 0)) bIsChange = true;
				ImGui::SameLine();
				if (ImGui::RadioButton("int", &m_desc.m_paramDescs[i].iType, 1)) bIsChange = true;
				ImGui::SameLine();
				if (ImGui::RadioButton("float", &m_desc.m_paramDescs[i].iType, 2)) bIsChange = true;
				ImGui::SameLine();
				if (ImGui::RadioButton("float2", &m_desc.m_paramDescs[i].iType, 4)) bIsChange = true;
				ImGui::SameLine();
				if (ImGui::RadioButton("float3", &m_desc.m_paramDescs[i].iType, 5)) bIsChange = true;
				ImGui::SameLine();
				if (ImGui::RadioButton("float4", &m_desc.m_paramDescs[i].iType, 3)) bIsChange = true;


				if (m_desc.m_paramDescs[i].iType == 0) {//bool
					if (ImGui::Checkbox("Data", reinterpret_cast<_bool*>(&m_desc.m_paramDescs[i].vData))) bIsChange = true;
				}
				else if (m_desc.m_paramDescs[i].iType == 1) {//int
					if (ImGui::DragInt("Data", reinterpret_cast<_int*>(&m_desc.m_paramDescs[i].vData), 0.01f)) bIsChange = true;
				}
				else if (m_desc.m_paramDescs[i].iType == 2) {//float
					if (ImGui::DragFloat("Data", reinterpret_cast<_float*>(&m_desc.m_paramDescs[i].vData), 0.01f))bIsChange = true;
				}
				else if (m_desc.m_paramDescs[i].iType == 4) {//float2
					if (ImGui::DragFloat2("Data", reinterpret_cast<_float*>(&m_desc.m_paramDescs[i].vData), 0.01f)) bIsChange = true;
				}
				else if (m_desc.m_paramDescs[i].iType == 5) {//float3
					if (ImGui::DragFloat3("Data", reinterpret_cast<_float*>(&m_desc.m_paramDescs[i].vData), 0.01f)) bIsChange = true;
				}
				else if (m_desc.m_paramDescs[i].iType == 3) {//float4
					if (ImGui::DragFloat4("Data", reinterpret_cast<_float*>(&m_desc.m_paramDescs[i].vData), 0.01f)) bIsChange = true;
				}
				if (ImGui::InputText("ConstantName", m_desc.m_paramDescs[i].sConstantName, 60)) {
					bIsChange = true;
				}
				if (ImGui::Button("Delete")) {
					iDeleteIdx = i;
					bIsChange = true;
				}
			}
			ImGui::PopID();
		}
		if (iDeleteIdx != -1) {
			m_desc.m_iParamSize -= 1;
			for (_int i = iDeleteIdx; i < 19; i++)
				m_desc.m_paramDescs[i] = m_desc.m_paramDescs[i + 1];
		}
	}

	if (ImGui::CollapsingHeader("Render State Param")) {
		_int iDeleteIdx = -1;
		if (ImGui::Button("Add")) {
			if (m_desc.m_iParamSize != 10) {
				m_desc.m_rsParamDescs[m_desc.m_iRSParamSize] = {};
				m_desc.m_iRSParamSize++;
			}
		}
		for (_int i = 0; i < m_desc.m_iRSParamSize; i++) {
			ImGui::PushID(i);
			auto _sName = "Param" + to_string(i);
			if (ImGui::CollapsingHeader(_sName.c_str())) {
				if (ImGui::DragInt("RendererIdx", &m_desc.m_rsParamDescs[i].iRendererIdx, 1, 0, m_iNumMeshes - 1))
					bIsChange = true;
				if (ImGui::DragInt("CullType", &m_desc.m_rsParamDescs[i].iCullType, 1, 0, 2))
					bIsChange = true;
				if (ImGui::DragInt("BlendType", &m_desc.m_rsParamDescs[i].iBlendType, 1, 0, 2))
					bIsChange = true;
				if (ImGui::Button("Delete")) {
					iDeleteIdx = i;
					bIsChange = true;
				}
			}
			ImGui::PopID();
		}
		if (iDeleteIdx != -1) {
			m_desc.m_iRSParamSize -= 1;
			for (_int i = iDeleteIdx; i < 9; i++)
				m_desc.m_rsParamDescs[i] = m_desc.m_rsParamDescs[i + 1];
		}
	}
	if (bIsChange) {
		Set_RenderParam();
	}
}

bool CModel::Check_BeatTransition()
{
	if (m_ClipTransitionDesc.iBeatTransit == 0) return true;

	for (_uint i = 0; i < 4; i++) {
		if (m_ClipTransitionDesc.iBeatTransit & (1 << i)) {
			if (m_pGameInstance->IsOnBeat({ _float(i) })) {
				return true;
			}
		}
	}

	return false;
}

void CModel::Set_RenderParam()
{
	for (size_t i = 0; i < m_vecSPMeshRenderers.size(); i++)
	{
		m_vecSPMeshRenderers[i]->Clear_ConstantParameters();
		m_vecSPMeshRenderers[i]->Clear_RSParameters();
	}
	for (size_t i = 0; i < m_desc.m_iParamSize; i++)
	{
		_int iRendererIdx = m_desc.m_paramDescs[i].iRendererIdx;
		if (iRendererIdx >= m_vecSPMeshRenderers.size()) continue;
		int iType = m_desc.m_paramDescs[i].iType;
		CRenderer::SHADERPARAM_DESC _desc;
		_desc.eParamType = CRenderer::EPARAMTYPE::PT_RAWDATA;
		if (iType == 0) _desc.iSize = sizeof(bool);
		else if (iType == 1) _desc.iSize = sizeof(int);
		else if (iType == 2) _desc.iSize = sizeof(float);
		else if (iType == 3) _desc.iSize = sizeof(_float4);
		else if (iType == 4) _desc.iSize = sizeof(_float2);
		else if (iType == 5) _desc.iSize = sizeof(_float3);
		_desc.sConstantName = m_desc.m_paramDescs[i].sConstantName;
		_desc.pData = &m_desc.m_paramDescs[i].vData;
		m_vecSPMeshRenderers[iRendererIdx]->Add_ConstantParameters(_desc);
	}
	for (size_t i = 0; i < m_desc.m_iRSParamSize; i++)
	{
		_int iRendererIdx = m_desc.m_rsParamDescs[i].iRendererIdx;
		if (iRendererIdx >= m_vecSPMeshRenderers.size()) continue;
		m_vecSPMeshRenderers[iRendererIdx]->Add_RSParameters(m_desc.m_rsParamDescs[i]);
	}
}

void CModel::Render_ModelFilter()
{
	/*if (ImGui::Button("Load")) {
		Ready_MeshRenderers();
	}*/
	vector<_wstring> _vecNames = m_pGameInstance->GetAssetNames(EASSETTYPE::MODEL);

	static ImGuiTextFilter filter;
	filter.Draw("Search");

	if (ImGui::IsItemActive()) m_bListOpen = true;
	if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) m_bListOpen = false;

	if (m_bListOpen) {
		for (auto& name : _vecNames)
		{
			auto _sName = ws2s(name);
			if (!filter.PassFilter(_sName.c_str()))
				continue;

			if (ImGui::Selectable(_sName.c_str()))
			{
				filter.Clear();
				strcpy_s(filter.InputBuf, _sName.c_str());
				filter.Build();
				lstrcpy(m_desc.m_sModelAssetName, s2ws(_sName).c_str());
				m_spModelAsset = nullptr;
				Load_Asset();
				Ready_MeshRenderers();
				for (auto _pMeshRenderer : m_vecSPMeshRenderers) {
					_pMeshRenderer->CheckLateInitialize();
				}
				Set_RenderParam();
				m_bListOpen = false;
			}
		}
	}
}

void CModel::Render_ModelInfo()
{
	wstring _sAssetName = m_desc.m_sModelAssetName;
	ImGui::Text("Name");
	ImGui::SameLine(200);
	ImGui::Text(ws2s(_sAssetName).c_str());

	ImGui::Text("Is Copy");
	ImGui::SameLine(200);
	ImGui::Text((m_desc.m_bIsCopy) ? "True" : "False");

	ImGui::Text("Num Mesh");
	ImGui::SameLine(200);
	ImGui::Text(to_string(m_iNumMeshes).c_str());

	ImGui::Text("Num Animations");
	ImGui::SameLine(200);
	ImGui::Text(to_string(m_iNumAnimations).c_str());
}

void CModel::Render_AnimationInfo()
{
	static int		m_iSelectedAnimIdx = -1;

	if (m_vecAnimations.empty()) return;
	m_curAnimClip.iLocomotionAnimIdx = min((Engine::_int)m_vecAnimations.size() - 1, m_curAnimClip.iLocomotionAnimIdx);

	if (m_curAnimClip.iLocomotionAnimIdx != -1) {
		wstring sAnimationName = m_vecAnimations[m_curAnimClip.iLocomotionAnimIdx]->GetDesc().m_sAnimationName;
		sAnimationName = L"Cur Anim :" + sAnimationName;
		ImGui::Text(ws2s(sAnimationName).c_str());
	}

	ImGui::BeginChild("Animation Inspector", ImVec2(0, 140), true);

	Render_AnimationCombo(&m_iSelectedAnimIdx, "Base Anim");
	if (m_iSelectedAnimIdx < m_vecAnimations.size()) {
		if (m_iSelectedAnimIdx != m_nextAnimClip.iLocomotionAnimIdx && m_iSelectedAnimIdx != m_prevAnimClip.iLocomotionAnimIdx)
			TransitAnimation(m_iSelectedAnimIdx, true, 0.2f);
	}

	ImGui::Separator();
	if (ImGui::Button("Stop")) { m_fAnimationSpeed = 0.f; }
	ImGui::SameLine();
	if (ImGui::Button("Start")) { m_fAnimationSpeed = 1.f; }

	ImGui::PushItemWidth(200.f);
	ImGui::SliderFloat("Animation Speed", &m_fAnimationSpeed, 0.1f, 2.5f, "x %.2f", ImGuiSliderFlags_ClampOnInput);
	ImGui::PopItemWidth();

	ImGui::Separator();

	if (m_iSelectedAnimIdx < m_vecAnimations.size()) { m_vecAnimations[m_iSelectedAnimIdx]->Render_Animation_Property(); }

	ImGui::EndChild();
}

_matrix CModel::BlendMatrix(_fmatrix _srcMatrix, _cmatrix _dstMatrix, _float fRatio)
{
	_vector _vSrcScale, _vSrcRotation, _vSrcTranslation;
	_vector _vDstScale, _vDstRotation, _vDstTranslation;
	XMMatrixDecompose(&_vSrcScale, &_vSrcRotation, &_vSrcTranslation, _srcMatrix);
	XMMatrixDecompose(&_vDstScale, &_vDstRotation, &_vDstTranslation, _dstMatrix);

	_vector vScale = XMVectorLerp(
		_vSrcScale,
		_vDstScale,
		fRatio
	);
	_vector vRotation = XMQuaternionSlerp(
		_vSrcRotation,
		_vDstRotation,
		fRatio
	);
	_vector vTranslation = XMVectorLerp(
		_vSrcTranslation,
		_vDstTranslation,
		fRatio
	);

	return (XMMatrixScalingFromVector(vScale) * XMMatrixRotationQuaternion(vRotation) * XMMatrixTranslationFromVector(vTranslation));
}
CBone* CModel::FindBoneWithName(const wstring& _boneName) {
	int idx = FindBoneIdxWithName(_boneName);
	if (idx == -1)	return nullptr;
	else			return m_vecBones[MAIN][idx];
}
CBone* CModel::FindBoneWithIdx(int iBoneIdx)
{
	if (iBoneIdx < 0 || iBoneIdx >= m_vecBones[MAIN].size()) return nullptr;
	return m_vecBones[MAIN][iBoneIdx];
}
HRESULT CModel::ClearMatchingBone()
{
	m_vecBoneMatchIndices.clear();
	return S_OK;
}

float CModel::GetAnimRatio(int idx)
{
	return m_vecAnimations[idx]->GetRatio();
}
float CModel::GetAnimRatio()
{
	if (m_curAnimClip.iLocomotionAnimIdx >= 0 && m_curAnimClip.iLocomotionAnimIdx < m_vecAnimations.size()) {
		return GetAnimRatio(m_curAnimClip.iLocomotionAnimIdx);
	}
	else {
		return 0.0f;
	}

}
void CModel::AnimReset(int idx)
{
	m_vecAnimations[idx]->Reset();
}
_int CModel::FindBoneIdxWithName(const wstring& _boneName)
{
	for (int i = 0; i < m_vecBones[MAIN].size(); i++) {
		if (m_vecBones[MAIN][i]->GetDesc().m_sName == _boneName) return i;
	}
	return -1;
}

_float4x4 CModel::GetModelDir()
{
	_uint rootParentIdx = m_vecBones[MAIN][m_iRootBoneIdx]->GetDesc().m_iParentBoneIndex;
	_float4x4 rootParentWorld = m_vecBones[MAIN][rootParentIdx]->GetDesc().m_CombinedTransformationMatrix;
	_matrix curWorld = XMLoadFloat4x4(&rootParentWorld);
	_vector			vCurScale, vCurRotation, vCurTranslation;
	XMMatrixDecompose(&vCurScale, &vCurRotation, &vCurTranslation, curWorld);
	_matrix resultMat = XMMatrixAffineTransformation(vCurScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vCurRotation, XMVectorSet(0.f, 0.f, 0.f, 1.f));

	XMStoreFloat4x4(&rootParentWorld, resultMat);

	return rootParentWorld;
}

HRESULT CModel::Snap(class CBone* pBone, int iPivotBoneIdx, _bool bLocationOnly)
{
	auto _targetDesc = pBone->GetDesc();
	auto _pivotDesc = m_vecBones[MAIN][iPivotBoneIdx]->GetDesc();
	//auto _RootDesc		= m_vecBones[MAIN][m_iRootBoneIdx]->GetDesc();

	_vector vTargetScale, vTargetRotation, vTargetTranslation;
	//_vector vPivotSrtScale, vPivotSrtRotation, vPivotSrtTranslation;
	_vector vPivotDstScale, vPivotDstRotation, vPivotDstTranslation;
	//_vector vScale, vRotation, vTranslation;
	_vector vTranslation;

	XMMatrixDecompose(&vTargetScale, &vTargetRotation, &vTargetTranslation, XMLoadFloat4x4(&_targetDesc.m_CombinedTransformationMatrix));
	//XMMatrixDecompose(&vPivotSrtScale,	&vPivotSrtRotation, &vPivotSrtTranslation,	XMLoadFloat4x4(&_RootDesc.	m_CombinedTransformationMatrix));
	XMMatrixDecompose(&vPivotDstScale, &vPivotDstRotation, &vPivotDstTranslation, XMLoadFloat4x4(&_pivotDesc.m_CombinedTransformationMatrix));


	//vScale			= vPivotSrtScale;
	//vRotation		= vPivotSrtRotation;
	//vTranslation	= vTargetTranslation + vPivotSrtTranslation - vPivotDstTranslation;
	vTranslation = vTargetTranslation - vPivotDstTranslation;
	if (!bLocationOnly) {
	}
	vTranslation = XMVectorSetW(vTranslation, 1.f);
	GetGameObject()->Get_MainTransform().lock()->Translate(vTranslation, false);;

	return S_OK;
}

void CModel::SetRootMotionScale(float fXScale, float fYScale)
{
	m_curAnimClip.fRootMotionScale.x = fXScale;
	m_curAnimClip.fRootMotionScale.y = fYScale;
}

_vector CModel::GetBoneWorldPos(const vector<CBone*>& vecBones, _int idx)
{
	auto _CombineMatrix = vecBones[idx]->GetDesc().m_CombinedTransformationMatrix;

	return XMVectorSet(_CombineMatrix._41, _CombineMatrix._42, _CombineMatrix._43, 1.f);
}

_matrix CModel::GetBoneCombineMatrix(const vector<CBone*>& vecBones, _int idx)
{
	auto _CombineMatrix = vecBones[idx]->GetDesc().m_CombinedTransformationMatrix;

	return XMLoadFloat4x4(&_CombineMatrix);
}

_matrix CModel::GetParentCombinedMatrix(const vector<CBone*>& vecBones, _int idx)
{
	auto _ParentIdx = vecBones[idx]->GetDesc().m_iParentBoneIndex;
	auto _ParentMatrix = vecBones[_ParentIdx]->GetDesc().m_CombinedTransformationMatrix;

	return XMLoadFloat4x4(&_ParentMatrix);
}

void CModel::SetBoneLocalMatrix(const vector<CBone*>& vecBones, _int idx, _matrix matLocal)
{
	auto _Bone = vecBones[idx];
	_Bone->SetAnimMatrix(matLocal, false);
}

void CModel::SetBoneWorldRotation(const vector<CBone*>& vecBones, _int idx, _vector qworldRot)
{
	auto _desc = vecBones[idx]->GetDesc();

	_vector vLocalScale, vLocalRot, vLocalTranslation;
	_matrix matLocal = XMLoadFloat4x4(&_desc.m_TransformationMatrix);
	XMMatrixDecompose(&vLocalScale, &vLocalRot, &vLocalTranslation, matLocal);

	_matrix matParentCombined = GetParentCombinedMatrix(vecBones, idx);

	_vector vParentScale, vParentRot, vParentTranslation;
	XMMatrixDecompose(&vParentScale, &vParentRot, &vParentTranslation, matParentCombined);

	_vector qLocalRot = XMQuaternionMultiply(XMQuaternionInverse(vParentRot), qworldRot);
	qLocalRot = XMQuaternionNormalize(qLocalRot);

	_matrix matNewLocal =
		XMMatrixScalingFromVector(vLocalScale) *
		XMMatrixRotationQuaternion(qLocalRot) *
		XMMatrixTranslationFromVector(vLocalTranslation);

	SetBoneLocalMatrix(vecBones, idx, matNewLocal);

}

_bool CModel::CheckIkChain(const vector<CBone*>& vecBones, _int rootIdx, _int midIdx, _int tipIdx) const
{
	if (rootIdx < 0 || midIdx < 0 || tipIdx < 0)
		return false;

	if (rootIdx >= vecBones.size() || midIdx >= vecBones.size() || tipIdx >= vecBones.size())
		return false;

	auto rootDesc = vecBones[rootIdx]->GetDesc();
	auto midDesc = vecBones[midIdx]->GetDesc();
	auto tipDesc = vecBones[tipIdx]->GetDesc();

	if (midDesc.m_iParentBoneIndex != rootDesc.m_iBoneIndex)
		return false;

	if (tipDesc.m_iParentBoneIndex != midDesc.m_iBoneIndex)
		return false;

	return true;
}

void CModel::ReCalcCombined(const vector<CBone*>& vecBones, _int startIdx)
{
	_int endIdx = m_vecBoneLastChildrenIdx[startIdx];
	for (_int i = startIdx; i <= endIdx; ++i)
	{
		vecBones[i]->Update_CombinedTransformationMatrix(vecBones,
			m_pOwnerObj->Get_MainTransform().lock()->GetWorldMatrix());
	}
}

_bool CModel::SolveIK(const vector<CBone*>& vecBones, _int rootIdx, _int midIdx, _int tipIdx, _vector vTargetPos, _vector vPolePos, _float fDeltaTime)
{
	if (!CheckIkChain(vecBones, rootIdx, midIdx, tipIdx))
		return false;

	auto SafeNormalize = [](_vector v, _vector vFallback) -> _vector
		{
			if (XMVectorGetX(XMVector3LengthSq(v)) < 0.000001f)
				return XMVector3Normalize(vFallback);
			return XMVector3Normalize(v);
		};

	auto ClampFloat = [](_float v, _float minV, _float maxV) -> _float
		{
			return max(minV, min(maxV, v));
		};

	auto QuaternionBetweenVectors = [&SafeNormalize, &ClampFloat](_vector vFrom, _vector vTo) -> _vector
		{
			_vector from = SafeNormalize(vFrom, XMVectorSet(1.f, 0.f, 0.f, 0.f));
			_vector to = SafeNormalize(vTo, XMVectorSet(1.f, 0.f, 0.f, 0.f));

			_float dot = XMVectorGetX(XMVector3Dot(from, to));
			dot = ClampFloat(dot, -1.f, 1.f);

			if (dot > 0.9999f)
				return XMQuaternionIdentity();

			if (dot < -0.9999f)
			{
				_vector axis = XMVector3Cross(from, XMVectorSet(0.f, 1.f, 0.f, 0.f));
				if (XMVectorGetX(XMVector3LengthSq(axis)) < 0.0001f)
					axis = XMVector3Cross(from, XMVectorSet(1.f, 0.f, 0.f, 0.f));

				axis = XMVector3Normalize(axis);
				return XMQuaternionRotationAxis(axis, XM_PI);
			}

			_vector axis = XMVector3Normalize(XMVector3Cross(from, to));
			_float angle = acosf(dot);
			return XMQuaternionRotationAxis(axis, angle);
		};

	auto GetWorldRotation = [&](int idx) -> _vector
		{
			_matrix mat = GetBoneCombineMatrix(vecBones, idx);
			_vector s, r, t;
			XMMatrixDecompose(&s, &r, &t, mat);
			return XMQuaternionNormalize(r);
		};

	auto ClampQuaternionStep = [&ClampFloat](_vector qCurrent, _vector qTarget, _float maxAngleRad) -> _vector
		{
			qCurrent = XMQuaternionNormalize(qCurrent);
			qTarget = XMQuaternionNormalize(qTarget);

			_float dot = XMVectorGetX(XMVector4Dot(qCurrent, qTarget));

			// shortest path
			if (dot < 0.f)
			{
				qTarget = XMVectorNegate(qTarget);
				dot = -dot;
			}

			dot = ClampFloat(dot, -1.f, 1.f);

			_float angle = 2.f * acosf(dot);

			// dead zone : 너무 작은 변화는 무시
			if (angle < XMConvertToRadians(0.5f))
				return qCurrent;

			if (angle <= maxAngleRad)
				return qTarget;

			_float t = maxAngleRad / angle;
			t = ClampFloat(t, 0.f, 1.f);

			return XMQuaternionNormalize(XMQuaternionSlerp(qCurrent, qTarget, t));
		};


	if (!m_bInitIKSmoothedTarget)
	{
		m_vIKSmoothedTargetPos = vTargetPos;
		m_bInitIKSmoothedTarget = true;
	}

	_float targetLerpT = ClampFloat(m_fIKTargetFollowSpeed * fDeltaTime, 0.f, 1.f);
	m_vIKSmoothedTargetPos = XMVectorLerp(m_vIKSmoothedTargetPos, vTargetPos, targetLerpT);
	m_vIKSmoothedTargetPos = XMVectorSetW(m_vIKSmoothedTargetPos, 1.f);

	_vector vSmoothedTargetPos = m_vIKSmoothedTargetPos;


	_vector vRootPos = GetBoneWorldPos(vecBones, rootIdx);
	_vector vMidPos = GetBoneWorldPos(vecBones, midIdx);
	_vector vTipPos = GetBoneWorldPos(vecBones, tipIdx);

	_float upperLen = XMVectorGetX(XMVector3Length(vMidPos - vRootPos));
	_float lowerLen = XMVectorGetX(XMVector3Length(vTipPos - vMidPos));

	if (upperLen < 0.0001f || lowerLen < 0.0001f)
		return false;


	_vector vToTarget = vSmoothedTargetPos - vRootPos;
	_float distToTarget = XMVectorGetX(XMVector3Length(vToTarget));

	_float maxReach = upperLen + lowerLen - m_fIKStraightMargin;
	maxReach = max(0.001f, maxReach);

	distToTarget = ClampFloat(distToTarget, 0.001f, maxReach);

	_vector dirToTarget = SafeNormalize(vToTarget, XMVectorSet(1.f, 0.f, 0.f, 0.f));


	_vector vPoleDir = vPolePos - vRootPos;
	_vector poleProjected = vPoleDir - XMVector3Dot(vPoleDir, dirToTarget) * dirToTarget;

	_float poleLenSq = XMVectorGetX(XMVector3LengthSq(poleProjected));

	_vector bendAxis;
	if (poleLenSq < 0.0001f)
	{
		if (m_bHasLastIKBendAxis)
		{
			bendAxis = m_vLastIKBendAxis;
		}
		else
		{
			// fallback
			_vector fallbackUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
			bendAxis = SafeNormalize(fallbackUp - XMVector3Dot(fallbackUp, dirToTarget) * dirToTarget, XMVectorSet(1.f, 0.f, 0.f, 0.f));
			m_vLastIKBendAxis = bendAxis;
			m_bHasLastIKBendAxis = true;
		}
	}
	else
	{
		poleProjected = XMVector3Normalize(poleProjected);
		_vector bendNormal = XMVector3Normalize(XMVector3Cross(dirToTarget, poleProjected));
		bendAxis = XMVector3Normalize(XMVector3Cross(bendNormal, dirToTarget));

		m_vLastIKBendAxis = bendAxis;
		m_bHasLastIKBendAxis = true;
	}

	// =========================================================
	// 4. desired mid 위치 계산
	// =========================================================
	_float cosRoot = (upperLen * upperLen + distToTarget * distToTarget - lowerLen * lowerLen) / (2.f * upperLen * distToTarget);
	cosRoot = ClampFloat(cosRoot, -1.f, 1.f);

	_float rootAngle = acosf(cosRoot);
	_float projLen = cosRoot * upperLen;
	_float bendLen = sinf(rootAngle) * upperLen;

	_vector vDesiredMidPos =
		vRootPos +
		dirToTarget * projLen +
		bendAxis * bendLen;

	// =========================================================
	// 5. root 회전 계산
	// =========================================================
	_vector curRootToMid = SafeNormalize(vMidPos - vRootPos, XMVectorSet(1.f, 0.f, 0.f, 0.f));
	_vector dstRootToMid = SafeNormalize(vDesiredMidPos - vRootPos, XMVectorSet(1.f, 0.f, 0.f, 0.f));

	_vector qRootWorldCurrent = GetWorldRotation(rootIdx);
	_vector qRootDelta = QuaternionBetweenVectors(curRootToMid, dstRootToMid);
	_vector qRootWorldTarget = XMQuaternionNormalize(XMQuaternionMultiply(qRootDelta, qRootWorldCurrent));

	// IK weight
	_vector qRootWeighted = XMQuaternionNormalize(
		XMQuaternionSlerp(qRootWorldCurrent, qRootWorldTarget, m_fIKWeight)
	);

	// 프레임당 최대 회전량 clamp
	_float maxRootAngleThisFrame = m_fIKMaxRootTurnSpeed * fDeltaTime;
	_vector qRootWorldFinal = ClampQuaternionStep(qRootWorldCurrent, qRootWeighted, maxRootAngleThisFrame);

	SetBoneWorldRotation(vecBones, rootIdx, qRootWorldFinal);
	ReCalcCombined(vecBones, rootIdx);

	// =========================================================
	// 6. root 적용 후 다시 읽기
	// =========================================================
	vRootPos = GetBoneWorldPos(vecBones, rootIdx);
	vMidPos = GetBoneWorldPos(vecBones, midIdx);
	vTipPos = GetBoneWorldPos(vecBones, tipIdx);

	// =========================================================
	// 7. mid 회전 계산
	// =========================================================
	_vector curMidToTip = SafeNormalize(vTipPos - vMidPos, XMVectorSet(1.f, 0.f, 0.f, 0.f));
	_vector dstMidToTip = SafeNormalize(vSmoothedTargetPos - vMidPos, XMVectorSet(1.f, 0.f, 0.f, 0.f));

	_vector qMidWorldCurrent = GetWorldRotation(midIdx);
	_vector qMidDelta = QuaternionBetweenVectors(curMidToTip, dstMidToTip);
	_vector qMidWorldTarget = XMQuaternionNormalize(XMQuaternionMultiply(qMidDelta, qMidWorldCurrent));

	// IK weight
	_vector qMidWeighted = XMQuaternionNormalize(
		XMQuaternionSlerp(qMidWorldCurrent, qMidWorldTarget, m_fIKWeight)
	);

	// 프레임당 최대 회전량 clamp
	_float maxMidAngleThisFrame = m_fIKMaxMidTurnSpeed * fDeltaTime;
	_vector qMidWorldFinal = ClampQuaternionStep(qMidWorldCurrent, qMidWeighted, maxMidAngleThisFrame);

	SetBoneWorldRotation(vecBones, midIdx, qMidWorldFinal);
	ReCalcCombined(vecBones, midIdx);

	return true;
}

void CModel::SetDashAfterimage(Engine::_bool bDash)
{
	for (const auto& it : m_vecSPMeshRenderers)
		it->SetDashAfterimage(bDash);
}

void CModel::SetKaleAfterimage(Engine::_bool bDash)
{
	for (const auto& it : m_vecSPMeshRenderers)
		it->SetKaleAfterimage(bDash);
}

void CModel::SetExplosion(Engine::_bool bExplosion)
{
	for (const auto& it : m_vecSPMeshRenderers)
		it->SetKaleAfterimage(bExplosion);
}

void CModel::UpdateBoneConstraints()
{
	for (const auto& _desc : m_vecBoneMatchIndices)
	{
		if (!_desc.m_bIsActive) continue;
		int _srcIdx(0), _dstIdx(0);
		if (_desc.m_iConstraintType == 0) {
			BoneMatch(_desc.m_iTargetBoneIdx, _desc.m_iPivotSrcBoneIdx, _desc.m_iPivotDstBoneIdx, _desc.m_bUseLocationOnly);
			_srcIdx = _desc.m_iPivotSrcBoneIdx;
			_dstIdx = m_vecBoneLastChildrenIdx[_srcIdx];
		}
		else if (_desc.m_iConstraintType == 1) {
			BoneScale(_desc.m_iTargetBoneIdx, _desc.m_fScalingFactor);
			_srcIdx = _desc.m_iTargetBoneIdx;
			_dstIdx = m_vecBoneLastChildrenIdx[_srcIdx];
		}
		else if (_desc.m_iConstraintType == 2) {
			BoneMatch(_desc.m_pBone, _desc.m_iPivotSrcBoneIdx, _desc.m_iPivotDstBoneIdx, _desc.m_bUseLocationOnly);
			_srcIdx = _desc.m_iPivotSrcBoneIdx;
			_dstIdx = m_vecBoneLastChildrenIdx[_srcIdx];
		}
		for (size_t i = _srcIdx + 1; i < _dstIdx; i++)
		{
			m_vecBones[MAIN][i]->Update_CombinedTransformationMatrix(
				m_vecBones[MAIN],
				m_pOwnerObj->Get_MainTransform().lock()->GetWorldMatrix()
			);
		}
	}
}

void CModel::Render_CameraBone()
{
	static CBone* pBone = nullptr;
	string  _sItem = "None Selected";
	if (pBone) {
		_sItem = ws2s(wstring(pBone->GetDesc().m_sName));
	}

	if (ImGui::BeginCombo("CameraBone", _sItem.c_str()))
	{
		_bool _bIsDirty = false;
		bool isSelected = (pBone == nullptr);
		if (ImGui::Selectable("None Selected", isSelected)) {
			pBone = nullptr;
			_bIsDirty = true;
		}
		for (int i = 0; i < m_vecBones[MAIN].size(); ++i)
		{
			bool isSelected = (m_vecBones[MAIN][i] == pBone);
			string  _sItem = ws2s(wstring(m_vecBones[MAIN][i]->GetDesc().m_sName));
			if (ImGui::Selectable(_sItem.c_str(), isSelected)) {
				pBone = m_vecBones[MAIN][i];
				_bIsDirty = true;
			}
		}
		ImGui::EndCombo();

		if (_bIsDirty) {
			auto lstCamera = m_pGameInstance->Get_Objects(
				m_pGameInstance->GetLevelDesc().nextLevelTag,
				L"Layer_Camera"
			);

			if (lstCamera.size()) {
				static_cast<CCamera*>(lstCamera.front())->SetMinwhanBone(pBone);
			}
		}
	}


	return;
}

void CModel::Render_CameraBone(class CBone*& pBone)
{
	string  _sItem = "None Selected";
	if (pBone) {
		_sItem = ws2s(wstring(pBone->GetDesc().m_sName));
	}

	if (ImGui::BeginCombo("CameraBone", _sItem.c_str()))
	{
		_bool _bIsDirty = false;
		bool isSelected = (pBone == nullptr);
		if (ImGui::Selectable("None Selected", isSelected)) {
			pBone = nullptr;
			_bIsDirty = true;
		}
		for (int i = 0; i < m_vecBones[MAIN].size(); ++i)
		{
			bool isSelected = (m_vecBones[MAIN][i] == pBone);
			string  _sItem = ws2s(wstring(m_vecBones[MAIN][i]->GetDesc().m_sName));
			if (ImGui::Selectable(_sItem.c_str(), isSelected)) {
				pBone = m_vecBones[MAIN][i];
			}
		}
		ImGui::EndCombo();
	}


	return;
}

void CModel::AnimSet_RhythmTarget(_float fTargetRatio)
{
	if (m_curAnimClip.iLocomotionAnimIdx == -1)
		return;

	_int curAnimIdx = m_curAnimClip.iLocomotionAnimIdx;

	auto pAnim = m_vecAnimations[curAnimIdx];
	if (!pAnim)
		return;

	_float duration = pAnim->GetDesc().m_fDuration;
	_float baseTick = pAnim->GetDesc().m_fTickPerSecond;

	if (duration <= 0.f || baseTick <= 0.f)
		return;

	_float bpm = m_pGameInstance->GetBPM();
	if (bpm <= 0.f)
		return;

	_float baseSpeed = bpm / 96.f;

	_float curRatio = pAnim->GetTrackPostion() / duration;
	curRatio = clamp(curRatio, 0.f, 1.f);

	_float targetRatio = clamp(fTargetRatio, 0.f, 1.f);

	if (m_iRhythmCalculatedAnimIdx == curAnimIdx)
	{
		if (m_bRhythmSyncActive)
		{
			if (curRatio < m_fRhythmTargetRatio)
			{
				pAnim->SetSpeed(m_fRhythmSpeedCoeff);
			}
			else
			{
				_float returnRange = 0.06f;

				_float t = (curRatio - m_fRhythmTargetRatio) / returnRange;
				t = clamp(t, 0.f, 1.f);

				// SmoothStep
				t = t * t * (3.f - 2.f * t);

				_float speed = lerp(m_fRhythmSpeedCoeff, baseSpeed, t);
				pAnim->SetSpeed(speed);

				if (t >= 1.f)
				{
					pAnim->SetSpeed(baseSpeed);
					m_bRhythmSyncActive = false;
					m_fRhythmSpeedCoeff = baseSpeed;
				}
			}
		}

		return;
	}

	if (curRatio > 0.02f)
	{
		m_iRhythmCalculatedAnimIdx = curAnimIdx;
		return;
	}

	if (targetRatio <= curRatio)
	{
		pAnim->SetSpeed(baseSpeed);
		m_bRhythmSyncActive = false;
		m_fRhythmSpeedCoeff = baseSpeed;
		m_iRhythmCalculatedAnimIdx = curAnimIdx;
		return;
	}

	_float currentBeat = m_pGameInstance->GetElapsedBeat();
	_float secPerBeat = 60.f / bpm;

	_float remainRatio = targetRatio - curRatio;
	_float remainTick = remainRatio * duration;

	_float baseTime = remainTick / baseTick;
	_float predictedBeat = currentBeat + baseTime / secPerBeat;

	_float targetBeat = roundf(predictedBeat);

	if (targetBeat <= currentBeat)
		targetBeat = ceilf(currentBeat);

	_float targetTime = (targetBeat - currentBeat) * secPerBeat;

	if (targetTime <= 0.f)
		return;

	_float needTick = remainTick / targetTime;
	_float speedCoeff = needTick / baseTick;

	m_fRhythmTargetRatio = targetRatio;
	m_fRhythmSpeedCoeff = clamp(speedCoeff, 0.05f, 10.f);
	m_bRhythmSyncActive = true;

	pAnim->SetSpeed(m_fRhythmSpeedCoeff);

	m_iRhythmCalculatedAnimIdx = curAnimIdx;
}

HRESULT CModel::AddListener(wstring _tag, function<void(_float)> funcUpdate, function<void(void)> funcEnter, function<void(void)> funcExit, _bool isOnce)
{
	m_mapListener[_tag];
	auto it = m_mapListener.find(_tag);
	it->second.push_back(CListener::Create(funcUpdate, funcEnter, funcExit, isOnce));

	return S_OK;
}

HRESULT CModel::HandleNotifies(list<ANIMNOTIFY_DESC> _listNotifies)
{
	for (auto& _notify : _listNotifies)
	{
		auto it = m_mapListener.find(wstring(_notify.m_sName));
		if (it != m_mapListener.end())
		{
			for (auto& _listener : it->second)
			{
				if (_notify.m_bIsStart)
					_listener->Activate(_notify.m_fDuration);
				else
					_listener->Deactivate();
			}
		}
	}



	return S_OK;
}

void CModel::Update_Listener(_float fTimeDelta)
{
	for (auto& [_, _listeners] : m_mapListener)
	{
		auto it = _listeners.begin();
		while (it != _listeners.end())
		{
			if ((*it)->Update(fTimeDelta))
			{
				Safe_Release(*it);
				it = _listeners.erase(it);
			}
			else
				it++;
		}
	}
}

HRESULT CModel::AddNotify(_wstring _animName, ANIMNOTIFY_DESC& _desc)
{
	int _idx = FindAnimationClipIdxWithName(_animName);
	if (_idx < 0 || _idx >= m_vecAnimations.size()) return E_FAIL;
	m_vecAnimations[_idx]->AddNotify(_desc);

	return S_OK;
}

void CModel::Add_Notify(_float fInitframe, _float fEndFrame, _float fTotalFrame, _wstring AnimTag, _wstring NotifyTag)
{
	ANIMNOTIFY_DESC _desc{};
	lstrcpy(_desc.m_sName, NotifyTag.c_str());
	_desc.m_fDuration = 0;

	_desc.m_bIsStart = true;
	_desc.m_fPosition = fInitframe / fTotalFrame;
	AddNotify(AnimTag, _desc);

	_desc.m_bIsStart = false;
	_desc.m_fPosition = fEndFrame / fTotalFrame;
	AddNotify(AnimTag, _desc);
}

void CModel::Add_ImpulseNotify(_float fTargetFrame, _float fTotalFrame, _wstring AnimTag, _wstring NotifyTag)
{
	ANIMNOTIFY_DESC _desc{};
	lstrcpy(_desc.m_sName, NotifyTag.c_str());
	_desc.m_fDuration = 0.01f;
	_desc.m_bIsStart = true;
	_desc.m_fPosition = fTargetFrame / fTotalFrame;

	AddNotify(AnimTag, _desc);
}

void CModel::Add_SFXNotify(_float fTargetFrame, _float fTotalFrame, _wstring AnimTag, _wstring SFXTag, _float fVolume, _int iPriority, _float fEndPoint)
{
	ANIMNOTIFY_DESC _desc{};
	lstrcpy(_desc.m_sName, SFXTag.c_str());
	_desc.m_fDuration = 0.01f;
	_desc.m_bIsStart = true;
	_desc.m_fPosition = fTargetFrame / fTotalFrame;
	AddNotify(AnimTag, _desc);

	AddListener(SFXTag, nullptr, [=]() { m_pGameInstance->Play_SFX(SFXTag, fVolume, iPriority, fEndPoint); }, nullptr, false);
}

void CModel::Add_VFXNotify(_float fTargetFrame, _float fTotalFrame, _wstring AnimTag, _wstring VFXTag)
{
	ANIMNOTIFY_DESC _desc{};
	lstrcpy(_desc.m_sName, VFXTag.c_str());
	_desc.m_fDuration = 0.01f;
	_desc.m_bIsStart = true;
	_desc.m_fPosition = fTargetFrame / fTotalFrame;
	AddNotify(AnimTag, _desc);

	// VFX 전용 Notify는 여기서 런타임 Notify만 추가한다.
	// 실제 VFX 생성은 기존 방식처럼 AddListener(VFXTag, ...)를 외부에서 연결하거나,
	// 프로젝트의 VFX 재생 함수가 확정되면 이곳에 연결하면 된다.
}

bool CModel::Render_AnimationCombo(int* idx, string sName, bool bExcludePose)
{
	static char searchBuf[128] = "";
	ImGui::InputText("Search", searchBuf, IM_ARRAYSIZE(searchBuf));

	string  _sItem;
	int prevIdx = *idx;
	if (*idx < m_vecAnimations.size()) {
		_sItem = ws2s(wstring(m_vecAnimations[*idx]->GetDesc().m_sAnimationName));
	}
	else {
		_sItem = "None Selected";
	}

	ImGui::PushID(*idx);
	if (ImGui::BeginCombo(sName.c_str(), _sItem.c_str()))
	{
		bool isSelected = (*idx == -1);
		string  _sItem = "None";
		if (ImGui::Selectable(_sItem.c_str(), isSelected)) {
			*idx = -1;
		}

		for (int i = 0; i < m_vecAnimations.size(); ++i)
		{
			string  _sItem = ws2s(wstring(m_vecAnimations[i]->GetDesc().m_sAnimationName));

			if (strlen(searchBuf) > 0)
			{
				auto _slowerName = _sItem;
				transform(_slowerName.begin(), _slowerName.end(), _slowerName.begin(), ::tolower);
				if (strstr(_slowerName.c_str(), searchBuf) == nullptr) continue;
			}

			bool isSelected = (*idx == i);
			if (ImGui::Selectable(_sItem.c_str(), isSelected)) {
				*idx = i;
			}

			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
	return (prevIdx != *idx);
}
const char* CModel::NotifyTypeToString(NotifyType eType) const
{
	switch (eType)
	{
	case NT_ANIM:
		return "Anim";

	case NT_VFX:
		return "VFX";

	case NT_SFX:
		return "SFX";

	default:
		return "Anim";
	}
}

_wstring CModel::GetNotifyJsonPath(NotifyType eType) const
{
	namespace fs = std::filesystem;

	wstring sModelName = m_desc.m_sModelAssetName;
	if (sModelName.empty())
		return L"";

	string sFileName = ws2s(sModelName);
	for (char& ch : sFileName)
	{
		switch (ch)
		{
		case '<': case '>': case ':': case '"': case '/': case '\\': case '|': case '?': case '*':
			ch = '_';
			break;
		default:
			break;
		}
	}

	string sSuffix;
	switch (eType)
	{
	case NT_ANIM:
		sSuffix = "";
		break;

	case NT_VFX:
		sSuffix = "_VFX";
		break;

	case NT_SFX:
		sSuffix = "_SFX";
		break;

	default:
		sSuffix = "";
		break;
	}

	fs::path path = fs::path("../../Client/Notifies/") / (sFileName + sSuffix + ".json");
	return path.wstring();
}


void CModel::SortNotifyConfigEntries(NotifyType eType)
{
	auto& vecEntries = m_vecNotifyConfigEntries[eType];

	sort(vecEntries.begin(), vecEntries.end(),
		[](const NOTIFY_CONFIG_ENTRY& a, const NOTIFY_CONFIG_ENTRY& b)
		{
			if (a.sAnimTag != b.sAnimTag)
				return a.sAnimTag < b.sAnimTag;

			if (a.iType != b.iType)
				return a.iType < b.iType;

			const float fA = (a.iType == NOTIFY_CONFIG_BASIC) ? a.fStartRatio : a.fTargetRatio;
			const float fB = (b.iType == NOTIFY_CONFIG_BASIC) ? b.fStartRatio : b.fTargetRatio;

			if (fabs(fA - fB) > 0.0001f)
				return fA < fB;

			if (a.sNotifyTag != b.sNotifyTag)
				return a.sNotifyTag < b.sNotifyTag;

			return a.fEndRatio < b.fEndRatio;
		});
}


void CModel::SortAllNotifyConfigEntries()
{
	for (_int i = NT_ANIM; i <= NT_SFX; ++i)
		SortNotifyConfigEntries((NotifyType)i);
}

void CModel::ClearRuntimeNotifyData()
{
	for (auto* pAnimation : m_vecAnimations)
	{
		if (pAnimation)
			pAnimation->Get_Notifies().clear();
	}
}

void CModel::ApplyAnimNotifyEntry(const NOTIFY_CONFIG_ENTRY& entry)
{
	switch (entry.iType)
	{
	case NOTIFY_CONFIG_BASIC:
		Add_Notify(entry.fStartRatio, entry.fEndRatio, 1.f, entry.sAnimTag, entry.sNotifyTag);
		break;

	case NOTIFY_CONFIG_IMPULSE:
		Add_ImpulseNotify(entry.fTargetRatio, 1.f, entry.sAnimTag, entry.sNotifyTag);
		break;

	case NOTIFY_CONFIG_SFX:
		Add_SFXNotify(entry.fTargetRatio, 1.f, entry.sAnimTag, entry.sNotifyTag, entry.fVolume, entry.iPriority, entry.fEndPoint);
		break;
	}
}

void CModel::ApplyVFXNotifyEntry(const NOTIFY_CONFIG_ENTRY& entry)
{
	Add_VFXNotify(entry.fTargetRatio, 1.f, entry.sAnimTag, entry.sNotifyTag);
}

void CModel::ApplySFXNotifyEntry(const NOTIFY_CONFIG_ENTRY& entry)
{
	Add_SFXNotify(entry.fTargetRatio, 1.f, entry.sAnimTag, entry.sNotifyTag, entry.fVolume, entry.iPriority, entry.fEndPoint);
}

void CModel::ApplyAllNotifyConfigEntriesToRuntime()
{
	ClearRuntimeNotifyData();

	for (_int iType = NT_ANIM; iType <= NT_SFX; ++iType)
	{
		NotifyType eType = (NotifyType)iType;
		SortNotifyConfigEntries(eType);

		const auto& vecEntries = m_vecNotifyConfigEntries[eType];
		for (const auto& entry : vecEntries)
		{
			if (entry.sAnimTag.empty() || entry.sNotifyTag.empty())
				continue;

			switch (eType)
			{
			case NT_ANIM:
				ApplyAnimNotifyEntry(entry);
				break;

			case NT_VFX:
				ApplyVFXNotifyEntry(entry);
				break;

			case NT_SFX:
				ApplySFXNotifyEntry(entry);
				break;
			}
		}
	}
}



void CModel::LoadNotifyConfigFromJson(NotifyType eType)
{
	auto& vecEntries = m_vecNotifyConfigEntries[eType];
	vecEntries.clear();

	wstring sPath = GetNotifyJsonPath(eType);
	if (sPath.empty())
		return;

	if (!std::filesystem::exists(std::filesystem::path(sPath)))
	{
		SyncNotifyConfigEntriesFromRuntime(eType);
		return;
	}

	ifstream ifs(std::filesystem::path(sPath), ios::binary);
	if (!ifs.is_open())
	{
		SyncNotifyConfigEntriesFromRuntime(eType);
		return;
	}

	Json::Value root;
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	string errs;

	if (!Json::parseFromStream(builder, ifs, &root, &errs))
	{
		SyncNotifyConfigEntriesFromRuntime(eType);
		return;
	}

	if (!root.isObject() || !root.isMember("Notifies") || !root["Notifies"].isArray())
	{
		SyncNotifyConfigEntriesFromRuntime(eType);
		return;
	}

	const Json::Value& notifies = root["Notifies"];
	for (Json::ArrayIndex i = 0; i < notifies.size(); ++i)
	{
		const Json::Value& item = notifies[i];
		if (!item.isObject())
			continue;

		NOTIFY_CONFIG_ENTRY entry{};
		entry.sAnimTag = s2ws(item.isMember("AnimTag") ? item["AnimTag"].asString() : "");

		string sType = item.isMember("Type") ? item["Type"].asString() : "Notify";

		if (sType == "ImpulseNotify" || sType == "ImpurseNotify")
			entry.iType = NOTIFY_CONFIG_IMPULSE;
		else if (sType == "SFXNotify")
			entry.iType = NOTIFY_CONFIG_SFX;
		else
			entry.iType = NOTIFY_CONFIG_BASIC;

		const Json::Value& desc = item["Desc"];

		if (eType == NT_ANIM)
		{
			if (entry.iType == NOTIFY_CONFIG_BASIC)
			{
				entry.sNotifyTag = s2ws(desc.isMember("NotifyTag") ? desc["NotifyTag"].asString() : "");
				entry.fStartRatio = desc.isMember("StartRatio") ? desc["StartRatio"].asFloat() : 0.f;
				entry.fEndRatio = desc.isMember("EndRatio") ? desc["EndRatio"].asFloat() : entry.fStartRatio;
			}
			else if (entry.iType == NOTIFY_CONFIG_SFX)
			{
				entry.sNotifyTag = s2ws(desc.isMember("SFXTag") ? desc["SFXTag"].asString() : "");
				entry.fTargetRatio = desc.isMember("TargetRatio") ? desc["TargetRatio"].asFloat() : 0.f;
				entry.fVolume = desc.isMember("Volume") ? desc["Volume"].asFloat() : 1.f;
				entry.iPriority = desc.isMember("Priority") ? desc["Priority"].asInt() : 0;
				entry.fEndPoint = desc.isMember("EndPoint") ? desc["EndPoint"].asFloat() : 50.f;
			}
			else
			{
				entry.sNotifyTag = s2ws(desc.isMember("NotifyTag") ? desc["NotifyTag"].asString() : "");
				entry.fTargetRatio = desc.isMember("TargetRatio") ? desc["TargetRatio"].asFloat() : 0.f;
			}
		}
		else if (eType == NT_VFX)
		{
			entry.iType = NOTIFY_CONFIG_IMPULSE;

			if (desc.isMember("VFXTag"))
				entry.sNotifyTag = s2ws(desc["VFXTag"].asString());
			else
				entry.sNotifyTag = s2ws(desc.isMember("NotifyTag") ? desc["NotifyTag"].asString() : "");

			entry.fTargetRatio = desc.isMember("TargetRatio") ? desc["TargetRatio"].asFloat() : 0.f;
		}
		else if (eType == NT_SFX)
		{
			entry.iType = NOTIFY_CONFIG_SFX;

			if (desc.isMember("SFXTag"))
				entry.sNotifyTag = s2ws(desc["SFXTag"].asString());
			else
				entry.sNotifyTag = s2ws(desc.isMember("NotifyTag") ? desc["NotifyTag"].asString() : "");

			entry.fTargetRatio = desc.isMember("TargetRatio") ? desc["TargetRatio"].asFloat() : 0.f;
			entry.fVolume = desc.isMember("Volume") ? desc["Volume"].asFloat() : 1.f;
			entry.iPriority = desc.isMember("Priority") ? desc["Priority"].asInt() : 0;
			entry.fEndPoint = desc.isMember("EndPoint") ? desc["EndPoint"].asFloat() : 50.f;
		}

		if (!entry.sAnimTag.empty() && !entry.sNotifyTag.empty())
			vecEntries.push_back(entry);
	}

	SortNotifyConfigEntries(eType);
}


void CModel::LoadAllNotifyConfigFromJson()
{
	for (_int i = NT_ANIM; i <= NT_SFX; ++i)
		LoadNotifyConfigFromJson((NotifyType)i);
}

void CModel::SaveNotifyConfigToJson(NotifyType eType) const
{
	wstring sPath = GetNotifyJsonPath(eType);
	if (sPath.empty())
		return;

	std::filesystem::path path(sPath);
	std::filesystem::create_directories(path.parent_path());

	Json::Value root(Json::objectValue);
	root["ModelName"] = ws2s(m_desc.m_sModelAssetName);
	root["NotifyType"] = NotifyTypeToString(eType);
	root["Notifies"] = Json::arrayValue;

	const auto& vecEntries = m_vecNotifyConfigEntries[eType];
	for (const auto& entry : vecEntries)
	{
		Json::Value item(Json::objectValue);
		item["AnimTag"] = ws2s(entry.sAnimTag);

		Json::Value desc(Json::objectValue);

		if (eType == NT_ANIM)
		{
			if (entry.iType == NOTIFY_CONFIG_BASIC)
			{
				item["Type"] = "Notify";
				desc["NotifyTag"] = ws2s(entry.sNotifyTag);
				desc["StartRatio"] = entry.fStartRatio;
				desc["EndRatio"] = entry.fEndRatio;
			}
			else if (entry.iType == NOTIFY_CONFIG_SFX)
			{
				item["Type"] = "SFXNotify";
				desc["SFXTag"] = ws2s(entry.sNotifyTag);
				desc["TargetRatio"] = entry.fTargetRatio;
				desc["Volume"] = entry.fVolume;
				desc["Priority"] = entry.iPriority;
				desc["EndPoint"] = entry.fEndPoint;
			}
			else
			{
				item["Type"] = "ImpulseNotify";
				desc["NotifyTag"] = ws2s(entry.sNotifyTag);
				desc["TargetRatio"] = entry.fTargetRatio;
			}
		}
		else if (eType == NT_VFX)
		{
			item["Type"] = "VFXNotify";
			desc["VFXTag"] = ws2s(entry.sNotifyTag);
			desc["TargetRatio"] = entry.fTargetRatio;
		}
		else if (eType == NT_SFX)
		{
			item["Type"] = "SFXNotify";
			desc["SFXTag"] = ws2s(entry.sNotifyTag);
			desc["TargetRatio"] = entry.fTargetRatio;
			desc["Volume"] = entry.fVolume;
			desc["Priority"] = entry.iPriority;
			desc["EndPoint"] = entry.fEndPoint;
		}

		item["Desc"] = desc;
		root["Notifies"].append(item);
	}

	ofstream ofs(path, ios::binary | ios::trunc);
	if (!ofs.is_open())
		return;

	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "  ";
	string output = Json::writeString(builder, root);
	ofs.write(output.c_str(), static_cast<std::streamsize>(output.size()));
}


void CModel::SaveAllNotifyConfigToJson() const
{
	for (_int i = NT_ANIM; i <= NT_SFX; ++i)
		SaveNotifyConfigToJson((NotifyType)i);
}

void CModel::SyncNotifyConfigEntriesFromRuntime(NotifyType eType)
{
	auto& vecEntries = m_vecNotifyConfigEntries[eType];
	vecEntries.clear();

	// 새로 추가한 VFX/SFX 전용 json이 없으면 빈 상태로 둔다.
	// 기존 런타임 Animation Notify에서 자동 복구하는 것은 기존 Anim Notify 파일만 대상으로 한다.
	if (eType != NT_ANIM)
		return;

	for (int iAnimIdx = 0; iAnimIdx < (int)m_vecAnimations.size(); ++iAnimIdx)
	{
		const wstring sAnimTag = m_vecAnimations[iAnimIdx]->GetDesc().m_sAnimationName;
		auto& vecNotifies = m_vecAnimations[iAnimIdx]->Get_Notifies();
		vector<bool> vecUsed(vecNotifies.size(), false);

		for (size_t i = 0; i < vecNotifies.size(); ++i)
		{
			if (vecUsed[i])
				continue;

			const ANIMNOTIFY_DESC& cur = vecNotifies[i];
			if (!cur.m_bIsStart)
				continue;

			NOTIFY_CONFIG_ENTRY entry{};
			entry.sAnimTag = sAnimTag;
			entry.sNotifyTag = cur.m_sName;

			int iMatchedEndIdx = -1;
			for (size_t j = i + 1; j < vecNotifies.size(); ++j)
			{
				if (vecUsed[j])
					continue;

				const ANIMNOTIFY_DESC& candidate = vecNotifies[j];
				if (!candidate.m_bIsStart && wstring(candidate.m_sName) == wstring(cur.m_sName))
				{
					iMatchedEndIdx = (int)j;
					break;
				}
			}

			if (iMatchedEndIdx != -1)
			{
				entry.iType = NOTIFY_CONFIG_BASIC;
				entry.fStartRatio = cur.m_fPosition;
				entry.fEndRatio = vecNotifies[iMatchedEndIdx].m_fPosition;
				vecUsed[iMatchedEndIdx] = true;
			}
			else
			{
				entry.iType = NOTIFY_CONFIG_IMPULSE;
				entry.fTargetRatio = cur.m_fPosition;
			}

			vecUsed[i] = true;
			vecEntries.push_back(entry);
		}
	}

	SortNotifyConfigEntries(eType);
}


void CModel::Activate_Aditive(_int _animIdx)
{
	if (_animIdx == -1 || _animIdx >= m_vecAnimations.size())
	{
		_animIdx = (Engine::_int)Random(0ull, m_vecAddtiveAnimIdx.size() - 1);

	}
	if (m_vecAddtiveAnimIdx.size())
	{
		m_iAddtiveAnimIdx = m_vecAddtiveAnimIdx[_animIdx];
		m_bIsOnAddtiveAnim = true;
	}
}

void CModel::DeActivate_Aditive()
{
	m_bIsOnAddtiveAnim = false;
	m_iAddtiveAnimIdx = -1;
	m_bIsAddtiveAnimFirstFrame = false;
}



void CModel::Render_BoneCombo(int* idx)
{
	string  _sItem;
	if (*idx < m_vecBones[MAIN].size()) {
		_sItem = ws2s(wstring(m_vecBones[MAIN][*idx]->GetDesc().m_sName));
	}
	else {
		_sItem = "None Selected";
	}

	if (ImGui::BeginCombo("Bone", _sItem.c_str()))
	{
		bool isSelected = (*idx == -1);
		string  _sItem = "None";
		if (ImGui::Selectable(_sItem.c_str(), isSelected)) {
			*idx = -1;
		}

		for (int i = 0; i < m_vecBones[MAIN].size(); ++i)
		{
			bool isSelected = (*idx == i);
			string  _sItem = ws2s(wstring(m_vecBones[MAIN][i]->GetDesc().m_sName));
			if (ImGui::Selectable(_sItem.c_str(), isSelected)) {
				*idx = i;
			}

			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void CModel::SetCurrentDir()
{
	m_iCurAnimationDir = GetModelDir();
}

HRESULT CModel::AddMatchingBone(CBone* pBone, const wstring& PivotSrtBoneName, const wstring& PivotDstBoneName, bool bIsLocationOnly)
{
	BONECONSTRAINT_DESC _desc;
	int pivotSrcIdx = FindBoneIdxWithName(PivotSrtBoneName);
	int pivotDstIdx = FindBoneIdxWithName(PivotDstBoneName);
	if (pivotSrcIdx < 0 || pivotDstIdx < 0) return E_FAIL;

	_desc.m_iConstraintType = 2;
	_desc.m_bUseLocationOnly = bIsLocationOnly;
	_desc.m_pBone = pBone;
	_desc.m_iPivotSrcBoneIdx = pivotSrcIdx;
	_desc.m_iPivotDstBoneIdx = pivotDstIdx;

	m_vecBoneMatchIndices.push_back(_desc);
	return S_OK;
}

HRESULT CModel::AddMatchingBone(const wstring& targetBoneName, const wstring& PivotSrcBoneName, const wstring& PivotDstBoneName, bool bIsLocationOnly)
{
	BONECONSTRAINT_DESC _desc;
	int targetIdx = FindBoneIdxWithName(targetBoneName);
	int pivotSrcIdx = FindBoneIdxWithName(PivotSrcBoneName);
	int pivotDstIdx = FindBoneIdxWithName(PivotDstBoneName);
	if (targetIdx < 0 || pivotSrcIdx < 0 || pivotDstIdx < 0) return E_FAIL;

	_desc.m_iConstraintType = 0;
	_desc.m_bUseLocationOnly = bIsLocationOnly;
	_desc.m_iTargetBoneIdx = targetIdx;
	_desc.m_iPivotSrcBoneIdx = pivotSrcIdx;
	_desc.m_iPivotDstBoneIdx = pivotDstIdx;

	m_vecBoneMatchIndices.push_back(_desc);
	return S_OK;
}

HRESULT CModel::AddBoneScale(const wstring& targetBoneName, float fScalingFactor)
{
	BONECONSTRAINT_DESC _desc;
	int targetIdx = FindBoneIdxWithName(targetBoneName);
	if (targetIdx < 0 || fScalingFactor < 0) return E_FAIL;

	_desc.m_iConstraintType = 1;
	_desc.m_iTargetBoneIdx = targetIdx;
	_desc.m_fScalingFactor = fScalingFactor;

	m_vecBoneMatchIndices.push_back(_desc);
	return S_OK;
}

void CModel::SetIdentityRefBones()
{
	for (auto pBone : m_vecBones[REF])
	{
		pBone->SetAnimMatrix(XMMatrixIdentity());
	}
}

_matrix CModel::ApplyLocomotionAnim(float fDeltaTime, const vector<CBone*>& vecBones, const ANIMCLIP_DESC& animClipDesc)
{
	return m_vecAnimations[animClipDesc.iLocomotionAnimIdx]->Update_TransformationMatrices(fDeltaTime, vecBones);
}

void CModel::ApplyLookPose(const vector<CBone*>& vecBones)
{
	if (!m_bUseLook)
		return;

	m_fLookRatio.x = clamp(m_fLookRatio.x, -1.f, 1.f);
	m_fLookRatio.y = clamp(m_fLookRatio.y, -1.f, 1.f);

	_float _wx[3] = { max(-m_fLookRatio.x, 0), 1.f - abs(m_fLookRatio.x), max(m_fLookRatio.x, 0) };
	_float _wy[3] = { max(-m_fLookRatio.y, 0), 1.f - abs(m_fLookRatio.y), max(m_fLookRatio.y, 0) };
	for (size_t i = 0; i < 3; i++) for (size_t j = 0; j < 3; j++)  m_fLookingWeights[i][j] = _wy[i] * _wx[j];



	for (size_t i = 0; i < 3; i++) for (size_t j = 0; j < 3; j++) {
		_int _iLookingPoseIdx = m_arrLookingPoseIndices[i][j];   //0 - up , 1 - down , 2 - Left, 3 - Right , 4 - UL ,5 - UR, 6 - DL , 7 - DR
		if (_iLookingPoseIdx == -1) continue;
		if (m_fLookingWeights[i][j] > 0.0001f)
			m_vecAnimations[_iLookingPoseIdx]->Update_Additive_TransformationMatrices(0.f, vecBones, m_vecBones[REF], m_fLookingWeights[i][j]);
	}
}

void CModel::ApplyLocomotionAddtiveAnim(float fDeltaTime, const vector<CBone*>& vecBones, const ANIMCLIP_DESC& animClipDesc)
{
	// 실행중인 Animation - AddtiveAnimation = delta, 이 델타를 다시 Animation에 더한다.

	//원하는 Animation = 실행중인 Animation + delta animation * weight
	//delta animation = Additive Animation - RefPose(Additive Anim의 첫 프레임)
	if (m_iAddtiveAnimIdx == -1 || !m_bIsOnAddtiveAnim)
		return;
	if (!m_bIsAddtiveAnimFirstFrame)
	{
		m_bIsAddtiveAnimFirstFrame = true;
		m_vecAnimations[m_iAddtiveAnimIdx]->Reset();
	}

	m_vecAnimations[m_iAddtiveAnimIdx]->Update_Additive_TransformationMatrices(fDeltaTime, vecBones, m_vecBones[REF], 1.f);
}

void CModel::ApplyIk(const vector<CBone*>& vecBones, _float fTimeDelta)
{
	if (!m_bUseIK)
		return;

	SolveIK(vecBones, m_iIkRootBoneIdx, m_iIkMidBoneIdx, m_iIkTipBoneIdx, m_vIKTargetPos, m_vIKPolePos, fTimeDelta);
}

HRESULT CModel::ActiveMatchingBone(_int idx, bool bIsActive)
{
	if (m_vecBoneMatchIndices.size() <= idx) return E_FAIL;
	m_vecBoneMatchIndices[idx].m_bIsActive = bIsActive;
	return S_OK;
}

HRESULT CModel::TransitAnimationClip(const ANIMCLIP_DESC& NxtAnimClip, const CLIPTRANSIT_DESC& NxtAnimTransition)
{
	m_nextAnimClip = NxtAnimClip;
	memcpy(&m_ClipTransitionDesc, &NxtAnimTransition, sizeof CLIPTRANSIT_DESC);

	auto _bpm = m_pGameInstance->GetBPM();
	auto _offset = _bpm / 96;

	if (m_nextAnimClip.iLocomotionAnimIdx != -1)
		m_vecAnimations[m_nextAnimClip.iLocomotionAnimIdx]->SetSpeed(NxtAnimClip.fAnimationSpeed * _offset);
	return S_OK;
}
_bool	CModel::IsAnimationClipEnd(const ANIMCLIP_DESC& animClip) {
	if (animClip.iActionAnimIdx != -1) {
		return m_vecAnimations[animClip.iActionAnimIdx]->IsEnd();
	}
	else {
		return m_vecAnimations[animClip.iLocomotionAnimIdx]->IsEnd();
	}
}

_bool CModel::IsAnimEnd(_int idx) 
{
	return m_vecAnimations[idx]->IsEnd();
}

CAnimation* CModel::FindAnimationClipWithName(const wstring& animationName) {

	for (int i = 0; i < m_vecAnimations.size(); i++) {
		if (m_vecAnimations[i]->GetDesc().m_sAnimationName == animationName) return m_vecAnimations[i];
	}
	return nullptr;
}

_int		CModel::FindAnimationClipIdxWithName(const wstring& animationName) {

	for (int i = 0; i < m_vecAnimations.size(); i++) {
		wstring _sAnimName = m_vecAnimations[i]->GetDesc().m_sAnimationName;
		if (_sAnimName == animationName)
			return i;
	}
	return -1;
}

_int CModel::FindAddtiveAnimIdx(_int animIdx)
{
	auto animName = m_vecAnimations[animIdx]->GetDesc().m_sAnimationName;

	if (animName && (
			wcsstr(animName, L"add") != nullptr
		||	wcsstr(animName, L"em7510_Barm-chance-cp_060") != nullptr
		||	wcsstr(animName, L"em7520_Farm-chance-cp_060") != nullptr
		||	wcsstr(animName, L"em7530_Sarm-chance-cp-L_060") != nullptr
		||	wcsstr(animName, L"em7530_Sarm-chance-cp-R_060") != nullptr
	))
	{
		return FindAnimationClipIdxWithName(animName);
	}

	return -1;
}

_int CModel::FindRefPoseAnimIdx(_int animIdx)
{
	auto animName = m_vecAnimations[animIdx]->GetDesc().m_sAnimationName;

	if (animName && (
			wcsstr(animName, L"idle_000") != nullptr 
		||	wcsstr(animName, L"em7510_pose_000") != nullptr
		||	wcsstr(animName, L"em7520_pose_000") != nullptr
		||	wcsstr(animName, L"em7530_pose_000") != nullptr
	))
	{
		return FindAnimationClipIdxWithName(animName);
	}

	return -1;
}

_int CModel::FindLookAnimIdx(_int animIdx)
{
	auto animName = m_vecAnimations[animIdx]->GetDesc().m_sAnimationName;

	if (animName &&
		(wcsstr(animName, L"look_00") != nullptr ||
			wcsstr(animName, L"CP_Peppermint_gimmic_look_") != nullptr
			)
		)
	{
		return FindAnimationClipIdxWithName(animName);
	}

	return -1;
}


HRESULT CModel::BoneMatch(int iTargetBoneIdx, int iPivotSrcBoneIdx, int iPivotDstBoneIdx, _bool bLocationOnly)
{
	//CheckValid
	if (iTargetBoneIdx < 0 || iPivotSrcBoneIdx < 0 || iPivotDstBoneIdx < 0)										return E_FAIL; // �ε��� ��ȿ��
	if (iPivotSrcBoneIdx > iPivotDstBoneIdx || m_vecBoneLastChildrenIdx[iPivotSrcBoneIdx] < iPivotDstBoneIdx)	return E_FAIL; // dst�� �ڽ� ���� üũ
	if (iPivotSrcBoneIdx <= iTargetBoneIdx && m_vecBoneLastChildrenIdx[iPivotSrcBoneIdx] >= iTargetBoneIdx)	return E_FAIL; // Ÿ���� ���ڽ� ���� üũ

	auto _targetDesc = m_vecBones[MAIN][iTargetBoneIdx]->GetDesc();
	auto _pivotSrtDesc = m_vecBones[MAIN][iPivotSrcBoneIdx]->GetDesc();
	auto _pivotDstDesc = m_vecBones[MAIN][iPivotDstBoneIdx]->GetDesc();

	_vector vTargetScale, vTargetRotation, vTargetTranslation;
	_vector vPivotSrtScale, vPivotSrtRotation, vPivotSrtTranslation;
	_vector vPivotDstScale, vPivotDstRotation, vPivotDstTranslation;
	_vector vScale, vRotation, vTranslation;

	XMMatrixDecompose(&vTargetScale, &vTargetRotation, &vTargetTranslation, XMLoadFloat4x4(&_targetDesc.m_CombinedTransformationMatrix));
	XMMatrixDecompose(&vPivotSrtScale, &vPivotSrtRotation, &vPivotSrtTranslation, XMLoadFloat4x4(&_pivotSrtDesc.m_CombinedTransformationMatrix));
	XMMatrixDecompose(&vPivotDstScale, &vPivotDstRotation, &vPivotDstTranslation, XMLoadFloat4x4(&_pivotDstDesc.m_CombinedTransformationMatrix));


	vScale = vPivotSrtScale;
	vRotation = vPivotSrtRotation;
	vTranslation = vTargetTranslation + vPivotSrtTranslation - vPivotDstTranslation;
	if (!bLocationOnly) {
	}

	_matrix _matCombine = XMMatrixScalingFromVector(vScale) * XMMatrixRotationQuaternion(vRotation) * XMMatrixTranslationFromVector(vTranslation);
	m_vecBones[MAIN][iPivotSrcBoneIdx]->SetAnimMatrix(_matCombine, true);
	return S_OK;
}

HRESULT CModel::BoneMatch(CBone* pBone, int iPivotSrcBoneIdx, int iPivotDstBoneIdx, _bool bLocationOnly)
{
	if (iPivotSrcBoneIdx < 0 || iPivotDstBoneIdx < 0)										return E_FAIL; // �ε��� ��ȿ��
	if (iPivotSrcBoneIdx > iPivotDstBoneIdx || m_vecBoneLastChildrenIdx[iPivotSrcBoneIdx] < iPivotDstBoneIdx)	return E_FAIL; // dst�� �ڽ� ���� üũ

	auto _targetDesc = pBone->GetDesc();
	auto _pivotSrtDesc = m_vecBones[MAIN][iPivotSrcBoneIdx]->GetDesc();
	auto _pivotDstDesc = m_vecBones[MAIN][iPivotDstBoneIdx]->GetDesc();

	_vector vTargetScale, vTargetRotation, vTargetTranslation;
	_vector vPivotSrtScale, vPivotSrtRotation, vPivotSrtTranslation;
	_vector vPivotDstScale, vPivotDstRotation, vPivotDstTranslation;
	_vector vScale, vRotation, vTranslation;

	XMMatrixDecompose(&vTargetScale, &vTargetRotation, &vTargetTranslation, XMLoadFloat4x4(&_targetDesc.m_CombinedTransformationMatrix));
	XMMatrixDecompose(&vPivotSrtScale, &vPivotSrtRotation, &vPivotSrtTranslation, XMLoadFloat4x4(&_pivotSrtDesc.m_CombinedTransformationMatrix));
	XMMatrixDecompose(&vPivotDstScale, &vPivotDstRotation, &vPivotDstTranslation, XMLoadFloat4x4(&_pivotDstDesc.m_CombinedTransformationMatrix));


	vScale = vPivotSrtScale;
	vRotation = vPivotSrtRotation;
	vTranslation = vTargetTranslation + vPivotSrtTranslation - vPivotDstTranslation;
	if (!bLocationOnly) {
	}

	_matrix _matCombine = XMMatrixScalingFromVector(vScale) * XMMatrixRotationQuaternion(vRotation) * XMMatrixTranslationFromVector(vTranslation);
	m_vecBones[MAIN][iPivotSrcBoneIdx]->SetAnimMatrix(_matCombine, true);
	return S_OK;
}

HRESULT CModel::BoneScale(int iTargetBoneIdx, float fScalingFactor)
{
	if (iTargetBoneIdx < 0)										return E_FAIL;
	auto _targetDesc = m_vecBones[MAIN][iTargetBoneIdx]->GetDesc();
	_vector vTargetScale, vTargetRotation, vTargetTranslation;
	XMMatrixDecompose(&vTargetScale, &vTargetRotation, &vTargetTranslation, XMLoadFloat4x4(&_targetDesc.m_CombinedTransformationMatrix));
	vTargetScale = XMVectorScale(vTargetScale, fScalingFactor);
	_matrix _matCombine = XMMatrixScalingFromVector(vTargetScale) * XMMatrixRotationQuaternion(vTargetRotation) * XMMatrixTranslationFromVector(vTargetTranslation);
	m_vecBones[MAIN][iTargetBoneIdx]->SetAnimMatrix(_matCombine, true);

	return S_OK;
}

pair<_float4, _float> CModel::UpdateAnimationDeltaMatrix(int iAnimaionIdx, _fmatrix matPrevLocal, _cmatrix matCurLocal, _cmatrix matWorldDir)
{
	_vector	vDeltaScale, vDeltaRotation, vDeltaTranslation;
	XMMatrixDecompose(&vDeltaScale, &vDeltaRotation, &vDeltaTranslation, XMMatrixInverse(nullptr, matPrevLocal) * matCurLocal);

	vDeltaTranslation = XMVector3TransformCoord(vDeltaTranslation, matWorldDir);

	switch (m_vecAnimations[iAnimaionIdx]->GetDesc().m_iRootMotionType)
	{
	case CAnimation::EROOTMOTIONTYPE::NONE:
		vDeltaRotation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		vDeltaTranslation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		break;
	case CAnimation::EROOTMOTIONTYPE::INPLACE:
		vDeltaTranslation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		break;
	case CAnimation::EROOTMOTIONTYPE::DELTA:
		break;
	default:
		break;
	}

	if (m_vecAnimations[iAnimaionIdx]->IsEnd()) {
		switch (m_vecAnimations[iAnimaionIdx]->GetDesc().m_iLoopDeltaType) {
		case CAnimation::ELOOPDELTATYPE::DELTAIGNORE:
			vDeltaRotation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			vDeltaTranslation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			break;
		case CAnimation::ELOOPDELTATYPE::DELTAKEEP:
			break;
		case CAnimation::ELOOPDELTATYPE::DELTASNAP:
			vDeltaRotation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			vDeltaTranslation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			break;
		}
	}
	auto _transMat = XMMatrixTranslationFromVector(vDeltaTranslation);
	auto _rotMat = XMMatrixRotationQuaternion(vDeltaRotation);

	pair<_float4, _float> _delta;
	XMStoreFloat4(&_delta.first, vDeltaTranslation);

	XMVECTOR forward = XMVector3Rotate(
		XMVectorSet(0, 0, 1, 0),
		vDeltaRotation
	);

	// XZ 평면에 투영 (Y 제거)
	forward = XMVectorSetY(forward, 0);
	forward = XMVector3Normalize(forward);

	// yaw 계산
	_delta.second = atan2f(
		XMVectorGetX(forward),
		XMVectorGetZ(forward)
	);

	return _delta;
}

HRESULT CModel::Save(void* _pDesc, _uint& _iSize) const
{
	_iSize = sizeof(MODEL_DESC);
	static_assert(is_trivially_copyable_v<MODEL_DESC>);

	memcpy(_pDesc, &m_desc, _iSize);
	return S_OK;
}

HRESULT CModel::Load(void* _pDesc)
{
	m_desc = *reinterpret_cast<MODEL_DESC*>(_pDesc);
	//if (m_desc.m_uiShadow > 2)
	m_desc.m_uiShadow = { 1 };
	m_uiOldShadow = { m_desc.m_uiShadow };
	if (m_desc.m_iParamSize > 20 || m_desc.m_iParamSize < 0)
		m_desc.m_iParamSize = 0;
	if (m_desc.m_iRSParamSize > 10 || m_desc.m_iRSParamSize < 0)
		m_desc.m_iRSParamSize = 0;

	//Ready_MeshRenderers();
	return S_OK;
}

_bool CModel::PickingMesh(RAY InWorldRay, CPicking::PICKING_DESC* pDesc)
{
	_bool _bIsPicking = false;
	if (m_vecAnimations.size()) return false;
	for (auto pRenderer : m_vecSPMeshRenderers) {
		if (!pRenderer->IsActive()) continue;

		CPicking::PICKING_DESC _tmpDesc;
		if (pRenderer->RayCast(InWorldRay, &_tmpDesc)) {
			if (pDesc->fWorldDist > _tmpDesc.fWorldDist) {
				*pDesc = _tmpDesc;
				pDesc->wpRenderer = ConvertWPComponent<CRenderer>(pRenderer);
				pDesc->eHitType |= CPicking::MESH;
			}
			_bIsPicking = true;
		}
	}
	return _bIsPicking;
}

shared_ptr<class CMeshRenderer> CModel::GetMeshRenderer(_int idx)
{
	if (m_vecSPMeshRenderers.size() <= idx) return nullptr;
	return m_vecSPMeshRenderers[idx];
}

HRESULT CModel::TransitAnimation(int iNxtAnimIdx, bool bForceTransit, float fBlendDuaration, EBLENDINGDELTATYPE eBlendType)
{
	ANIMCLIP_DESC _nextAnimClip;
	_nextAnimClip.iLocomotionAnimIdx = iNxtAnimIdx;

	CLIPTRANSIT_DESC _transitDesc;
	_transitDesc.bForceTransit = bForceTransit;
	_transitDesc.eBlendType = eBlendType;
	_transitDesc.fBlendDuration = fBlendDuaration;

	return TransitAnimationClip(_nextAnimClip, _transitDesc);
}

wstring CModel::FindAnimTag(_uint iAnimIdx)
{
	if (iAnimIdx >= m_vecAnimations.size()) return L"Invalid Anim Idx";

	return m_vecAnimations[iAnimIdx]->GetDesc().m_sAnimationName;
}
