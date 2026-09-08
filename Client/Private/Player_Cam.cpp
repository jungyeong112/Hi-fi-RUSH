#include "Player_Cam.h"
#include "GameInstance.h"
#include "Player.h"
#include "Bone.h"

CPlayer_Cam::CPlayer_Cam(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera(pDevice, pContext)
{
}

CPlayer_Cam::CPlayer_Cam(const CPlayer_Cam& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CPlayer_Cam::Initialize_Prototype(void* pArg)
{
	CHKFAIL(__super::Initialize_Prototype(pArg))
		return S_OK;
}

HRESULT CPlayer_Cam::Initialize(void* pDesc)
{
	CHKFAIL(__super::Initialize(pDesc))
		m_upCamTransitTimer = m_pGameInstance->CreateTimerWithDuration(0.2f, false, nullptr, false);

	Load_Config();
	return S_OK;
}

HRESULT CPlayer_Cam::Late_Initialize()
{
	CHKFAIL(__super::Late_Initialize())
	SetCamPos(0.f);
	m_vCurrentPivotPos = m_vTargetPivotPos;
	return S_OK;
}

void CPlayer_Cam::SetCamPos(float dDeltaTime)
{
	m_fCameraLegth = Damp(m_fCameraLegth, m_fCameraTargetLegth, dDeltaTime, 20.f);
	if (abs(m_fCameraLegth - m_fCameraTargetLegth) < .1f) m_fCameraLegth = m_fCameraTargetLegth;

	_float _tmpV = 1.f;
	_vector _vCurrentPivotPos = Damp(XMLoadFloat4(&m_vCurrentPivotPos), XMLoadFloat4(&m_vTargetPivotPos), dDeltaTime, m_fCameraDamp * _tmpV);

	if (XMVectorGetX(XMVector3Length(_vCurrentPivotPos - XMLoadFloat4(&m_vTargetPivotPos))) < 1.f) _vCurrentPivotPos = XMLoadFloat4(&m_vTargetPivotPos);

	XMStoreFloat4(&m_vCurrentPivotPos, _vCurrentPivotPos);

	if (auto spTransform = m_defaultCamTransform.lock()) {
		spTransform->Set_Identity();
		_vector _vLookDir = XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMMatrixRotationX(m_fPitch) * XMMatrixRotationY(m_fYaw));

		auto fCameraLength = SetSpringCam(_vCurrentPivotPos, _vLookDir);

		_vector _vCurrentCamPos = _vCurrentPivotPos + XMVectorScale(_vLookDir, -fCameraLength);

		auto _vLook	= XMVector3Normalize(m_pPlayer->Get_MainTransform().lock()->Get_WorldState(STATE::LOOK));
		auto _vRight	= XMVector3Normalize(m_pPlayer->Get_MainTransform().lock()->Get_WorldState(STATE::RIGHT));

		spTransform->Set_LocalState(STATE::POSITION, _vCurrentCamPos);
		spTransform->Aim(_vLookDir);

		XMStoreFloat4(&m_vCameraDir, _vLookDir);
		XMStoreFloat4(&m_vCameraPos, _vCurrentCamPos);
		m_pPlayer->SetCameraDir(m_vCameraDir);
	}
}


void CPlayer_Cam::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (!m_bisHandle) 
	{
		SetCamAngle(fTimeDelta);
		SetCamOffset();
		SetCamPos(fTimeDelta);
		SetTargetPos();
		ApplyCameraEffect(fTimeDelta);
	}
	else 
	{
		ApplyCameraEffect(fTimeDelta);
	}
	

	Update_PipeLines();
}

void CPlayer_Cam::SetCamAngle(const Engine::_float& fTimeDelta)
{
	if (m_upCamTransitTimer->IsActive() || m_pTargetBone != nullptr) return;

	if (auto dx = m_pGameInstance->GetMouseMoveState(EMOUSEMOVE::X)) {
		m_fTargetYaw += dx * fTimeDelta * m_fCameraSensitivity;
		m_fTargetYaw = wrap(m_fTargetYaw, -XM_PI, XM_PI);
	}

	if (auto dy = m_pGameInstance->GetMouseMoveState(EMOUSEMOVE::Y)) {
		m_fTargetPitch += dy * fTimeDelta * m_fCameraSensitivity;
		m_fTargetPitch = clamp(m_fTargetPitch, -XM_PIDIV2 * m_fPitchLimit, XM_PIDIV2 * m_fPitchLimit);
	}

	auto _yawDist = abs(wrap(m_fYaw - m_fTargetYaw, -XM_PI, XM_PI));
	if (_yawDist < 2.f * XM_2PI / 360.f)  m_fYaw = m_fTargetYaw;
	else                                    m_fYaw = AngleDamp(m_fYaw, m_fTargetYaw, fTimeDelta, 10.f);


	auto _pitchDist = abs(wrap(m_fPitch - m_fTargetPitch, -XM_PI, XM_PI));
	if (_pitchDist < 1.f * XM_2PI / 360.f)  m_fPitch = m_fTargetPitch;
	else                                    m_fPitch = Damp(m_fPitch, m_fTargetPitch, fTimeDelta, 10.f);
}

void CPlayer_Cam::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	m_fCameraSensitivity = m_fCameraMeleeSensitivity;

}

void CPlayer_Cam::SetTargetPos()
{
	if (m_upCamTransitTimer->IsActive()) {
		if (m_pTargetBone) {
			auto _desc = m_pTargetBone->GetDesc();
			if (auto spTransformCom = m_targetTransform.lock()) {
				spTransformCom->Set_Identity();

				spTransformCom->Set_LocalState(STATE(0),
					XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[1]))
				);
				spTransformCom->Set_LocalState(STATE(1),
					-XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[2]))
				);
				spTransformCom->Set_LocalState(STATE(2),
					-XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[0]))
				);

				spTransformCom->Set_LocalState(STATE(3),
					XMLoadFloat4(
						reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[3])
					)
				);
				spTransformCom->Set_Scale(1.f, 1.f, 1.f);
			}
		}
		else {
			m_targetTransform.lock()->Copy_LocalMatrix(m_defaultCamTransform.lock()->GetWorldMatrix());
		}
		_float fRatio = (Engine::_float)m_upCamTransitTimer->GetElapsedRatio();
		fRatio = easeOutSine(fRatio);
		Get_MainTransform().lock()->Lerp_LocalState(m_prevTargetTransform, m_targetTransform, fRatio);
		SetFOV(lerp(m_fTargetFOV, m_fPrevTargetFOV, fRatio));
	}
	else {
		if (m_pTargetBone) {
			auto _desc = m_pTargetBone->GetDesc();
			if (auto spTransformCom = m_targetTransform.lock()) {
				spTransformCom->Set_Identity();
				
				spTransformCom->Set_LocalState(STATE(0),
					XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[1]))
				);
				spTransformCom->Set_LocalState(STATE(1),
					-XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[2]))
				);
				spTransformCom->Set_LocalState(STATE(2),
					-XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[0]))
				);
				
				spTransformCom->Set_LocalState(STATE(3),
					XMLoadFloat4(
						reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[3])
					)
				);
				spTransformCom->Set_Scale(1.f, 1.f, 1.f);
			}
		}
		else {
			m_targetTransform.lock()->Copy_LocalMatrix(m_defaultCamTransform.lock()->GetWorldMatrix());
		}
		Get_MainTransform().lock()->Copy_LocalState(m_targetTransform);
	}
}

void CPlayer_Cam::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

void CPlayer_Cam::SetCamOffset()
{
	_vector _vPlayerPosition = m_pPlayer->Get_MainTransform().lock()->Get_WorldState(STATE::POSITION);

	m_vTargetPivotPos.y = XMVectorGetY(_vPlayerPosition) + 2.f;
	m_vTargetPivotPos.x = XMVectorGetX(_vPlayerPosition);
	m_vTargetPivotPos.z = XMVectorGetZ(_vPlayerPosition);
}

_float CPlayer_Cam::SetSpringCam(_fvector vPivotPos, _fvector vLookDir)
{
	_float _fCameraLength = m_fCameraLegth;

	RAY vCameraRay;
	XMStoreFloat4(&vCameraRay.vRayOrigin,vPivotPos);
	XMStoreFloat4(&vCameraRay.vRayDir,	-vLookDir);
	
	_int iCollidorMask = 0;
	iCollidorMask |= (1 << ECollidorType::CT_COMMON_BODY);
	list<pair<_float, WPCollidor>> _lstCollidors;
	if (m_pGameInstance->RayCast_Collidor(iCollidorMask, vCameraRay, _fCameraLength, _lstCollidors)) {
		//auto [_fDist, _pCollidor] = *_lstCollidors.begin();

		_fCameraLength-= 0.2f;
		_fCameraLength = max(_fCameraLength, 0.2f);
	}
	
	return _fCameraLength;
}

CGameObject* CPlayer_Cam::Clone(void* pArg)
{
	return CloneBase<CPlayer_Cam>(pArg);
}

void CPlayer_Cam::Free()
{
	__super::Free();
}

HRESULT CPlayer_Cam::Reset()
{
	return S_OK;
}

HRESULT CPlayer_Cam::Save(void* _pDesc, _uint& _iSize) const
{
	_iSize = 0;
	return S_OK;
}

HRESULT CPlayer_Cam::Load(void* _pDesc)
{
	return S_OK;
}

HRESULT CPlayer_Cam::Add_InitComponents()
{
	CHKFAIL(__super::Add_InitComponents())

	Add_Component<CTransform>(L"Com_Transform_PrevTarget");
	auto spPrevTargetTransform = dynamic_pointer_cast<CTransform>(Get_Component(L"Com_Transform_PrevTarget").lock());
	spPrevTargetTransform->SetInitialAdd();
	m_prevTargetTransform = spPrevTargetTransform;

	Add_Component<CTransform>(L"Com_Transform_Target");
	auto spTargetTransform = dynamic_pointer_cast<CTransform>(Get_Component(L"Com_Transform_Target").lock());
	spTargetTransform->SetInitialAdd();
	m_targetTransform = spTargetTransform;

	Add_Component<CTransform>(L"Com_Transform_DefaultCam");
	auto spDefaultCamTransform = dynamic_pointer_cast<CTransform>(Get_Component(L"Com_Transform_DefaultCam").lock());
	spDefaultCamTransform->SetInitialAdd();
	m_defaultCamTransform = spDefaultCamTransform;

	return S_OK;
}

HRESULT CPlayer_Cam::Load_Config()
{
	/* m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.BaseCamera.fFovy",   &m_desc.fFovy);
	 m_desc.fFovy *= XM_PI / 180.f;
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.BaseCamera.fNear",   &m_desc.fNear);
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.BaseCamera.fFar",    &m_desc.fFar);

	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fPitchLimit",        &m_fPitchLimit);
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fCameraDamp",        &m_fCameraDamp);
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fAimCameraLegth",    &m_fAimCameraLegth);
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fNonAImCameraLegth", &m_fNonAimCameraLegth);
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fShieldCameraLegth", &m_fShieldCameraLegth);

	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fCameraMeleeSensitivity",   &m_fCameraMeleeSensitivity  );
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fCameraAimSensitivity",     &m_fCameraAimSensitivity    );

	 vector<_float> vecOffset;
	 m_pGameInstance->ReadArraySubConfig(L"PlayerDataBase.json", L"Camera.vAimCamOffset", &vecOffset);
	 m_vAimCamOffset.x = vecOffset[0];
	 m_vAimCamOffset.y = vecOffset[1];
	 m_vAimCamOffset.z = vecOffset[2];

	 m_pGameInstance->ReadArraySubConfig(L"PlayerDataBase.json", L"Camera.vGrabCamOffset", &vecOffset);
	 m_vGrabCamOffset.x = vecOffset[0];
	 m_vGrabCamOffset.y = vecOffset[1];
	 m_vGrabCamOffset.z = vecOffset[2];

	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fInjectFOV", &m_fInjectFOV);
	 m_fInjectFOV *= XM_PI / 180.f;
	 m_pGameInstance->ReadSubConfig(L"PlayerDataBase.json", L"Camera.fNormalFOV", &m_fNormalFOV);
	 m_fNormalFOV *= XM_PI / 180.f;*/

	m_desc.fFovy = XMConvertToRadians(60.f);
	m_desc.fNear = 0.1875f;
	m_desc.fFar  = 5000.f;
	m_fYaw = 0.f;
	m_fTargetYaw = 0.f;
	m_fPitch = XMConvertToRadians(30.f);
	m_fTargetPitch = XMConvertToRadians(30.f);
	m_fPitchLimit = XMConvertToRadians(75.f);
	m_vCameraDir = { 0.f, -0.3f, 1.f, 0.f };
	m_fCameraDamp = 2000.f;
	m_fCameraTargetLegth = 3.5f;

	m_fCameraMeleeSensitivity = 0.12f;
	m_fCameraAimSensitivity = 0.07f;
	m_fCameraSensitivity = m_fCameraMeleeSensitivity;
	m_vTargetPivotPos = { 0.f, 0.5f, 0.f, 1.f };

	m_vCameraPos =
	{
		m_vCurrentPivotPos.x,
		m_vCurrentPivotPos.y,
		m_vCurrentPivotPos.z - m_fCameraLegth,
		1.f
	};



	return S_OK;
}

void CPlayer_Cam::StartCamShake(_float fPower, _float fDuration, _float fFrequence)
{
	if (fPower <= 0 || fDuration <= 0) return;

	m_bIsCamShake     = true;
	m_fShakePower     = fPower;
	m_fShakeElapsed   = 0.f;
	m_fShakeFrequency = fFrequence;
	m_fShakeDuration  = fDuration;
	m_fShakePhase     += 1.73f;
}

void CPlayer_Cam::StartCameraRoll(_float fRollAngle, _float fDuration, _float fInRatio, _float fOutPower)
{
	if (fDuration <= 0.f || fInRatio <= 0.f)
		return;

	m_bCameraRoll = true;
	m_bRollReturning = false;

	m_fRollAngle = fRollAngle;
	m_fRollDuration = fDuration;

	m_fRollInRatio = clamp(fInRatio, 0.01f, 0.99f);
	m_fRollOutPower = max(fOutPower, 0.01f);

	m_fRollElapsed = 0.f;
	m_fRollCurve = 0.f;
}
void CPlayer_Cam::StartCameraFovKick(_float fFovOffset, _float fDuration, _float fInRatio, _float Outpower)
{
	if (fDuration <= 0.f || fInRatio <= 0.f)
		return;

	m_bFovKick      = true;
	m_bFovReturning = false;

	m_fFovOffset    = fFovOffset;
	m_fFovDuration  = fDuration;

	m_fInRatio      = clamp(fInRatio, 0.01f, 0.99f);
	m_fOutPower     = max(Outpower, 0.01f);
				    
	m_fFovElapsed   = 0.f;
	m_fFovCurve     = 0.f;
}

void CPlayer_Cam::SetPitchRoll()
{
	_vector _vLookDir	= XMVector3Normalize(Get_MainTransform().lock()->Get_WorldState(STATE::LOOK));
	float x = XMVectorGetX(_vLookDir);
	float y = XMVectorGetY(_vLookDir);
	float z = XMVectorGetZ(_vLookDir);

	m_fYaw = m_fTargetYaw = atan2f(x, z);

	m_fPitch = m_fTargetPitch = -atan2f(
		y,
		sqrtf(x * x + z * z));
}

void CPlayer_Cam::SetPitchRoll(_vector vLook)
{
	float x = XMVectorGetX(vLook);
	float y = XMVectorGetY(vLook);
	float z = XMVectorGetZ(vLook);

	m_fYaw = m_fTargetYaw = atan2f(x, z);

	m_fPitch = m_fTargetPitch = -atan2f(
		y,
		sqrtf(x * x + z * z));
}

void CPlayer_Cam::ApplyCamShake(_float fTimeDelta)
{
	if (!m_bIsCamShake)
		return;

	auto spTransform = Get_MainTransform().lock();
	if (!spTransform)
		return;

	m_fShakeElapsed += fTimeDelta;


	_float fRatio = 0.f;

	if (m_fShakeDuration)
		fRatio = m_fShakeElapsed / m_fShakeDuration;

	fRatio = clamp(fRatio, 0.f, 1.f);

	_float fFade   = 1.f - fRatio;
	_float fPower  = m_fShakePower * fFade;
	_float fTime   = m_fShakeElapsed * m_fShakeFrequency;

	_float fShakeX = sinf(fTime * 17.f + m_fShakePhase) * fPower;
	_float fShakeY = cosf(fTime * 23.f + m_fShakePhase * 0.7f) * fPower;

	_vector vPos   = spTransform->Get_LocalState(STATE::POSITION);
	_vector vRight = XMVector3Normalize(spTransform->Get_LocalState(STATE::RIGHT));
	_vector vUp    = XMVector3Normalize(spTransform->Get_LocalState(STATE::UP));

	_vector vShakeOffset = vRight * fShakeX + vUp * fShakeY;

	spTransform->Set_LocalState(STATE::POSITION, vPos + vShakeOffset);

	if (m_fShakeElapsed >= m_fShakeDuration)
	{
		m_bIsCamShake    = false;
		m_fShakeElapsed  = 0.f;
		m_fShakeDuration = 0.f;
		m_fShakePower    = 0.f;
	}
}

void CPlayer_Cam::ApplyCamRoll(_float fTimeDelta)
{
	if (!m_bCameraRoll)
		return;

	auto spTransform = Get_MainTransform().lock();
	if (!spTransform)
		return;

	if (!m_bRollReturning)
	{
		m_fRollElapsed += fTimeDelta;

		_float fInDuration = m_fRollDuration * m_fRollInRatio;

		if (fInDuration <= 0.f)
			fInDuration = 0.01f;

		_float t = m_fRollElapsed / fInDuration;
		t = clamp(t, 0.f, 1.f);

		m_fRollCurve = 1.f - powf(2.f, -12.f * t);

		if (t >= 1.f)
		{
			m_fRollCurve = 1.f;
			m_bRollReturning = true;
			m_fRollElapsed = 0.f;
		}
	}
	else
	{
		
		_float fReturnSpeed = m_fRollOutPower;

		m_fRollCurve *= powf(0.001f, fTimeDelta * fReturnSpeed);

		if (m_fRollCurve <= 0.001f)
		{
			m_fRollCurve = 0.f;

			m_bCameraRoll = false;
			m_bRollReturning = false;

			m_fRollElapsed = 0.f;
			m_fRollDuration = 0.f;
			m_fRollAngle = 0.f;

			return;
		}
	}

	_float fCurRollAngle = m_fRollAngle * m_fRollCurve;

	_vector vRight = XMVector3Normalize(spTransform->Get_LocalState(STATE::RIGHT));
	_vector vUp = XMVector3Normalize(spTransform->Get_LocalState(STATE::UP));
	_vector vLook = XMVector3Normalize(spTransform->Get_LocalState(STATE::LOOK));

	_matrix matRoll = XMMatrixRotationAxis(vLook, fCurRollAngle);

	vRight = XMVector3Normalize(XMVector3TransformNormal(vRight, matRoll));
	vUp = XMVector3Normalize(XMVector3TransformNormal(vUp, matRoll));

	spTransform->Set_LocalState(STATE::RIGHT, vRight);
	spTransform->Set_LocalState(STATE::UP, vUp);
}
void CPlayer_Cam::ApplyFovKick(_float fTimeDelta)
{
	if (!m_bFovKick)
	{ 
		return;
	}

	if (!m_bFovReturning)
	{
		m_fFovElapsed += fTimeDelta;

		_float fInDuration = m_fFovDuration * m_fInRatio;

		if (fInDuration <= 0.f)
			fInDuration = 0.01f;

		_float t = m_fFovElapsed / fInDuration;
		t = clamp(t, 0.f, 1.f);

		m_fFovCurve = 1.f - powf(2.f, -12.f * t);

		if (t >= 1.f)
		{
			m_fFovCurve = 1.f;
			m_bFovReturning = true;
			m_fFovElapsed = 0.f;
			m_desc.fFovy = m_fBaseFov;
		}
	}
	else
	{

		_float fReturnSpeed = m_fOutPower;

		
		m_fFovCurve *= powf(0.001f, fTimeDelta * fReturnSpeed);

		if (m_fFovCurve <= 0.001f)
		{
			m_fFovCurve = 0.f;

			m_bFovKick = false;
			m_bFovReturning = false;

			m_fFovElapsed = 0.f;
			m_fFovDuration = 0.f;
			m_fFovOffset = 0.f;

			m_desc.fFovy = m_fBaseFov;
			return;
		}
	}

	_float fCurFov = m_fBaseFov + m_fFovOffset * m_fFovCurve;

	_float fMinFov = XMConvertToRadians(35.f);
	_float fMaxFov = XMConvertToRadians(90.f);

	fCurFov = clamp(fCurFov, fMinFov, fMaxFov);

	m_desc.fFovy = fCurFov;
}

void CPlayer_Cam::ApplyCameraEffect(_float fTimeDelta)
{
	ApplyCamShake(fTimeDelta);
	ApplyCamRoll (fTimeDelta);
	ApplyFovKick (fTimeDelta);
}

void CPlayer_Cam::SetTargetBone(CBone* pTargetBone, float fTransitDuration, float fTargetFOV)
{
	if (m_pTargetBone != pTargetBone) {
		m_pPrevBone		= m_pTargetBone;
		m_pTargetBone	= pTargetBone;

		m_fTargetFOV	= fTargetFOV;
		m_fPrevTargetFOV= m_desc.fFovy;

		m_upCamTransitTimer->SetDuration(fTransitDuration);
		m_upCamTransitTimer->Restart();

		if (m_pPrevBone) {
			auto _desc = m_pPrevBone->GetDesc();
			if (auto spTransformCom = m_prevTargetTransform.lock()) {
				spTransformCom->Aim(-XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[0])));
				spTransformCom->Set_LocalState(STATE(3),
					XMLoadFloat4(
						reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[3])
					)
				);
				spTransformCom->Set_Scale(1.f, 1.f, 1.f);
			}
		}
		else {
			m_prevTargetTransform.lock()->Copy_LocalMatrix(m_defaultCamTransform.lock()->GetWorldMatrix());
		}
	}
}
void CPlayer_Cam::SetLerpFOV(float fTransitDuration, float fTargetFOV)
{
   m_fTargetFOV = fTargetFOV;
   m_fPrevTargetFOV = m_desc.fFovy;
      
   m_upCamTransitTimer->SetDuration(fTransitDuration);
   m_upCamTransitTimer->Restart();
}     

