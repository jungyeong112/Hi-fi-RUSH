#include "Player.h"
#include "Player_Cam.h"
#include "BaseModel.h"
#include "BlackBoard.h"
#include "Bone.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Collidor.h"
#include "Player_FSM.h"
#include "Weapon.h"
#include "Interact_Magnetic.h"
#include "Interact_LastMagnetic.h"
#include "Interact_Mimosa_Object.h"
#include "EnemyShield.h"
#include "MeshRenderer.h"
#include "MeshEffect.h"
#include "EffectRoot.h"
#include "VFX_Trail.h"
#include "808_Ball.h"
#include "808_Cat.h"
#include "Enemy.h"
#include "Enemy_Mimosa.h"
#include "Enemy_Kale.h"
#include "CameraHandler.h"
#include "Partner_Character_Peppermint.h"
#include "Partner_Character_Macaron.h"
#include "MagnetRail.h"

#include "UC_GamePlay.h"
#include "UC_Beathit.h"
#include "UC_PlayerStatus.h"
#include "UC_Interact.h"
#include "UC_UIEffect.h"
#include "UC_Score.h"
#include "UC_RhythmNote.h"
#include "UC_Widget.h"
#include "UC_Message.h"
#include "FSM_PartnerState.h"

#include "Level_GamePlay.h"
#include "CutScene.h"

#include "BattleField.h"

#include "QTEManager.h"
#include <Kale_Arm.h>
#include "Enemy_Kale_Phase4.h"
#include <QTERhythmHero.h>

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter(pDevice, pContext)
{
}

CPlayer::CPlayer(const CPlayer& Prototype)
	: CCharacter(Prototype)
{
}

HRESULT CPlayer::Initialize_Prototype(void* pArg)
{
	CHKFAIL(__super::Initialize_Prototype(pArg))
		return S_OK;
}

HRESULT CPlayer::Initialize(void* pDesc)
{
	CHKFAIL(__super::Initialize(pDesc))

		//RhythmDash
		m_pGameInstance->AddBeatNotifiy(3.7f, 0.4f, L"rhythmDash");
	m_pGameInstance->AddBeatNotifiy(2.7f, 3.4f, L"rhythmDash");
	m_pGameInstance->AddBeatNotifiy(1.7f, 2.4f, L"rhythmDash");
	m_pGameInstance->AddBeatNotifiy(0.7f, 1.4f, L"rhythmDash");

	//Input Timming
	m_pGameInstance->AddBeatNotifiy(3.9f, 0.1f, L"Perfect");
	m_pGameInstance->AddBeatNotifiy(2.9f, 3.1f, L"Perfect");
	m_pGameInstance->AddBeatNotifiy(1.9f, 2.1f, L"Perfect");
	m_pGameInstance->AddBeatNotifiy(0.9f, 1.1f, L"Perfect");

	m_pGameInstance->AddBeatNotifiy(3.7f, 0.3f, L"Good");
	m_pGameInstance->AddBeatNotifiy(2.7f, 3.3f, L"Good");
	m_pGameInstance->AddBeatNotifiy(1.7f, 2.3f, L"Good");
	m_pGameInstance->AddBeatNotifiy(0.7f, 1.3f, L"Good");



	_upBeatListener = m_pGameInstance->CreateBeatListener(L"rhythmDash", nullptr,
		[=]() {	m_bRhythmDash = true; }, [=]() {	m_bRhythmDash = false; }, false);

	_upPerfectTime = m_pGameInstance->CreateBeatListener(L"Perfect", nullptr,
		[=]() {	m_bPerfectTime = true; }, [=]() {	m_bPerfectTime = false; }, false);

	_upGoodTime = m_pGameInstance->CreateBeatListener(L"Good", nullptr,
		[=]() {	m_bGoodTime = true; }, [=]() {	m_bGoodTime = false; }, false);


	//Model
	/*CModel::MODEL_DESC _modelDesc;
	lstrcpy(_modelDesc.m_sModelAssetName, L"Player_Chai_Export");
	_modelDesc.m_bIsCopy = false;
	Add_Component<CModel>(L"Com_Model_Main", &_modelDesc);
	*/

	m_iCurState = IDLE;

	CGameObject::LoadPrototype(L"Weapon_chai_Guitar",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pGuitar));

	CGameObject::LoadPrototype(L"Weapon_TrashGuitar",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pTrashGuitar));

	CGameObject::LoadPrototype(L"Prototype_PartnerWeapon_Peppermint",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pWeaponPartner[0]));

	CGameObject::LoadPrototype(L"Prototype_PartnerWeapon_Macaron",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pWeaponPartner[1]));

	CGameObject::LoadPrototype(L"Prototype_PartnerWeapon_Korsica",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pWeaponPartner[2]));

	CGameObject::LoadPrototype(L"Prototype_Weapon_Korsica",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pWeaponPartnerWeaponKorsica_L));
	CGameObject::LoadPrototype(L"Prototype_Weapon_Korsica",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pWeaponPartnerWeaponKorsica_R));
	CGameObject::LoadPrototype(L"Prototype_Weapon_Peppermint_Blaster",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pWeaponPartnerWeaponPeppermint_MasterBlaster));



	m_pGuitar->SetLevelObject();
	m_pTrashGuitar->SetLevelObject();

	m_pWeaponPartner[0]->SetLevelObject();
	m_pWeaponPartner[1]->SetLevelObject();
	m_pWeaponPartner[2]->SetLevelObject();

	m_pWeaponPartnerWeaponKorsica_L->SetLevelObject();
	m_pWeaponPartnerWeaponKorsica_R->SetLevelObject();
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->SetLevelObject();






	m_pTrashGuitar->SetActive(false);

	ActiveGuitar();

	SetPartner();
	Set_Timer();

	CGameObject::LoadPrototype(L"808_Ball",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_p808_Ball));

	CGameObject::LoadPrototype(L"808_Cat",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_p808_Cat));

	CGameObject::LoadPrototype(L"CamHandler_MimosaDance",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&m_pMimosaDanceCamHandler));

	return S_OK;
}

void CPlayer::SetBB()
{
	wstring ConfigPath = L"../AnimNodes/Animator_Player.json";
	Json::Value tmp;
	m_pGameInstance->LoadJson(ConfigPath, tmp);

	SetBBFromAnimJson(tmp, m_ctxDesc);
}

void CPlayer::Restart()
{
	SetBoolElement("Parry", false);
	SetBoolElement("HIT", false);
	SetBoolElement("DIE", false);
	m_bisDie = false;
	m_PlayerState.m_fCurHp = m_PlayerState.m_fMaxHp;
	m_PlayerState.m_iCurEnergy = 0;
	if (!XMVector3Equal(m_vRespawnPos, XMVectorZero()))
		SetPos(m_vRespawnPos);
}

HRESULT CPlayer::Late_Initialize()
{
	CHKFAIL(__super::Late_Initialize())
		SetBB();

	m_wpSkeletonModel = ConvertWPComponent<CModel>(Get_Component(L"Model_Skeleton"));
	m_wpSkeletonModel.lock()->RenderActive(false);

	if (auto spModel = m_wpModel.lock()) {
		m_pCameraBone = spModel->FindBoneWithName(L"attach_cam_socket");
	}

	wstring ConfigPath = L"../AnimNodes/Animator_Player.json";
	Json::Value tmp;
	m_pGameInstance->LoadJson(ConfigPath, tmp);
	m_upAnimController = CAnimController::Create(tmp, m_wpModel.lock());
	m_upAnimController->SetActive(true);

	m_upAnimController->SetLayerIdx("IDLE_Idx", 0);
	m_upAnimController->SetLayerIdx("RUN_Idx", 0);

	if (auto spModel = m_wpModel.lock()) {
		spModel->GetMeshRenderer(19)->SetActive(false);
		spModel->GetMeshRenderer(20)->SetActive(false);
		spModel->GetMeshRenderer(21)->SetActive(false);
	}


	Add_Listener();
	Set_TransitData();

	m_upPlayerFSM = CPlayer_FSM::Create(nullptr);
	m_upPlayerFSM->Connect_Data(&m_TransitData);
	m_upPlayerFSM->Late_Initialize();
	SetWeapon();

	Set_VFXBone();

	Set_VFX();
	SetCollider();


	m_p808_Ball->SetLevelObject();
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	m_p808_Ball->SetBone(pBone);
	m_p808_Ball->SetActive(true);


	m_p808_Cat->SetLevelObject();
	auto pBone1 = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	m_p808_Cat->SetBone(pBone1);
	m_p808_Cat->SetPlayer(this);
	m_p808_Cat->SetActive(false);


	// UIController
	if (m_pGameInstance->GetCurrentLevelTag() != L"Level_Editor")
	{
		m_pUCGamePlay = static_cast<CUC_GamePlay*>         (m_pGameInstance->GetCurrentLevelUC());
		m_pUCBeathit = static_cast<CUC_Beathit*>           (m_pUCGamePlay->GetUIController(L"UC_Bithit"));
		m_pUCPlayerStatus = static_cast<CUC_PlayerStatus*> (m_pUCGamePlay->GetUIController(L"UC_PlayerStatus"));
		m_pUCInteract = static_cast<CUC_Interact*>         (m_pUCGamePlay->GetUIController(L"UC_Interact"));
		m_pUCEffect = static_cast<CUC_UIEffect*>           (m_pUCGamePlay->GetUIController(L"UC_UIEffect"));
		m_pUCScore = static_cast<CUC_Score*>		       (m_pUCGamePlay->GetUIController(L"UC_Score"));
		m_pUCRhythmNote = static_cast<CUC_RhythmNote*>     (m_pUCGamePlay->GetUIController(L"UC_RhythmNote"));
		m_pUCWidget = static_cast<CUC_Widget*>		       (m_pUCGamePlay->GetUIController(L"UC_Widget"));
		m_PUCMessage = static_cast<CUC_Message*>           (m_pUCGamePlay->GetUIController(L"UC_Message"));
		m_pUCGamePlay->Set_StorePlayer(this);
		//m_pUCGamePlay->UpGradeHpBar();
	}

	/* QTE Test Code */

	if (m_pGameInstance->GetCurrentLevelTag() == L"Level_GamePlay")
		m_pQTEManager = CQTEManager::Create_QTEManager();
	/*
	m_tQTEDesc.bResetOnFail = false;
	m_tQTEDesc.bFailToTimeOut = false;

	FQTEBuildCursor tCursor{};

	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_LBUTTON, 8.f, 4.f, 1.f, 1.f);
	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_RBUTTON, 8.f, 4.f, 1.f, 1.f);
	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_DIK_Q, 8.f, 4.f, 1.f, 1.f);
	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_DIK_E, 8.f, 4.f, 1.f, 1.f);
	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_DIK_SPACE, 8.f, 4.f, 1.f, 1.f);
	AddHoldReleaseStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_LBUTTON, 8.f, 4.f, 8.f, 4.f, 1.f, 1.f);
	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_LBUTTON, 8.f, 4.f, 1.f, 1.f);
	AddPressStep(m_tQTEDesc, tCursor, EQTE_Input_Type::QTE_INPUT_RBUTTON, 8.f, 4.f, 1.f, 1.f);
	*/

	// DSP
	m_pGamePlay = { dynamic_cast<CLevel_GamePlay*>(m_pGameInstance->GetCurrentLevel()) };
	if (m_pGamePlay)
		Safe_AddRef(m_pGamePlay);


	CBone* pOriginBone = m_wpModel.lock()->FindBoneWithName(L"origin");

	m_pWeaponPartner[0]->Set_Bone(pOriginBone);
	m_pWeaponPartner[1]->Set_Bone(pOriginBone);
	m_pWeaponPartner[2]->Set_Bone(pOriginBone);

	m_pWeaponPartnerWeaponKorsica_L->Set_Bone(m_pWeaponPartner[2]->GetBone(L"l_attach_hand_00"));
	m_pWeaponPartnerWeaponKorsica_R->Set_Bone(m_pWeaponPartner[2]->GetBone(L"r_attach_hand_00"));
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->Set_Bone(m_pWeaponPartner[0]->GetBone(L"Weapon_attach_Socket_L"));


	m_pWeaponPartner[0]->SetTrashAnim(0, false);
	m_pWeaponPartner[1]->SetTrashAnim(0, false);
	m_pWeaponPartner[2]->SetTrashAnim(0, false);

	for (size_t i = 0; i < 3; i++)
	{
		m_pWeaponPartner[i]->Priority_Update(0.f);
		m_pWeaponPartner[i]->Update(0.f);
		m_pWeaponPartner[i]->Late_Update(0.f);
		m_pWeaponPartner[i]->SetActive(false);
	}

	for (auto pWeapon : { m_pWeaponPartnerWeaponKorsica_L , m_pWeaponPartnerWeaponKorsica_R, m_pWeaponPartnerWeaponPeppermint_MasterBlaster }) {

		pWeapon->Priority_Update(0.f);
		pWeapon->Update(0.f);
		pWeapon->Late_Update(0.f);
		pWeapon->SetActive(false);
	}
	//m_pWeaponPartner[1]->SetActive(false);
	//m_pWeaponPartner[2]->SetActive(false);

	SetEmissive();

	return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (!m_listEnemy.empty() && !m_pAtkCntTimer->IsActive() && !m_bisDie)
	{
		m_fElapsedAtkTime += fTimeDelta;
		if (m_fElapsedAtkTime >= 1.f)
		{
			m_fElapsedAtkTime = 0.f;
			AddRankScore(-40);
		}
	}
	if (m_bisRp)m_pAtkCntTimer->Restart();

	m_fElasedBattleTime += fTimeDelta;
	auto _peppCool = pPartners[EPartnerType::PT_PEPPERMINT]->GetCooldownRatio();
	auto _macaCool = pPartners[EPartnerType::PT_MACARON]->GetCooldownRatio();
	auto _korCool = pPartners[EPartnerType::PT_KORSICA]->GetCooldownRatio();

	if (m_pUCPlayerStatus)
	{
		m_pUCPlayerStatus->CoolDownKor(_korCool);
		m_pUCPlayerStatus->CoolDownMaca(_macaCool);
		m_pUCPlayerStatus->CoolDownPepp(_peppCool);
	}


	if (m_pUCBeathit != nullptr && m_pBeatHitTimer->IsActive() && m_eRhythmResult == ERhythmResult::None)
	{
		m_pUCBeathit->Set_BHRatio((Engine::_float)m_pBeatHitTimer->GetElapsedRatio(), !m_bisRp);
	}
	if (m_pUCBeathit != nullptr && m_pRP_StartUI_Timer->IsActive())
		m_pUCBeathit->Set_Parry_BHRatio((Engine::_float)m_pRP_StartUI_Timer->GetElapsedRatio());
	if (m_pUCEffect != nullptr && m_pUCEffect->Get_StartPowerPlant())
		m_pUCEffect->PowerPlantPlayerPos(m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION));


	Load_Config();
	DebugPos();
	m_fTestTime += fTimeDelta;
	CalcScore();
	if (m_bisDie && m_pGameInstance->IsKeyState(DIK_1, EKEYACTIONSTATE::ENTER))
		Restart();
	if (!m_bisDie && m_pGameInstance->IsKeyState(DIK_2, EKEYACTIONSTATE::ENTER))
		ChangeSP();
	if (m_bisRp && (m_eRhythmResult == ERhythmResult::Good || m_eRhythmResult == ERhythmResult::Perfect))
		SuccessRP_WA();
	if (!m_bisDie && m_pGameInstance->IsKeyState(DIK_3, EKEYACTIONSTATE::ENTER))
	{
		for (int i = 0; i < 4; i++)
		{
			Unlock_PartnerJam(i);
		}
	}
	if (!m_bisDie && m_pGameInstance->IsKeyState(DIK_4, EKEYACTIONSTATE::ENTER))
	{
		m_PlayerState.m_iCurEnergy = m_PlayerState.m_iMaxEnergy;
	}

	// Store Test
	if (m_pGameInstance->IsKeyState(DIK_E, EKEYACTIONSTATE::ENTER) && m_bIsStoreAble && !m_bisStore && !m_bMoveKeyInput)
	{
		m_pUCGamePlay->StoreOn(m_PlayerState.m_iGear);
	}



	/* QTE Test Code */
	/*
	if (m_pGameInstance->IsKeyState(DIK_NUMPAD8, EKEYACTIONSTATE::ENTER))
		Client::Request_QTE(m_pGameInstance, m_tQTEDesc, [this]() {ElectricShock(); }, [this]() {Ignite(); });
	*/


	if (m_pGameInstance->IsKeyState(DIK_R, EKEYACTIONSTATE::ENTER) && !CCutScene::Get_IsCutScene() && !m_bisRp
		&& m_PlayerState.m_iCurEnergy == m_PlayerState.m_iMaxEnergy && !m_bKeyLock)
	{
		if (m_iCurSPIdx && !m_bisHibiki)
			StartHibiki();
		else
			StartSP808();

		m_bInvincible = true;
		m_PlayerState.m_iCurEnergy -= m_PlayerState.m_iMaxEnergy;
	}
	if (m_wpSkeletonModel.lock()->GetAnimRatio() >= 0.8f && m_bisElectricShock)
	{
		EndElectricShock();
	}
	Check_MimosaDance();
	CheckHibiKi();
	CheckInputIngnite();
	if (m_pColorReserveTimer && m_pColorReserveTimer->IsActive())
	{
		auto _ratio = m_pColorReserveTimer->GetElapsedRatio();
		auto _colorRatio = 1 - _ratio;

		m_pGameInstance->SetColorReverse((Engine::_float)_colorRatio);
	}
	if (m_pScoreMulTimer->IsActive())
		m_pUCScore->Get_ScoreMulTime_Ratio((Engine::_float)m_pScoreMulTimer->GetElapsedRatio());


	PlaySoundXXXX();
	Update_Flicker(fTimeDelta);
	UpdateEmissive(fTimeDelta);
}
void CPlayer::Update(_float fTimeDelta)
{
	/* QTE Test Code */

	if (m_pQTEManager)
	{
		if (m_pQTEManager->IsPlaying()) m_pQTEManager->Update(fTimeDelta, Make_QTEInputDesc());
		if (m_bQtePlay && !m_pQTEManager->IsPlaying())
		{
			m_bSkipUpdate = false;
			m_bQtePlay = false;

			if (m_pQTEManager->IsSuccess())
				SuccessQTE();
			else
				FailQTE();

			m_pPlayerCam->SetHandle(false);
		}
	}


	if (!m_bisDie && !m_bSkipUpdate && !CCutScene::Get_IsCutScene())
	{
		CheckGround(fTimeDelta);
		CheckState();
		if (m_bIsAttack)
			AttackSnap(fTimeDelta);

		StayMagnet(fTimeDelta);
		Dash(fTimeDelta);
		Move(fTimeDelta);
		if (!m_pMimosaGrabBone)
			ApplyGravity(fTimeDelta);
		ChangePartner();
	}
	StayMagnetRail(fTimeDelta);
	if (m_pRail && m_pRail->IsEnd())
		EndMangetRail();
	__super::Update(fTimeDelta);
	UpdateEffectTrail();

	if (m_upPlayerFSM)
		m_upPlayerFSM->Update(fTimeDelta);

	if (!CCutScene::Get_IsCutScene())
	{
		m_upAnimController->UpdateCTX(m_ctxDesc);
		m_upAnimController->Update(fTimeDelta);
	}

	if (m_pGameInstance->GetCurrentLevelTag() != L"Level_Editor")
	{
		m_pUCPlayerStatus->SetPlayerState(m_PlayerState);
		m_pUCScore->SetPlayerState(m_PlayerState);
	}
	if (m_pMimosaGrabBone)
	{
		SetPos(
			lerp(
				m_pMimosa->Get_MainTransform().lock()->Get_WorldState(STATE::POSITION),
				XMLoadFloat4(reinterpret_cast<const _float4*>(m_pMimosaGrabBone->GetDesc().m_CombinedTransformationMatrix.m[3])) + XMVectorSet(0.f, 0.f, 0.f, 0.f)
				/*+ XMVector3Normalize(XMLoadFloat4(reinterpret_cast<const _float4*>(&m_pMimosaGrabBone->GetDesc().m_CombinedTransformationMatrix.m[0]))) * 0.07f*/,
				1.0f
			));

		m_wpMainTransformCom.lock()->Aim(
			XMVectorSetY(XMLoadFloat4(reinterpret_cast<const _float4*>(&m_pMimosaGrabBone->GetDesc().m_CombinedTransformationMatrix.m[0])), 0.f));


	}



}
void CPlayer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_pBeatHitTimer->IsActive())
	{
		auto time = m_pBeatHitTimer->GetElapsedTime();
		if (!time)
		{
			m_pGameInstance->Play_SFX(L"ui_BH_timing_circle_01_Play1", 1.5f);
		}
		else if (time >= 0.31f && !m_bFirstbeatSFX)
		{
			m_bFirstbeatSFX = true;
			m_pGameInstance->Play_SFX(L"ui_BH_timing_circle_01_Play2", 1.5f);
		}
		else if (time >= 0.7f && !m_bFirstbeatSFX2)
		{
			m_bFirstbeatSFX2 = true;
			m_pGameInstance->Play_SFX(L"ui_BH_timing_circle_01_Play1", 1.5f);
			m_pGameInstance->Play_SFX(L"ui_BH_timing_circle_01_Play2", 1.5f);
		}
	}


	if (!m_bGrounded)
		SetBoolElement("Grounded", false);
	else
		SetBoolElement("Grounded", true);


	if (m_bisCutScene && !CCutScene::Get_IsCutScene())
		OutCutScene();
	else if (!m_bisCutScene && CCutScene::Get_IsCutScene())
		InCutScene();

	m_bisCutScene = CCutScene::Get_IsCutScene();

	EnemyRot();

	if (m_bMimosaDance)
		m_bSkipUpdate = true;

	if (m_PlayerState.m_fCurHp <= m_PlayerState.m_fMaxHp * 0.3f && !m_bisCutScene)
		m_pGameInstance->SetLowHP(true);
	else m_pGameInstance->SetLowHP(false);
	//m_pGameInstance->Add_Chasing_Light(m_wpMainTransformCom.lock()->Get_WorldState(STATE::POSITION), CHASING_TYPE::PLAYER);
}

void CPlayer::Add_Listener()
{
	auto pModel = m_wpModel.lock();
	auto propBone = pModel->FindBoneWithName(L"prop_02");

	pModel->AddListener(L"toStayJump", nullptr, [=]() {SetBoolElement("Enter_Jump", false); }, nullptr);
	pModel->AddListener(L"toJumpDashLoop", nullptr, [=]() {SetBoolElement("JumpDash", false); }, nullptr);
	pModel->AddListener(L"Moveable", nullptr, [=]() {m_bCanMove = false; }, [=]() {if (!m_bisRp)m_bCanMove = true; });
	pModel->AddListener(L"Jumpable", nullptr, [=]() {m_bJumpable = true; }, [=]() {SetBoolElement("Land_Jump", false); });
	pModel->AddListener(L"toStayDoubleJump", nullptr, [=]() {SetBoolElement("Enter_DoubleJump", false); }, nullptr);
	pModel->AddListener(L"DashChangeAble", nullptr, [=]() {m_bChangeDash = true; }, [=]() {m_bChangeDash = false; });
	pModel->AddListener(L"DashAnimEnd", nullptr, [=]() {m_bDashAnimEnd = false; }, [=]() {m_bDashAnimEnd = true; });
	pModel->AddListener(L"MagnetGravity", nullptr, [=]() {m_fGravityMul = 0.f; }, [=]() {m_fGravityMul = 1.f; });
	pModel->AddListener(L"VFX_Jump", nullptr, [=]() {RestartVFX(VFX_JUMP, GetPos()); if (!m_bCanDoubleJump)
	{
		RestartVFX(VFX_DOUBLEJUMP, GetPos()); RestartVFX(VFX_JUMPFONT, GetPos());
	} }, nullptr);
	pModel->AddListener(L"VFX_Land", nullptr, [=]() {RestartVFX(VFX_LAND, GetPos()); }, nullptr);
	pModel->AddListener(L"AttackX", nullptr, [=]() {SetBoolElement("AttackX", false);  }, nullptr);
	pModel->AddListener(L"AttackY", nullptr, [=]() {SetBoolElement("AttackY", false);  }, nullptr);
	pModel->AddListener(L"ComboAble", nullptr, [=]() {m_bComboAble = false;  }, [=]() {m_bComboAble = true; });
	pModel->AddListener(L"EnterCombo", nullptr, [=]()
		{SetBoolElement("ComboX", false); SetBoolElement("ComboY", false);	m_bComboXQueued = false; }, nullptr);
	pModel->AddListener(L"VFX_AirY", nullptr, [=]() {RestartVFX(VFX_ATTACK_AIR_Y, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_Idle", nullptr, [=]() {RestartVFX(VFX_IDLE, Get_BonePos(L"l_hand"), GetLook()); if (m_pMimosa)m_pGamePlay->SetHalftoneStepPos(Get_BonePos(L"LeftFootSocket"), 0); }, nullptr);
	pModel->AddListener(L"VFX_ComboX", nullptr, [=]() {RestartVFX(VFX_ATTACK_X, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ComboXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_XX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ComboXXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_XXX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ComboXXXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_XXXX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ComboY", nullptr, [=]() {RestartVFX(VFX_ATTACK_Y, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ComboYY", nullptr, [=]() {RestartVFX(VFX_ATTACK_YY, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ComboYYY", nullptr, [=]() {RestartVFX(VFX_ATTACK_YYY, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_AirComboX", nullptr, [=]() {RestartVFX(VFX_ATTACK_AIR_X, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_AirComboXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_AIR_XX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_AirComboXXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_AIR_XXX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_AirComboXXXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_AIR_XXXX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_X_X", nullptr, [=]() {RestartVFX(VFX_ATTACK_X_X, GetPos());  }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_X_XX", nullptr, [=]() {RestartVFX(VFX_ATTACK_X_XX, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XX_X", nullptr, [=]() {RestartVFX(VFX_ATTACK_XX_X, GetPos());  }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XX_XX", nullptr, [=]() {RestartVFX(VFX_ATTACK_XX_XX, GetPos());  }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XX_XXX", nullptr, [=]() {RestartVFX(VFX_ATTACK_XX_XXX, GetPos());  }, nullptr);
	pModel->AddListener(L"VFX_BEATHIT_XX_XXX", nullptr, [=]() {RestartVFX(VFX_BEATHIT_XX_XXX, GetPos());  }, nullptr);

	pModel->AddListener(L"VFX_HIBIKI_GUITAR", nullptr, [=]() {
		auto guitarBone = m_pGuitar->GetBone(L"g_07");

		auto& mat = guitarBone->GetDesc().m_CombinedTransformationMatrix;

		_vector vPos = XMVectorSet(mat._41, mat._42, mat._43, 1.f);
		_vector vLook = XMVectorSet(mat._31, mat._32, mat._33, 0.f);
		vLook = XMVector3Normalize(vLook);
		RestartVFX(VFX_HIBIKI_GUITAR, vPos, vLook);
		}, nullptr);
	pModel->AddListener(L"VFX_SP_808", nullptr, [=]() {RestartVFX(VFX_SP_808, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_HIBIKI_END", nullptr, [=]() {RestartVFX(VFX_HIBIKI_END, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_HIBIKI", nullptr, [=]() {m_pEffect[VFX_HIBIKI]->Restart(); }, nullptr);
	pModel->AddListener(L"VFX_GainTornado", nullptr, [=]() {RestartVFX(VFX_GAINTORNADO, GetPos()); }, nullptr);
	pModel->AddListener(L"VFX_GainTornadoSwing", nullptr, [=]() {RestartVFX(VFX_GAINTORNADO_SWING, GetPos(), GetLook()); }, nullptr);

	pModel->AddListener(L"VFX_DANCE_A", nullptr, [=]() {RestartVFX(VFX_DANCE_A, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_DANCE_LB", nullptr, [=]() {RestartVFX(VFX_DANCE_LB, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_DANCE_RB", nullptr, [=]() {RestartVFX(VFX_DANCE_RB, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_DANCE_FAIL", nullptr, [=]() {RestartVFX(VFX_DANCE_FAIL, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_DANCE_SUCCESS", nullptr, [=]() {RestartVFX(VFX_DANCE_SUCCESS, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_BATTLE_START_GUITAR", nullptr, [=]()
		{
			auto guitarBone = m_pGuitar->GetBone(L"g_07");
			auto& mat = guitarBone->GetDesc().m_CombinedTransformationMatrix;
			_vector vPos = XMVectorSet(mat._41, mat._42, mat._43, 1.f);
			RestartVFX(VFX_BATTLE_START_GUITAR, vPos);
		}, nullptr);


	pModel->AddListener(L"BeatHit", nullptr, [=]() { m_pBeatHitTimer->Restart();	m_pPlayerCam->StartCameraFovKick(XMConvertToRadians(7.f), 0.5f, 0.5f, 3.f); }, nullptr);
	pModel->AddListener(L"AttackCol", nullptr, [=]() { m_wpAttackCollider.lock()->SetActive(true); }, [=]() { m_wpAttackCollider.lock()->SetActive(false); });
	pModel->AddListener(L"isParry", nullptr, [=]() { m_bisParry = true; }, [=]() { m_bisParry = false; SetBoolElement("Parry", false); });
	pModel->AddListener(L"isAirParry", nullptr, [=]() { m_bisParry = true; }, [=]() { m_bisParry = false; });
	pModel->AddListener(L"isAirParryEnd", nullptr, nullptr, [=]() {SetBoolElement("Parry", false); });
	pModel->AddListener(L"AirCombo", nullptr, [=]() { m_bIsAirCombo = true; }, [=]() {m_bIsAirCombo = false; });
	pModel->AddListener(L"Hit", nullptr, [=]() {
		m_bisHit = true; m_bCanMove = false; }, [=]() { m_bisHit = false; m_bCanMove = true; });
	pModel->AddListener(L"Mvnt", nullptr, [=]() { SetBoolElement("MVNT", false); }, nullptr);
	pModel->AddListener(L"RhythmTarget", [=](float) {m_wpModel.lock()->AnimSet_RhythmTarget(0.5f); }, nullptr, nullptr);
	pModel->AddListener(L"EndCheer", nullptr, [=]() { SetBoolElement("Success_Cheer", false); SetBoolElement("Fail_Cheer", false);   m_bCanMove = true; m_bJumpable = true; m_bisGimmic = false; m_b808AnimOnce = false; Switch808_Ball(); }, nullptr);
	pModel->AddListener(L"LinkAirCombo", nullptr, [=]() { m_bIsAirCombo = true; }, [=]() { m_bIsAirCombo = false; m_fSpeedY = 0.f; m_bJumpable = false;  });
	pModel->AddListener(L"ComboReset", nullptr, [=]() {ResetCombo(); }, nullptr);
	pModel->AddListener(L"RestAble", nullptr, [=]() {m_bRestAble = true; }, nullptr);
	pModel->AddListener(L"Switch808_Ball", nullptr, [=]() { Switch808_Ball(); }, nullptr);
	pModel->AddListener(L"Switch808_Cat", nullptr, [=]() { Switch808_Cat(); }, nullptr);
	pModel->AddListener(L"AttackDistortionY", nullptr, [=]() {DistortionComboY(); }, nullptr);
	pModel->AddListener(L"EnterCutScene", nullptr, [=]() {m_bisCutScene = true; CutSceneReset(); m_bCanMove = false; }, [=]() {m_bisCutScene = false; m_bCanMove = true;
	if (m_pUCEffect != nullptr)m_pUCEffect->CombatStartEffect();
	if (m_pUCGamePlay != nullptr)m_pUCGamePlay->SetScoreCalc(true); });
	pModel->AddListener(L"RP_WA_RESET", nullptr, [=]() {ResetRp_WA(); }, nullptr);
	pModel->AddListener(L"AttachCam", nullptr, [=]() {	if (m_pPlayerCam) m_pPlayerCam->SetTargetBone(propBone, 0.4f, XM_PI / 18.f * 10.f); }, [=]() {	if (m_pPlayerCam) m_pPlayerCam->SetTargetBone(nullptr, 1.f); });
	pModel->AddListener(L"Kill_RPEnemy", nullptr, [=]() { Kill_RP_Enemy(); }, nullptr);
	pModel->AddListener(L"EnterKnockback", nullptr, [=]() { SetBoolElement("Knockback", false); }, nullptr);
	pModel->AddListener(L"Enter_Dance", nullptr, [=]() { SetBoolElement("MIMOSA_DANCE", false); }, nullptr);
	pModel->AddListener(L"Enter_DanceInput", nullptr, [=]() {Reset_DanceInput(); }, nullptr);
	pModel->AddListener(L"Enter_MimosaGrab", nullptr, [=]() { SetBoolElement("MimosaGrab", false); }, nullptr);

	pModel->AddListener(L"Enter_PeppGroundJam", nullptr, [=]() { SetBoolElement("Enter_PeppGroundJam", false); }, nullptr);
	pModel->AddListener(L"Enter_MasterBlaster", nullptr, [=]() { SetBoolElement("Enter_PeppAirJam", false); }, nullptr);
	pModel->AddListener(L"Enter_DoubleBaseDrop", nullptr, [=]() { SetBoolElement("Enter_MacaGroundJam", false); }, nullptr);
	pModel->AddListener(L"Enter_Tornado_Lift", nullptr, [=]() { SetBoolElement("Enter_KorAirJam", false); }, nullptr);

	pModel->AddListener(L"EnterStayJump", nullptr, [=]() { m_bIsJump = true; }, nullptr);
	pModel->AddListener(L"EndHibiki", nullptr, [=]() { EndHibiki(); }, nullptr);
	pModel->AddListener(L"End_SP808", nullptr, [=]() {  EndSP808(); }, nullptr);
	pModel->AddListener(L"CamAttachStart", nullptr, [=]() { m_pPlayerCam->SetTargetBone(propBone, 0.4f); }, nullptr);
	pModel->AddListener(L"CamAttachEnd", nullptr, [=]() { m_pPlayerCam->SetTargetBone(nullptr, 0.4f); }, nullptr);
	pModel->AddListener(L"SFX_FS", nullptr, [=]() { PlayRandomSound(L"pl_ch0000_fs_bare_walk_01_Play", 4, 6); if (m_pMimosa && m_pGamePlay)m_pGamePlay->SetHalftoneStepPos(GetPos(), 0); }, nullptr);
	//RestartVFX(VFX_Player_HalftoneStep, GetPos());
	pModel->AddListener(L"Enter_MagnetRail", nullptr, [=]() { SetBoolElement("Enter_MagnetRail", false); }, nullptr);
	pModel->AddListener(L"Enter_Ignite", nullptr, [=]() { SetBoolElement("Enter_Ignite", false); }, nullptr);
	pModel->AddListener(L"EnterRPMimosa", nullptr, [=]() { SetBoolElement("ENTER_RP_MIMOSA", false); }, nullptr);
	//pModel->AddListener(L"EnterRPMimosaLast",   nullptr, [=]() { }, nullptr);
	pModel->AddListener(L"FINISH_IMAGE", nullptr, [=]() { m_PUCMessage->IllustImage(2, 1.f);  SetBoolElement("SUCCESS_RP_MIMOSA", false); 	m_bCanMove = false; }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XY", nullptr, [=]() {RestartVFX(VFX_ATTACK_XY, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XYmY", nullptr, [=]() {m_pEffect[VFX_ATTACK_XYmY]->Restart();  }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XYX", nullptr, [=]() {RestartVFX(VFX_ATTACK_XYX, GetPos(), GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_Player_Attack_XYXX_Entry", nullptr, [=]() {RestartVFX_PosBone(VFX_Player_Attack_XYXX_Entry, GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_Player_Attack_XYXX_Mid", nullptr, [=]() {RestartVFX_PosBone(VFX_Player_Attack_XYXX_Mid, GetLook());  }, nullptr);
	pModel->AddListener(L"VFX_ATTACK_XYY", nullptr, [=]() {RestartVFX(VFX_ATTACK_XYY, GetPos(), GetLook()); }, nullptr);
	pModel->AddListener(L"VFX_Guitar_XYY", nullptr, [=]() {m_pBreakDownEffectTimer->Restart(); m_bisEnemyRot = true; }, [=]() {m_pBreakDownEffectTimer->SetActive(false); m_bisEnemyRot = false; m_vecRotEnemy.clear(); });
	pModel->AddListener(L"VFX_Player_Attack_XYXX_End", nullptr, [=]() {RestartVFX_PosBone(VFX_Player_Attack_XYXX_End, GetLook());  }, nullptr);
	pModel->AddListener(L"Enter_RP_Kale", nullptr, [=]() { SetBoolElement("Enter_RP_Kale", false); }, nullptr);
	pModel->AddListener(L"Enter_RPLast_Kale", nullptr, [=]() { SetBoolElement("Enter_LastRP_Kale", false); }, nullptr);

	pModel->AddListener(L"Enter_MagnetRailMoveL", nullptr, [=]() {PlayRandomSound(L"sp_mng_st02_magnetrail_pl_dodge_02_Play", 1, 3); }, nullptr);
	pModel->AddListener(L"Enter_MagnetRailMoveR", nullptr, [=]() {PlayRandomSound(L"sp_mng_st02_magnetrail_pl_dodge_01_Play", 1, 3); }, nullptr);
	pModel->AddListener(L"End_DodgeKale", nullptr, [=]() {m_bKaleDodge = false; }, nullptr);
	pModel->AddListener(L"End_Dodge_Rail", nullptr, [=]() {m_bSuccessRail = false; }, nullptr);

	pModel->AddListener(L"KeyLock", nullptr, [=]() {m_bKeyLock = true; }, [=]() {if (!m_bisRp && !m_bisCutScene)m_bKeyLock = false; });
	pModel->AddListener(L"Enter_QTE", nullptr, [=]() {SetBoolElement("Enter_QTE", false); }, nullptr);

	pModel->AddListener(L"Eco_Trash", nullptr, [=]() { TrashGuitarChangeAnim(0, 0); }, [=]() { m_pTrashGuitar->SetActive(false); });
	pModel->AddListener(L"Shred_Trash", nullptr, [=]() { TrashGuitarChangeAnim(1, 0); }, [=]() { m_pTrashGuitar->SetActive(false); });
	pModel->AddListener(L"End_PartnerCombo", nullptr, [=]()
		{
			m_pTrashGuitar->SetActive(false);
			m_pWeaponPartner[0]->SetActive(false);
			m_pWeaponPartner[1]->SetActive(false);
			m_pWeaponPartner[2]->SetActive(false);

			m_pWeaponPartnerWeaponKorsica_L->SetActive(false);
			m_pWeaponPartnerWeaponKorsica_R->SetActive(false);
			m_pWeaponPartnerWeaponPeppermint_MasterBlaster->SetActive(false);

			if (!m_bInputPartner)
			{
				pPartners[0]->m_bIsOnLock = false;
				pPartners[1]->m_bIsOnLock = false;
				pPartners[2]->m_bIsOnLock = false;

			}
			m_bInvincible = false;

			m_bKeyLock = false;
			m_bIsJam   = false;

			if (m_pTarget)
			{
				m_pTarget->SetGamBone(nullptr, EGamComboType::NONE);
			}
		}, nullptr);


	pModel->AddListener(L"MulATK", nullptr, [=]()
		{
			RestartMulAtk(m_fMulAtk_StayTime);
		},
		nullptr);
	pModel->AddListener(L"MulATK_Impulse", nullptr, [=]()
		{
			FDamageInfo _damageInfo = {};
			_damageInfo.pSourceObj = this;
			_damageInfo.fDamage = Calc_Damage(10.f);
			_damageInfo.eAttackType = EAttackType::MULTIHITATK;

			if (m_pTarget)
				m_pTarget->OnDamage(_damageInfo);
		},
		nullptr);

	pModel->AddListener(L"NomalATK", nullptr, [=]()
		{
			FDamageInfo _damageInfo = {};
			_damageInfo.pSourceObj = this;
			_damageInfo.fDamage = Calc_Damage(10.f);
			_damageInfo.eAttackType = EAttackType::NORMAL;

			if (m_pTarget)
				m_pTarget->OnDamage(_damageInfo);

		}, nullptr);

	pModel->AddListener(L"AirBone_ATK", nullptr, [=]()
		{
			FDamageInfo _damageInfo = {};
			_damageInfo.pSourceObj = this;
			_damageInfo.fDamage = Calc_Damage(10.f);
			_damageInfo.eAttackType = EAttackType::AIRBORNE;

			if (m_pTarget)
				m_pTarget->OnDamage(_damageInfo);

			if (!m_iPartnerIdx && m_bGrounded) 
			{
				m_pTarget->SetGamBone(nullptr);
				PrintDebug("AIR_BONE NULL");
			}
			
		}, nullptr);

	pModel->AddListener(L"AirBone_ATK_LockOff", nullptr, [=]()
		{
			FDamageInfo _damageInfo = {};
			_damageInfo.pSourceObj = this;
			_damageInfo.fDamage = Calc_Damage(10.f);
			_damageInfo.eAttackType = EAttackType::AIRBORNE;

			m_pTarget->SetGamBone(nullptr, EGamComboType::KORSICA_AIR);
			if (m_pTarget)
				m_pTarget->OnDamage(_damageInfo);
		}, nullptr);

	pModel->AddListener(L"AirBoneFinisher_ATK", nullptr, [=]()
		{
			FDamageInfo _damageInfo = {};
			_damageInfo.pSourceObj = this;
			_damageInfo.fDamage = Calc_Damage(10.f);
			_damageInfo.eAttackType = EAttackType::AIRBONE_FINISHER;

			if (m_pTarget)
				m_pTarget->OnDamage(_damageInfo);
			//m_pTarget->SetGamBone(nullptr, EGamComboType::KORSICA_AIR);
		}, nullptr);


	pModel->AddListener(L"VFX_CP_Player_JumpImpact", nullptr, [=]() {
		RestartVFX(VFX_JUMP, GetPos());
		}, nullptr);
	pModel->AddListener(L"VFX_CP_Macaron_JumpImpact", nullptr, [=]() {
		VFX_SetBonePos(VFX_PARTNER_JUMPIMPACT, m_pWeaponPartner[1]->GetBone(L"l_toe"));
		m_pEffect[VFX_PARTNER_JUMPIMPACT]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_Player_DoubleJump", nullptr, [=]() {
		VFX_SetBone(VFX_PLAYER_DJ, m_pVFX_TrailBone[TB_CHEST]);
		m_pEffect[VFX_PLAYER_DJ]->Restart();
		}, nullptr);

	pModel->AddListener(L"VFX_CP_PLAYER_LANDINGSMOKE", nullptr, [=]() {
		RestartVFX(VFX_LAND, GetPos());
		}, nullptr);
	pModel->AddListener(L"VFX_CP_MACARON_LANDINGSMOKE", nullptr, [=]() {
		VFX_SetBonePos(VFX_MACARON_LANDINGSMOKE, m_pWeaponPartner[1]->GetBone(L"l_foot"));
		m_pEffect[VFX_MACARON_LANDINGSMOKE]->Restart();
		}, nullptr);


	pModel->AddListener(L"VFX_CP_Macaron_Whirl_Srt", nullptr, [=]() {
		auto _desc0 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos = XMVectorSetY(_vPos0, XMVectorGetY(_vPos1));
		m_pEffect[VFX_MACARON_WHIRL]->Set_Pos(_vPos);
		m_pEffect[VFX_MACARON_WHIRL]->Set_Dir(XMVectorSetY(_vPos0 - _vPos1, 0.f));
		m_pEffect[VFX_MACARON_WHIRL]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_Macaron_Whirl_End", nullptr, [=]() {
		m_pEffect[VFX_MACARON_WHIRL]->DissolveStop(0.2f);
		}, nullptr);

	pModel->AddListener(L"VFX_CP_Macaron_Wave", nullptr, [=]() {
		auto _desc0 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos = XMVectorSetY(_vPos0, XMVectorGetY(_vPos1));
		m_pEffect[VFX_MACARON_WAVE]->Set_Dir(XMVectorSetY(_vPos0 - _vPos1, 0.f));
		m_pEffect[VFX_MACARON_WAVE]->Set_Pos(_vPos);
		m_pEffect[VFX_MACARON_WAVE]->Restart();
		}, nullptr);

	pModel->AddListener(L"VFX_CP_Macaron_Font", nullptr, [=]() {
		auto _desc0 = m_pWeaponPartner[1]->GetBone(L"pelvis")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		auto _desc2 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos2 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc2.m_CombinedTransformationMatrix.m[3]));
		m_pEffect[VFX_MACARON_FONT]->Set_Pos(_vPos0);
		m_pEffect[VFX_MACARON_FONT]->Set_Dir(XMVectorSetY(_vPos1 - _vPos2, 0.f));
		m_pEffect[VFX_MACARON_FONT]->Restart();
		}, nullptr);


	pModel->AddListener(L"VFX_CP_DB_SHOOT", nullptr, [=]() {
		VFX_SetBonePos(VFX_PLAYER_SHOOT, m_pVFX_TrailBone[TB_CHEST]);

		auto _desc0 = m_pVFX_TrailBone[TB_CHEST]->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		auto _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*> (_desc0.m_CombinedTransformationMatrix.m[3]));
		auto _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*> (_desc1.m_CombinedTransformationMatrix.m[3]));
		m_pEffect[VFX_PLAYER_SHOOT]->Set_Dir(_vPos1 - _vPos0);
		m_pEffect[VFX_PLAYER_SHOOT]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_MacaronHit_R", nullptr, [=]() {
		VFX_SetBonePos(VFX_PARTNER_ENEMYHIT, m_pWeaponPartner[1]->GetBone(L"r_hand"));
		m_pEffect[VFX_PARTNER_ENEMYHIT]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_GuitarHit", nullptr, [=]() {
		VFX_SetBonePos(VFX_PLAYER_ENEMYHIT, m_pGuitar->GetBone(L"g_05"));
		m_pEffect[VFX_PLAYER_ENEMYHIT]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_MULTIHIT", nullptr, [=]() {
		_int iOffset = m_iMultiHItIdx;
		m_iMultiHItIdx = (m_iMultiHItIdx + 1) % 3;
		VFX_SetBonePos(
			VFX(VFX_MULTI_ENEMYHIT0 + iOffset),
			m_wpModel.lock()->FindBoneWithName(L"prop_01"),
			XMVectorSet(Random(-1.f, 1.f), Random(-1.f, 1.f), Random(-1.f, 1.f), 0.f) * 0.7f
		);
		m_pEffect[VFX(VFX_MULTI_ENEMYHIT0 + iOffset)]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_BEAMHIT", nullptr, [=]() {
		_int iOffset = m_iMultiHItIdx;
		m_iMultiHItIdx = (m_iMultiHItIdx + 1) % 3;
		VFX_SetBonePos(
			VFX(VFX_MULTI_PEPPHIT0 + iOffset),
			m_wpModel.lock()->FindBoneWithName(L"prop_01"),
			XMVectorSet(Random(-1.f, 1.f), Random(-1.f, 1.f), Random(-1.f, 1.f), 0.f) * 0.7f
		);
		m_pEffect[VFX(VFX_MULTI_PEPPHIT0 + iOffset)]->Restart();
		}, nullptr);

	pModel->AddListener(L"VFX_CP_KORSICA_SNAP_L", nullptr, [=]() {
		VFX_SetBonePos(VFX_KORSICA_SNAP, m_pWeaponPartner[2]->GetBone(L"l_hand"));
		m_pEffect[VFX_KORSICA_SNAP]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_KORSICA_SNAP_R", nullptr, [=]() {
		VFX_SetBonePos(VFX_KORSICA_SNAP, m_pWeaponPartner[2]->GetBone(L"r_hand"));
		m_pEffect[VFX_KORSICA_SNAP]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_KORSICA_SWING", nullptr, [=]() {
		VFX_SetBonePos(VFX_KORSICA_SWING, m_pWeaponPartner[2]->GetBone(L"l_hand"));
		m_pEffect[VFX_KORSICA_SWING]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_KORSICA_WIND0_START", nullptr, [=]() {
		VFX_SetBonePos(VFX_KORSICA_WIND0, m_wpModel.lock()->FindBoneWithName(L"prop_01"));
		m_pEffect[VFX_KORSICA_WIND0]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_KORSICA_WIND1_START", nullptr, [=]() {
		auto _desc0 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos = XMVectorSetY(_vPos0, XMVectorGetY(_vPos1));
		m_pEffect[VFX_KORSICA_WIND1]->Set_Pos(_vPos);
		m_pEffect[VFX_KORSICA_WIND1]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_KORSICA_WIND0_END", nullptr, [=]() {
		m_pEffect[VFX_KORSICA_WIND0]->DissolveStop(0.5f);
		}, nullptr);
	pModel->AddListener(L"VFX_CP_KORSICA_WIND1_END", nullptr, [=]() {
		m_pEffect[VFX_KORSICA_WIND1]->DissolveStop(0.5f);
		}, nullptr);

	pModel->AddListener(L"VFX_CP_SLAM", nullptr, [=]() {
		VFX_SetBonePos(VFX_SLAM, m_wpModel.lock()->FindBoneWithName(L"prop_01"));
		m_pEffect[VFX_SLAM]->Restart();
		}, nullptr);

	pModel->AddListener(L"VFX_CP_PEPP_BEAM_SRT", nullptr, [=]() {
		auto _desc0 = m_pTrashGuitar->GetBone(L"parts_38")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		auto _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*> (_desc0.m_CombinedTransformationMatrix.m[3]));
		auto _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*> (_desc1.m_CombinedTransformationMatrix.m[3]));
		m_pEffect[VFX_BEAM]->Set_Dir(_vPos1 - _vPos0);
		m_pEffect[VFX_BEAM]->Set_Pos(_vPos0);
		m_pEffect[VFX_BEAM]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_BEAM_END", nullptr, [=]() {
		m_pEffect[VFX_BEAM]->DissolveStop(0.5f);
		}, nullptr);


	pModel->AddListener(L"VFX_CP_PEPP_YEAH", nullptr, [=]() {
		auto _desc0 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		m_pEffect[VFX_PEPP_YEAH]->Set_Pos(_vPos0);
		m_pEffect[VFX_PEPP_YEAH]->Set_Dir(XMVectorSetY(_vPos1 - _vPos0, 0.f));
		m_pEffect[VFX_PEPP_YEAH]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_SWING", nullptr, [=]() {
		auto _desc0 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		m_pEffect[VFX_PEPP_SWING]->Set_Pos(_vPos0);
		m_pEffect[VFX_PEPP_SWING]->Set_Dir(XMVectorSetY(_vPos1 - _vPos0, 0.f));
		m_pEffect[VFX_PEPP_SWING]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_TURN_SRT", nullptr, [=]() {
		auto _desc0 = m_wpModel.lock()->FindBoneWithName(L"origin")->GetDesc();
		auto _desc1 = m_wpModel.lock()->FindBoneWithName(L"prop_01")->GetDesc();
		_vector _vPos0 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc0.m_CombinedTransformationMatrix.m[3]));
		_vector _vPos1 = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc1.m_CombinedTransformationMatrix.m[3]));
		m_pEffect[VFX_PEPP_TURN]->Set_Pos(_vPos0);
		m_pEffect[VFX_PEPP_TURN]->Set_Dir(XMVectorSetY(_vPos1 - _vPos0, 0.f));
		m_pEffect[VFX_PEPP_TURN]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_TURN_END", nullptr, [=]() {
		m_pEffect[VFX_PEPP_TURN]->DissolveStop(0.2f);
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_HIT0", nullptr, [=]() {
		VFX_SetBonePos(VFX_PARTNER_ENEMYHIT, m_pWeaponPartner[0]->GetBone(L"l_toe"));
		m_pEffect[VFX_PARTNER_ENEMYHIT]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_HIT1", nullptr, [=]() {
		VFX_SetBonePos(VFX_MULTI_ENEMYHIT0, m_pWeaponPartner[0]->GetBone(L"r_foot"));
		m_pEffect[VFX_MULTI_ENEMYHIT0]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_HIT2", nullptr, [=]() {
		VFX_SetBonePos(VFX_MULTI_ENEMYHIT1, m_pWeaponPartner[0]->GetBone(L"r_foot"));
		m_pEffect[VFX_MULTI_ENEMYHIT1]->Restart();
		}, nullptr);
	pModel->AddListener(L"VFX_CP_PEPP_HIT3", nullptr, [=]() {
		VFX_SetBonePos(VFX_MULTI_ENEMYHIT2, m_pWeaponPartner[0]->GetBone(L"r_foot"));
		m_pEffect[VFX_MULTI_ENEMYHIT2]->Restart();
		}, nullptr);
}
void CPlayer::Set_Timer()
{
	float beatHitTime = 60.f / m_pGameInstance->GetBPM();


	m_pDashCoolDownTimer = m_pGameInstance->CreateTimerWithDuration(m_fDashCoolTime, false, nullptr, false);
	m_pMagnetWaitTimer = m_pGameInstance->CreateTimerWithDuration(m_fMagnetWaitTime, false, nullptr, false);
	m_pComboTimer = m_pGameInstance->CreateTimerWithDuration(m_fComboStayTime, false, [=]() {
		ResetCombo();
		}, false);

	m_pRestTimer = m_pGameInstance->CreateTimerWithDuration(m_fOnebeatTime, false, [=]() {m_bRestTimerActive = false; }, false);
	m_pBeatHitTimer = m_pGameInstance->CreateTimerWithDuration(
		beatHitTime * 3.f, false, [=]() {m_eRhythmResult = ERhythmResult::None; m_bFirstRhythmRes = true; m_eBeatHitType = BH_NONE; m_bInputPartner = false;
	m_pUCEffect->MagnetRailEnd(); m_bFirstbeatSFX = false; m_bFirstbeatSFX2 = false;
		}, false);
	m_pHitTimer = m_pGameInstance->CreateTimerWithDuration(m_fHitTime, false, [=]()
		{
			SetBoolElement("HIT", false);
		}, false);

	m_pFallRespawnTimer = m_pGameInstance->CreateTimerWithDuration(m_fFalltoDieWaitTime, false, [=]() {	SetPos(m_vRespawnPos); }, false);

	m_pScoreMulTimer = m_pGameInstance->CreateTimerWithDuration(m_fScoreMulTime, false, [=]() {m_fScoreMul = 1.f; }, false);
	m_pAtkCntTimer = m_pGameInstance->CreateTimerWithDuration(m_fAtkHoldTime, false, [=]() { m_PlayerState.m_iComboCnt = 0; }, false);
	m_pRP_StartUI_Timer = m_pGameInstance->CreateTimerWithDuration(m_fRP_StatUI_Time, false, [=]() {m_pUCBeathit->Set_Parry_BHEffect(); }, false);
	m_pColorReserveTimer = m_pGameInstance->CreateTimerWithDuration(beatHitTime, false, nullptr, false);
	m_pIgniteTimer = m_pGameInstance->CreateTimerWithDuration(3.5f, false, [=]() {EndIgnite(); }, false);
	m_pFlickerTimer = m_pGameInstance->CreateTimerWithDuration(m_fFlickerTime, false, [=]() {Flicker_RenderOn(); m_bInvincible = false; ActiveGuitar(); }, false);
	m_pJustCntTimer = m_pGameInstance->CreateTimerWithDuration(m_fOnebeatTime * 3, false, [=]() {m_iCurJustCnt = 0; }, false);
	m_pBurn1EffectTimer = m_pGameInstance->CreateTimerWithDuration(m_fVFXBurn1LoopTime, true, [=]()
		{
			m_iBurnIdx1++;
			if (m_iBurnIdx1 >= m_iBurnPoolSize)
				m_iBurnIdx1 = 0;
			m_vecBurn1VFX[m_iBurnIdx1]->Set_Pos(Get_Center());
			m_vecBurn1VFX[m_iBurnIdx1]->Restart();
		}, false);
	m_pBurn2EffectTimer = m_pGameInstance->CreateTimerWithDuration(m_fVFXBurn2LoopTime, true, [=]()
		{
			m_iBurnIdx2++;
			if (m_iBurnIdx2 >= m_iBurnPoolSize)
				m_iBurnIdx2 = 0;
			m_vecBurn2VFX[m_iBurnIdx2]->Set_Pos(Get_Center());
			m_vecBurn2VFX[m_iBurnIdx2]->Restart();
		}, false);
	m_pBreakDownDelayTimer = m_pGameInstance->CreateTimerWithDuration(0.2f, false, nullptr, false);

	m_pBreakDownEffectTimer = m_pGameInstance->CreateTimerWithDuration(0.03f, true, [=]()
		{
			auto mat = m_wpModel.lock()->FindBoneWithName(L"Guitar_attach_Socket")->GetDesc().m_CombinedTransformationMatrix;
			_vector vPos = XMVectorSet(mat._41, mat._42, mat._43, 1.f);
			//auto vPos = m_wpAttackCollider.lock()->Get_Center();


			m_iBreakDownIdx++;
			if (m_iBreakDownIdx >= m_iBreakDownPoolSize)
				m_iBreakDownIdx = 0;
			m_vecBreakDownVFX[m_iBreakDownIdx]->Set_Pos(vPos);
			m_vecBreakDownVFX[m_iBreakDownIdx]->Restart();
		}, false);

	m_pIgniteDamageTimer = m_pGameInstance->CreateTimerWithDuration(1.f, true, [=]()
		{
			m_PlayerState.m_fCurHp -= 3.f;
		}, false);

	m_pAirComboDelayTimer = m_pGameInstance->CreateTimerWithDuration(0.5f, false, [=]()
		{
			m_bIsAirCombo = false;
		}, false);


	m_pMulAtkTimer = m_pGameInstance->CreateTimerWithDuration(m_fMulAtkTime, true, [=]()
		{
			FDamageInfo _damageInfo = {};
			_damageInfo.pSourceObj = this;
			_damageInfo.fDamage = Calc_Damage(10.f);
			_damageInfo.eAttackType = EAttackType::MULTIHITATK;


			if (m_pTarget && m_pMulAtk_StayTimer->IsActive())
				m_pTarget->OnDamage(_damageInfo);

			PrintDebug("MULATK!");

		}, false);
	m_pMulAtk_StayTimer = m_pGameInstance->CreateTimerWithDuration(1.f, false, [=]()
		{
			m_pMulAtkTimer->SetActive(false);
		}, false);

}

CGameObject* CPlayer::Clone(void* pArg)
{
	return CloneBase<CPlayer>(pArg);
}

void CPlayer::Free()
{
	__super::Free();
	Safe_Release(m_ctxDesc.pBlackboard);
	if (m_pGamePlay)
		Safe_Release(m_pGamePlay);

	if (m_pQTEManager) Safe_Release(m_pQTEManager);
}

HRESULT CPlayer::Reset()
{
	return S_OK;
}

HRESULT CPlayer::Save(void* _pDesc, _uint& _iSize) const
{
	_iSize = 0;
	return S_OK;
}

HRESULT CPlayer::Load(void* _pDesc)
{
	return S_OK;
}

HRESULT CPlayer::Add_InitComponents()
{
	CHKFAIL(__super::Add_InitComponents())

		return S_OK;
}

void CPlayer::SetRhythmParry(CBone* pBone, _int iDir)
{
	if (pBone == nullptr)
		ResetRp_WA();
	m_pRPBone = pBone;
	m_iRP_Dir = iDir;
}

void CPlayer::EnterRP_WA(_bool firstRp)
{
	ResetRp_WA();
	SetBoolElement("ENTER_RP_WA", firstRp);
	m_upAnimController->SetLayerIdx("IDLE_Idx", 4);
	m_pUCPlayerStatus->Set_PartnerLockOn();
	SetBoolElement("InputKey", false);
	m_bisRp = true;
}

void CPlayer::TryRp_WA()
{
	ResetRp_WA();
	SetBoolElement("TRY_RP_WA", true);
	m_upAnimController->SetLayerIdx("IDLE_Idx", 0);
}
void CPlayer::MissRp()
{
	ResetRp_WA();
	m_pUCPlayerStatus->Set_PartnerLockOff();
}

void CPlayer::SuccessRP_WA()
{
	ResetRp_WA();
	SetBoolElement("Parry", false);
	SetBoolElement("SUCCESS_RP_WA", true);
	m_pUCPlayerStatus->Set_PartnerLockOff();
	m_bisRp = false;
}

void CPlayer::Activate_RP_START_UI(_float fDuration)
{
	m_bKeyLock = false;
	m_pRP_StartUI_Timer->SetDuration(fDuration);
	m_pRP_StartUI_Timer->Restart();
	ResetRp_WA();
}

void CPlayer::ResetRp_WA()
{
	SetBoolElement("ENTER_RP_WA", false);
	SetBoolElement("TRY_RP_WA", false);
	SetBoolElement("SUCCESS_RP_WA", false);
}

void CPlayer::EnterRP_Mimosa()
{
	SetBoolElement("SUCCESS_RP_MIMOSA", false);
	
	SetBoolElement("ENTER_RP_MIMOSA", true);
	m_pUCPlayerStatus->Set_PartnerLockOn();
	SetBoolElement("InputKey", false);
	m_bCanMove = false;
	m_bisRp = true;
	m_pUCBeathit->Set_LockBeathit(true);
}

void CPlayer::SuccessRP_Mimosa()
{
	SetBoolElement("ENTER_RP_MIMOSA", false);
	SetBoolElement("SUCCESS_RP_MIMOSA", true);
	m_pUCPlayerStatus->Set_PartnerLockOff();
	m_bisRp = false;
	m_pUCBeathit->Set_LockBeathit(false);
}

void CPlayer::EnterRP_Kale(_bool isLast)
{
	if (isLast)
	{
		SetBoolElement("Enter_LastRP_Kale", true);
	}
	else
	{
		SetBoolElement("Enter_RP_Kale", true);
	}

	m_pUCBeathit->Set_LockBeathit(true);
	m_pUCPlayerStatus->Set_PartnerLockOn();
	SetBoolElement("InputKey", false);
	m_bCanMove = false;
	m_bisRp = true;
	m_bisDodgeRp = true;
	m_bKeyLock = true;
	m_upAnimController->SetLayerIdx("IDLE_Idx", 4);
}

void CPlayer::End_RP_Kale()
{
	m_pUCBeathit->Set_LockBeathit(false);
	m_pUCPlayerStatus->Set_PartnerLockOff();
	m_bCanMove = true;
	m_bisRp = false;
	m_bisDodgeRp = false;
	m_upAnimController->SetLayerIdx("IDLE_Idx", 0);
}

void CPlayer::SetRhythmParryPos()
{
	if (!m_pRPBone) return;

	auto _desc = m_pRPBone->GetDesc();

	m_wpMainTransformCom.lock()->Set_LocalState(
		STATE::POSITION,
		XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[3])));
	m_wpMainTransformCom.lock()->Aim(
		XMVectorSetY(XMLoadFloat4(reinterpret_cast<const _float4*>(&_desc.m_CombinedTransformationMatrix.m[0])), 0.f) * (float)m_iRP_Dir
	);

	m_bCanMove = false;
}

void CPlayer::SetPartner()
{
	CGameObject::LoadPrototype(
		L"Prototype_Partner_Peppermint",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&pPartners[0])
	);
	pPartners[0]->SetLevelObject();

	CGameObject::LoadPrototype(
		L"Prototype_Partner_Macaron",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&pPartners[1])
	);
	pPartners[1]->SetLevelObject();

	CGameObject::LoadPrototype(
		L"Prototype_Partner_Korsica",
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		reinterpret_cast<CGameObject**>(&pPartners[2])
	);
	pPartners[2]->SetLevelObject();


	for (int i = 0; i < 3; i++)
	{
		pPartners[i]->m_upPSFSM->AddStateFunc((_int)EPartnerState::BATTLE, EKEYACTIONSTATE::ENTER,
			[=](float)
			{
				m_pUCPlayerStatus->Set_PartnerState(CUI_PartnerStatus::UI_PARTNER_ACTIVE::ACTIVATING, i);
			});
		pPartners[i]->m_upPSFSM->AddStateFunc((_int)EPartnerState::BATTLE, EKEYACTIONSTATE::EXIT,
			[=](float)
			{
				//CoolDownPepp();
			});

	}
}

void CPlayer::ChangePartner()
{
	if (m_pGameInstance->IsKeyState(DIK_F, EKEYACTIONSTATE::ENTER)) {
		m_iPartnerIdx = m_iPartnerIdx + 1;
		m_iPartnerIdx %= CPartner::PARTNERCOUNT;
		m_p808_Ball->SetPartnerIdx(m_iPartnerIdx);
		if (m_pUCPlayerStatus && !m_pUCPlayerStatus->Get_IsLock())
			m_pUCPlayerStatus->ChangePartner();
	}
	for (int i = 0; i < CPartner::PARTNERCOUNT; i++)
	{
		pPartners[i]->SetSelected(i == m_iPartnerIdx);
	}

}

void CPlayer::SetWeapon()
{
	auto _pBone = m_wpModel.lock()->FindBoneWithName(L"Guitar_attach_Socket");
	m_pGuitar->Set_Bone(_pBone);
	m_pGuitar->Set_Animator(L"../AnimNodes/Weapon/Chai/Animator_Player_Chai_Guitar.json");

	//m_pTrashGuitar->Set_Bone(_pBone);
	m_pTrashGuitar->Set_Animator(L"../AnimNodes/Weapon/Chai/Animator_Player_Trash_Guitar.json");

	m_wpAttackCollider = ConvertWPComponent<CCollidor>(m_pGuitar->Get_Component<CCollidor>());

	m_stDamageInfo.pSourceObj = this;

	m_wpAttackCollider.lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());


			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(m_stDamageInfo);
					HitEnemy();
					if (m_bisEnemyRot && m_vecRotEnemy.size() <= 2)
					{
						auto iter = std::find(
							m_vecRotEnemy.begin(),
							m_vecRotEnemy.end(),
							pEnemy
						);

						if (iter == m_vecRotEnemy.end())
						{
							m_vecRotEnemy.push_back(pEnemy);
						}
					}

				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					if (pArm->OnDamage(m_stDamageInfo.fDamage)) {
						VFX_SetBonePos(VFX_PLAYER_ENEMYHIT, m_pGuitar->GetBone(L"g_05"));
						m_pEffect[VFX_PLAYER_ENEMYHIT]->Restart();
						HitEnemy();
					}

				}
				else
				{

					auto pShield = dynamic_cast<CEnemyShield*>(pMonsterCol->GetGameObject());
					if (pShield) {
						//hello
					}
					else {
						/*auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
						pInteractObj->OnDamage(m_stDamageInfo);*/
						auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
						if (pInteractObj->OnDamage(m_stDamageInfo)) {
							VFX_SetBonePos(VFX_PLAYER_ENEMYHIT, m_pGuitar->GetBone(L"g_05"));
							m_pEffect[VFX_PLAYER_ENEMYHIT]->Restart();
							HitEnemy();
						}
					}
				}
			}

		});

	m_wpAttackCollider.lock()->SetActive(false);
}


HRESULT CPlayer::LoadEffect(VFX eID, const _wstring& strPrototypeTag)
{
	CGameObject* pEffectObject = nullptr;
	HRESULT hr = CGameObject::LoadPrototype(strPrototypeTag, m_pGameInstance->GetLevelDesc().nextLevelTag, &pEffectObject);
	CHKFAIL(hr);

	m_pEffect[eID] = dynamic_cast<CEffectRoot*>(pEffectObject);
	if (m_pEffect[eID] == nullptr)
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Set_VFX()
{
	CHKFAIL(LoadEffect(VFX_IDLE, L"VFX_Player_Snap"));
	CHKFAIL(LoadEffect(VFX_REST, L"VFX_Player_Rest"));
	CHKFAIL(LoadEffect(VFX_JUMP, L"VFX_JumpImpact"));
	CHKFAIL(LoadEffect(VFX_DOUBLEJUMP, L"VFX_DoubleJump"));
	CHKFAIL(LoadEffect(VFX_JUMPFONT, L"VFX_Jump_Font"));
	CHKFAIL(LoadEffect(VFX_LAND, L"VFX_LandingSmoke"));
	CHKFAIL(LoadEffect(VFX_RHYTHMDASHSWING, L"VFX_RhythmDash_Swing"));
	CHKFAIL(LoadEffect(VFX_RHYTHMDASHIMPACT, L"VFX_RhythmDash_Impact"));
	CHKFAIL(LoadEffect(VFX_ATTACK_X, L"VFX_Player_Attack_X"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XX, L"VFX_Player_Attack_XX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XXX, L"VFX_Player_Attack_XXX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XXXX, L"VFX_Player_Attack_XXXX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_X_X, L"VFX_Player_Attack_X_X"));
	CHKFAIL(LoadEffect(VFX_ATTACK_X_XX, L"VFX_Player_Attack_X_XX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XX_X, L"VFX_Player_Attack_XX_X"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XX_XX, L"VFX_Player_Attack_XX_XX"));
	CHKFAIL(LoadEffect(VFX_BEATHIT_XX_XXX, L"VFX_Player_Attack_TuneUp"));
	CHKFAIL(LoadEffect(VFX_GAINTORNADO_SWING, L"VFX_Player_Attack_TuneUp"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XX_XXX, L"VFX_Player_Attack_XX_XXX"));
	CHKFAIL(LoadEffect(VFX_BEATHIT_XXXX, L"VFX_Player_BeatHit_XXXX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_AIR_X, L"VFX_Player_Attack_Air_X"));
	CHKFAIL(LoadEffect(VFX_ATTACK_AIR_XX, L"VFX_Player_Attack_Air_XX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_AIR_XXX, L"VFX_Player_Attack_Air_XXX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_AIR_XXXX, L"VFX_Player_Attack_Air_XXXX"));
	CHKFAIL(LoadEffect(VFX_BEATHIT_AIR_XXXX, L"VFX_Player_BeatHit_Air_XXXX"));
	CHKFAIL(LoadEffect(VFX_ATTACK_Y, L"VFX_Player_Attack_Y"));
	CHKFAIL(LoadEffect(VFX_ATTACK_YY, L"VFX_Player_Attack_YY"));
	CHKFAIL(LoadEffect(VFX_ATTACK_YYY, L"VFX_Player_Attack_YYY"));
	CHKFAIL(LoadEffect(VFX_BEATHIT_YYY, L"VFX_Player_BeatHit_YYY"));
	CHKFAIL(LoadEffect(VFX_ATTACK_AIR_Y, L"VFX_Player_Attack_Air_Y"));
	CHKFAIL(LoadEffect(VFX_MAGNET, L"VFX_Player_Magnet"));
	CHKFAIL(LoadEffect(VFX_MAGNET_TRAIL, L"VFX_Trail_Magnet"));
	CHKFAIL(LoadEffect(VFX_HIT, L"VFX_Player_Hit"));
	CHKFAIL(LoadEffect(VFX_PARRY, L"VFX_Parry"));
	CHKFAIL(LoadEffect(VFX_LASER, L"VFX_Mimosa_Attack_04_Laser"));

	CHKFAIL(LoadEffect(VFX_DANCE_A, L"VFX_Player_RhyrhmBattle_Success_A"));
	CHKFAIL(LoadEffect(VFX_DANCE_LB, L"VFX_Player_RhyrhmBattle_Success_LB"));
	CHKFAIL(LoadEffect(VFX_DANCE_RB, L"VFX_Player_RhyrhmBattle_Success_RB"));
	CHKFAIL(LoadEffect(VFX_DANCE_FAIL, L"VFX_Player_RhythmBattle_Fail"));
	CHKFAIL(LoadEffect(VFX_DANCE_SUCCESS, L"VFX_Player_RhythmBattle_Success"));
	CHKFAIL(LoadEffect(VFX_RAILSPEEDLINE, L"VFX_RailSprint"));
	CHKFAIL(LoadEffect(VFX_BATTLE_START, L"VFX_Player_BattleStart"));
	CHKFAIL(LoadEffect(VFX_BATTLE_START_GUITAR, L"VFX_Guitar_BattleStart"));
	CHKFAIL(LoadEffect(VFX_SP_808, L"VFX_Player_SpecialAttack_GigawattCatAttack"));
	CHKFAIL(LoadEffect(VFX_HIBIKI, L"VFX_Player_SpecialAttack_Hibiki"));
	CHKFAIL(LoadEffect(VFX_GAINTORNADO, L"VFX_Player_SpecialAttack_Hibiki"));
	CHKFAIL(LoadEffect(VFX_HIBIKI_END, L"VFX_Player_SpecialAttack_Hibiki_End"));
	CHKFAIL(LoadEffect(VFX_HIBIKI_GUITAR, L"VFX_Guitar_SpecialAttack_Hibiki"));

	CHKFAIL(LoadEffect(VFX_BURN_01, L"VFX_Player_Burn_01"));
	CHKFAIL(LoadEffect(VFX_BURN_01_1, L"VFX_Player_Burn_01"));
	CHKFAIL(LoadEffect(VFX_BURN_01_2, L"VFX_Player_Burn_01"));
	CHKFAIL(LoadEffect(VFX_BURN_01_3, L"VFX_Player_Burn_01"));

	CHKFAIL(LoadEffect(VFX_BURN_02, L"VFX_Player_Burn_02"));
	CHKFAIL(LoadEffect(VFX_BURN_02_1, L"VFX_Player_Burn_02"));
	CHKFAIL(LoadEffect(VFX_BURN_02_2, L"VFX_Player_Burn_02"));
	CHKFAIL(LoadEffect(VFX_BURN_02_3, L"VFX_Player_Burn_02"));

	CHKFAIL(LoadEffect(VFX_ATTACK_XY, L"VFX_Player_Attack_XY"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XYmY, L"VFX_Player_Attack_XYmY"));
	CHKFAIL(LoadEffect(VFX_ATTACK_XYX, L"VFX_Player_Attack_XYX"));

	CHKFAIL(LoadEffect(VFX_ATTACK_XYY, L"VFX_Player_Attack_XYY"));


	CHKFAIL(LoadEffect(VFX_Player_BeatHit_XYY, L"VFX_Player_BeatHit_XYY"));
	CHKFAIL(LoadEffect(VFX_Player_Attack_XYXX_Entry, L"VFX_Player_Attack_XYXX_Entry"));
	CHKFAIL(LoadEffect(VFX_Player_Attack_XYXX_Mid, L"VFX_Player_Attack_XYXX_Mid"));
	CHKFAIL(LoadEffect(VFX_Player_BeatHit_XYXX, L"VFX_Player_BeatHit_XYXX"));
	CHKFAIL(LoadEffect(VFX_Player_BeatHit_XYmY, L"VFX_Player_BeatHit_XYmY"));
	CHKFAIL(LoadEffect(VFX_Player_BeatHit_XYXX_KRASH, L"VFX_Player_BeatHit_XYXX_KRASH"));

	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY, L"VFX_Guitar_Attack_XYY"));
	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY_1, L"VFX_Guitar_Attack_XYY"));
	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY_2, L"VFX_Guitar_Attack_XYY"));
	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY_3, L"VFX_Guitar_Attack_XYY"));
	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY_4, L"VFX_Guitar_Attack_XYY"));
	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY_5, L"VFX_Guitar_Attack_XYY"));
	CHKFAIL(LoadEffect(VFX_Guitar_Attack_XYY_6, L"VFX_Guitar_Attack_XYY"));

	CHKFAIL(LoadEffect(VFX_Player_Attack_XYXX_End, L"VFX_Player_Attack_XYXX_End"));
	CHKFAIL(LoadEffect(VFX_Player_HalftoneStep, L"VFX_Player_HalftoneStep"));
	CHKFAIL(LoadEffect(VFX_RailSuccess, L"VFX_Raill_Success"));

	CHKFAIL(LoadEffect(VFX_PLAYER_ENEMYHIT, L"VFX_Enemy_Hit"));
	CHKFAIL(LoadEffect(VFX_PARTNER_ENEMYHIT, L"VFX_Enemy_Hit"));

	CHKFAIL(LoadEffect(VFX_MULTI_ENEMYHIT0, L"VFX_Enemy_Hit"));
	CHKFAIL(LoadEffect(VFX_MULTI_ENEMYHIT1, L"VFX_Enemy_Hit"));
	CHKFAIL(LoadEffect(VFX_MULTI_ENEMYHIT2, L"VFX_Enemy_Hit"));

	CHKFAIL(LoadEffect(VFX_PARTNER_JUMPIMPACT, L"VFX_JumpImpact"));
	CHKFAIL(LoadEffect(VFX_PLAYER_DJ, L"VFX_Player_DJ"));
	CHKFAIL(LoadEffect(VFX_MACARON_LANDINGSMOKE, L"VFX_CH2000_LandingSmoke"));

	CHKFAIL(LoadEffect(VFX_PLAYER_SHOOT, L"VFX_DOUBLEBASEDROP_SWING"));

	CHKFAIL(LoadEffect(VFX_MACARON_TURN, L"VFX_DOUBLEBASEDROP_SWING"));
	CHKFAIL(LoadEffect(VFX_MACARON_WHIRL, L"VFX_CH2000_JamCombo_Swing"));
	CHKFAIL(LoadEffect(VFX_MACARON_FONT, L"VFX_CH2000_JamCombo_Font"));
	CHKFAIL(LoadEffect(VFX_MACARON_WAVE, L"VFX_CH2000_JamCombo_Wave"));

	CHKFAIL(LoadEffect(VFX_KORSICA_SNAP, L"VFX_TORNADOLIFT_SNAP"));
	CHKFAIL(LoadEffect(VFX_KORSICA_SWING, L"VFX_TORNADOLIFT_SWINGLOOP"));
	CHKFAIL(LoadEffect(VFX_KORSICA_WIND0, L"VFX_TORNADOLIFT_WIND0"));
	CHKFAIL(LoadEffect(VFX_KORSICA_WIND1, L"VFX_TORNADOLIFT_WIND1"));

	CHKFAIL(LoadEffect(VFX_PEPP_SWING, L"VFX_CH1000_JamCombo_Swing"));
	CHKFAIL(LoadEffect(VFX_PEPP_YEAH, L"VFX_CH1000_JamCombo_Font"));
	CHKFAIL(LoadEffect(VFX_PEPP_TURN, L"VFX_CH1000_JamCombo_Turn"));

	CHKFAIL(LoadEffect(VFX_SLAM, L"VFX_CH4000_JamCombo_Air_Impact"));
	CHKFAIL(LoadEffect(VFX_BEAM, L"VFX_CH1000_JamCombo_Air"));

	CHKFAIL(LoadEffect(VFX_MULTI_PEPPHIT0, L"VFX_CH1000_Hit"));
	CHKFAIL(LoadEffect(VFX_MULTI_PEPPHIT1, L"VFX_CH1000_Hit"));
	CHKFAIL(LoadEffect(VFX_MULTI_PEPPHIT2, L"VFX_CH1000_Hit"));

	auto pPropBone = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	for (int i = 0; i < VFX_END; i++)
	{
		if (m_pEffect[i] == nullptr)
			continue;

		m_pEffect[i]->Stop();
		m_pEffect[i]->SetLevelObject();

		if (pPropBone != nullptr && !in(i,
			{ VFX_MAGNET_TRAIL , VFX_LASER ,VFX_Guitar_Attack_XYY,
			VFX_PLAYER_ENEMYHIT, VFX_MULTI_ENEMYHIT0, VFX_MULTI_ENEMYHIT1, VFX_MULTI_ENEMYHIT2,
			VFX_MACARON_LANDINGSMOKE,
			VFX_PLAYER_DJ, VFX_PLAYER_SHOOT,
			VFX_PARTNER_ENEMYHIT, VFX_PARTNER_JUMPIMPACT,
			VFX_KORSICA_SNAP, VFX_KORSICA_SWING, VFX_KORSICA_WIND0, VFX_KORSICA_WIND1,
			VFX_MACARON_TURN, VFX_MACARON_WHIRL, VFX_MACARON_FONT, VFX_MACARON_WAVE,
			VFX_PEPP_SWING, VFX_PEPP_YEAH, VFX_PEPP_TURN,
			VFX_SLAM, VFX_BEAM,
			VFX_MULTI_PEPPHIT0, VFX_MULTI_PEPPHIT1, VFX_MULTI_PEPPHIT2, VFX_GAINTORNADO,VFX_GAINTORNADO_SWING,
			VFX_Player_BeatHit_XYXX
			}
		))
		{
			if (i == VFX_Player_Attack_XYXX_Entry || i == VFX_Player_Attack_XYXX_Mid)// ||
				//i == VFX_Player_BeatHit_XYXX)
			{
				m_pEffect[i]->Set_Bone(pPropBone, CEffectRoot::BONE_ATTACH_POSITION_ONLY);
				continue;
			}

			m_pEffect[i]->Set_Bone(pPropBone);
		}

	}

	//m_pEffect[VFX_IDLE]->Set_Bone(m_pVFX_TrailBone[TB_LHAND]);


	auto pHandBone = m_wpModel.lock()->FindBoneWithName(L"r_gimmick");

	if (m_pEffect[VFX_MAGNET_TRAIL] != nullptr)
	{
		vector<CEffectPlayable*> vecChildEffects;
		m_pEffect[VFX_MAGNET_TRAIL]->GetChildEffects(vecChildEffects, true);

		for (CEffectPlayable* pEffect : vecChildEffects)
		{
			if (auto pTrail = dynamic_cast<CVFX_Trail*>(pEffect))
			{
				m_pMagnetTrail = pTrail;
				break;
			}
		}

		if (m_pMagnetTrail)
		{
			m_pMagnetTrail->Stop();
			m_pMagnetTrail->SetFollowTarget(Get_MainTransform(), { 0.f, 1.f, 0.f });
			m_pMagnetTrail->Set_Bone(pHandBone);
		}
	}
	if (m_pEffect[VFX_MAGNET] != nullptr)
		m_pEffect[VFX_MAGNET]->Set_Bone(pHandBone);

	HRESULT hr = S_OK;
	const _tchar* arrTrailPrototype[TB_END] =
	{
		L"VFX_Trail_Player_O", // TB_LHAND
		L"VFX_Trail_Player_G", // TB_RHAND
		L"VFX_Trail_Player_Y", // TB_CHEST
		L"VFX_Trail_Player_B", // TB_LKNEE
		L"VFX_Trail_Player_B", // TB_RKNEE
		L"VFX_Trail_Player_R", // TB_LFOOT
		L"VFX_Trail_Player_R"  // TB_RFOOT
	};

	m_vecBurn1VFX.resize(4);
	m_vecBurn2VFX.resize(4);
	m_vecBreakDownVFX.resize(m_iBreakDownPoolSize);

	m_vecBurn1VFX[0] = m_pEffect[VFX_BURN_01];
	m_vecBurn1VFX[1] = m_pEffect[VFX_BURN_01_1];
	m_vecBurn1VFX[2] = m_pEffect[VFX_BURN_01_2];
	m_vecBurn1VFX[3] = m_pEffect[VFX_BURN_01_3];

	m_vecBurn2VFX[0] = m_pEffect[VFX_BURN_02];
	m_vecBurn2VFX[1] = m_pEffect[VFX_BURN_02_1];
	m_vecBurn2VFX[2] = m_pEffect[VFX_BURN_02_2];
	m_vecBurn2VFX[3] = m_pEffect[VFX_BURN_02_3];

	m_vecBreakDownVFX[0] = m_pEffect[VFX_Guitar_Attack_XYY];
	m_vecBreakDownVFX[1] = m_pEffect[VFX_Guitar_Attack_XYY_1];
	m_vecBreakDownVFX[2] = m_pEffect[VFX_Guitar_Attack_XYY_2];
	m_vecBreakDownVFX[3] = m_pEffect[VFX_Guitar_Attack_XYY_3];
	m_vecBreakDownVFX[4] = m_pEffect[VFX_Guitar_Attack_XYY_4];
	m_vecBreakDownVFX[5] = m_pEffect[VFX_Guitar_Attack_XYY_5];
	m_vecBreakDownVFX[6] = m_pEffect[VFX_Guitar_Attack_XYY_6];


	for (_int i = 0; i < TB_END; ++i)
	{
		CGameObject* pTrailRootObject = nullptr;
		hr = CGameObject::LoadPrototype(arrTrailPrototype[i], m_pGameInstance->GetLevelDesc().nextLevelTag, &pTrailRootObject);
		CHKFAIL(hr);

		m_pEffectTrailRoot[i] = dynamic_cast<CEffectRoot*>(pTrailRootObject);
		if (m_pEffectTrailRoot[i] == nullptr)
			return E_FAIL;

		m_pEffectTrailRoot[i]->SetLevelObject();
		m_pEffectTrailRoot[i]->Stop();

		vector<CEffectPlayable*> vecChildEffects;
		m_pEffectTrailRoot[i]->GetChildEffects(vecChildEffects, true);

		m_pEffectTrail[i] = nullptr;
		for (CEffectPlayable* pEffect : vecChildEffects)
		{
			if (auto pTrail = dynamic_cast<CVFX_Trail*>(pEffect))
			{
				m_pEffectTrail[i] = pTrail;
				break;
			}
		}

		if (m_pEffectTrail[i] == nullptr)
			return E_FAIL;
	}

	return S_OK;
}
void CPlayer::Set_VFXBone()
{
	auto pLHand = m_wpModel.lock()->FindBoneWithName(L"l_hand");
	auto pRHand = m_wpModel.lock()->FindBoneWithName(L"r_hand");
	auto pChest = m_wpModel.lock()->FindBoneWithName(L"spine_02");
	auto pLKnee = m_wpModel.lock()->FindBoneWithName(L"l_sup_keen_01");
	auto pRKnee = m_wpModel.lock()->FindBoneWithName(L"r_sup_keen_01");
	auto pLFoot = m_wpModel.lock()->FindBoneWithName(L"LeftFootSocket");
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"RightFootSocket");

	m_pVFX_TrailBone[TB_LHAND] = pLHand;
	m_pVFX_TrailBone[TB_RHAND] = pRHand;
	m_pVFX_TrailBone[TB_CHEST] = pChest;
	m_pVFX_TrailBone[TB_LKNEE] = pLKnee;
	m_pVFX_TrailBone[TB_RKNEE] = pRKnee;
	m_pVFX_TrailBone[TB_LFOOT] = pLFoot;
	m_pVFX_TrailBone[TB_RFOOT] = pBone;
}

void CPlayer::RestartVFX(VFX eID, _vector pos, _vector look)
{
	if (m_pEffect[eID] == nullptr)
		return;

	VFX_SetPos(eID, pos);
	if (XMVectorGetX(XMVector3LengthSq(look)) > 0.000001f)
		m_pEffect[eID]->Set_Dir(look);
	m_pEffect[eID]->Restart();
}

void CPlayer::RestartVFX_PosBone(VFX eID, _vector look)
{
	if (m_pEffect[eID] == nullptr)
		return;

	if (XMVectorGetX(XMVector3LengthSq(look)) > 0.000001f)
		m_pEffect[eID]->Set_Dir(look);

	m_pEffect[eID]->Restart();
}

void CPlayer::VFX_SetPos(VFX eID, _vector pos)
{
	if (m_pEffect[eID] == nullptr)
		return;

	m_pEffect[eID]->Set_Pos(pos);
}

void CPlayer::VFX_SetLook(VFX eID, _vector Look)
{
	if (m_pEffect[eID] == nullptr)
		return;

	m_pEffect[eID]->Set_Dir(Look);
}

void CPlayer::VFX_SetBone(VFX eID, CBone* pBone)
{
	if (m_pEffect[eID] == nullptr)
		return;

	m_pEffect[eID]->Set_Bone(pBone);
}

void CPlayer::VFX_SetBonePos(VFX eID, CBone* pBone, _vector vOffset)
{
	if (m_pEffect[eID] == nullptr)
		return;

	auto _desc = pBone->GetDesc();
	auto _vPos = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc.m_CombinedTransformationMatrix.m[3]));
	m_pEffect[eID]->Set_Pos(_vPos + vOffset);
}
void CPlayer::VFX_SetBoneDir(VFX eID, CBone* pBone)
{
	if (m_pEffect[eID] == nullptr)
		return;

	auto _desc = pBone->GetDesc();
	auto _vDir = XMLoadFloat4(reinterpret_cast<_float4*>(&_desc.m_CombinedTransformationMatrix.m[0]));

	m_pEffect[eID]->Set_Pos(_vDir);
}

void CPlayer::Stop_VFX()
{
	for (int i = 0; i < VFX_END; i++)
	{
		if (m_pEffect[i] == nullptr)
			continue;

		m_pEffect[i]->Stop();
	}
}

void CPlayer::SetCollider()
{
	m_wpBodyCollider = ConvertWPComponent<CCollidor>(Get_Component(L"Com_Collidor_Main"));

	m_wpBeatHitCollider[BH_AIR_XXXX] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_BEATHIT_AIR_XXXX]->GetEffectCollider(L"Com_Collider_BeatHit_Air_XXXX"));
	m_wpBeatHitCollider[BH_YYY] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_BEATHIT_YYY]->GetEffectCollider(L"Com_Collider_BeatHit_YYY"));
	m_wpBeatHitCollider[BH_XXXX] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_BEATHIT_XXXX]->GetEffectCollider(L"Com_Collider_BeatHit_XXXX"));
	m_wpBeatHitCollider[BH_XXXX_2] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_BEATHIT_XXXX]->GetEffectCollider(L"Com_Collider_BeatHit_XXXX_2"));
	m_wpBeatHitCollider[BH_TUNEUP] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_BEATHIT_XX_XXX]->GetEffectCollider(L"Com_Collider_BeatHit_TuneUp"));
	m_wpBeatHitCollider[BH_SHRED] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_Player_BeatHit_XYmY]->GetEffectCollider(L"Com_Collider_BeatHit_XYmY"));
	m_wpBeatHitCollider[BH_ECOSPLASH] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_Player_BeatHit_XYXX_KRASH]->GetEffectCollider(L"Com_Collider_BeatHit_XYXX"));
	m_wpBeatHitCollider[BH_BREAKDOWN] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_Player_BeatHit_XYY]->GetEffectCollider(L"Com_Collider_BeatHit_XYY"));
	m_wpBeatHitCollider[SP_808] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_SP_808]->GetEffectCollider(L"Com_Collider_808Attack"));
	m_wpBeatHitCollider[SP_HIBIKI_SWING] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_HIBIKI]->GetEffectCollider(L"Com_Collider_Player_Atk_Hibiki"));
	m_wpBeatHitCollider[SP_HIBIKI_WAVE] = ConvertWPComponent<CCollidor>(m_pEffect[VFX_HIBIKI_END]->GetEffectCollider(L"Com_Collider_Player_Atk_Hibiki"));

	m_wpBodyCollider.lock()->Set_Func(EKEYACTIONSTATE::ENTER,
		[&](const CONTACTMF_DESC& mfDesc, CCollidor* pCollidor) {
			if (pCollidor->GetDesc()->m_iCollidorIdx != CT_COMMON_BODY) return;
			_float _fDot = XMVectorGetX(XMVector3Dot(
				XMVectorSet(0.f, 1.f, 0.f, 0.f),
				-XMLoadFloat4(&mfDesc.contacts[0].vNormal)
			));

			if (_fDot > cos(XM_PI * 0.25f)) {
				m_bGrounded = true;
			}
		});

	m_wpBodyCollider.lock()->Set_Func(EKEYACTIONSTATE::STAY,
		[&](const CONTACTMF_DESC& mfDesc, CCollidor* pCollidor) {
			if (pCollidor->GetDesc()->m_iCollidorIdx != CT_COMMON_BODY) return;
			_float _fDot = XMVectorGetX(XMVector3Dot(
				XMVectorSet(0.f, 1.f, 0.f, 0.f),
				-XMLoadFloat4(&mfDesc.contacts[0].vNormal)
			));

			if (_fDot > cos(XM_PI * 0.25f)) {
				m_bGrounded = true;
			}

		});

	m_wpBeatHitCollider[BH_XXXX].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(10.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;

			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}

		});
	m_wpBeatHitCollider[BH_XXXX_2].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(12.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[BH_AIR_XXXX].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(30.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[BH_YYY].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(20.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[BH_TUNEUP].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(32.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[BH_SHRED].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(32.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());
			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[BH_ECOSPLASH].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(42.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[BH_BREAKDOWN].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(22.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();

				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[SP_HIBIKI_WAVE].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(70.f);
			_beathitInfo.eAttackType = EAttackType::KNOCKBACK;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());
			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					pInteractObj->OnDamage(_beathitInfo);
					HitEnemy();
				}

			}
		});
	m_wpBeatHitCollider[SP_HIBIKI_SWING].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			FDamageInfo _beathitInfo = {};
			_beathitInfo.pSourceObj = this;
			_beathitInfo.fDamage = Calc_Damage(20.f);
			_beathitInfo.eAttackType = EAttackType::BEATHIT;
			auto pEnemy = dynamic_cast<CEnemy*>(pMonsterCol->GetGameObject());

			if (pEnemy)
			{
				if (pEnemy->GetIsInvincible())
				{
					SetBoolElement("Knockback", true);
					ResetCombo();
				}
				else
				{
					pEnemy->OnDamage(_beathitInfo);
					HitEnemy();
				}
			}
			else
			{
				auto pArm = dynamic_cast<CKale_Arm*>(pMonsterCol->GetGameObject());

				if (pArm)
				{
					pArm->OnDamage(_beathitInfo.fDamage);
					HitEnemy();
				}
				else
				{
					auto pInteractObj = static_cast<CInteract_Mimosa_Object*>(pMonsterCol->GetGameObject());
					HitEnemy();
					pInteractObj->OnDamage(_beathitInfo);
				}

			}
		});
	m_wpBeatHitCollider[SP_808].lock()->Set_Func(EKEYACTIONSTATE::ENTER, [=](const CONTACTMF_DESC&, CCollidor* pMonsterCol)
		{
			m_fMulAtk_StayTime = 1.f;
			RestartMulAtk(m_fMulAtk_StayTime);
		});
}

void CPlayer::DebugPos()
{
	auto _InputRespawn = m_pGameInstance->IsKeyState(DIK_F1, EKEYACTIONSTATE::ENTER);
	auto _InputSaveRespawn = m_pGameInstance->IsKeyState(DIK_F2, EKEYACTIONSTATE::ENTER);

	if (_InputRespawn && XMVectorGetX(m_vRespawnPos))
	{
		SetPos(m_vRespawnPos);
	}
	if (_InputSaveRespawn)
	{
		m_vRespawnPos = m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION);
	}

}

_vector CPlayer::GetPos()
{
	return  m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION);
}

_vector CPlayer::GetLook()
{
	return m_wpMainTransformCom.lock()->Get_LocalState(STATE::LOOK);
}


_vector CPlayer::Get_Center()
{
	return m_wpBodyCollider.lock()->Get_Center();
}

_vector CPlayer::Get_BonePos(const _wstring strBoneName)
{
	auto pBone = m_wpModel.lock()->FindBoneWithName(strBoneName);
	const auto boneDesc = pBone->GetDesc();
	_vector vBonePos = XMLoadFloat4(reinterpret_cast<const _float4*>(&boneDesc.m_CombinedTransformationMatrix.m[3]));
	return vBonePos;
}



void CPlayer::WeaponChangeAnim(int NodeIdx, float fBlendTime, float fExitRatio, bool bSync)
{
	m_pGuitar->Change_Animation(NodeIdx, fBlendTime, fExitRatio, bSync);
}
void CPlayer::TrashGuitarChangeAnim(int NodeIdx, float fBlendTime, float fExitRatio, bool bSync)
{
	auto _pBone = m_wpModel.lock()->FindBoneWithName(L"Guitar_attach_Socket");
	m_pTrashGuitar->Set_Bone(_pBone);
	m_pTrashGuitar->Priority_Update(0.f);
	m_pTrashGuitar->Update(0.f);
	m_pTrashGuitar->Late_Update(0.f);
	m_pTrashGuitar->SetActive(true);
	m_pTrashGuitar->Change_Animation(NodeIdx, fBlendTime, fExitRatio, bSync);
}
void CPlayer::TrashGuitarChangeAnim(wstring wstr, float fBlendTime, float fExitRatio, bool bSync)
{
	auto partnerBone = m_pWeaponPartner[0]->GetBone(L"Weapon_attach_Socket_L");
	m_pTrashGuitar->Set_Bone(partnerBone);
	m_pTrashGuitar->Priority_Update(0.f);
	m_pTrashGuitar->Update(0.f);
	m_pTrashGuitar->Late_Update(0.f);
	m_pTrashGuitar->SetActive(true);
	m_pTrashGuitar->Change_Animation(wstr, fBlendTime, fExitRatio, bSync);
}

_float3 CPlayer::GetTrailBoneWorldPos(ETRAILBONE eBone) const
{
	_float3 vWorldPos = {};

	CBone* pBone = m_pVFX_TrailBone[eBone];
	if (pBone == nullptr)
		return vWorldPos;

	const auto desc = pBone->GetDesc();

	vWorldPos.x = desc.m_CombinedTransformationMatrix.m[3][0];
	vWorldPos.y = desc.m_CombinedTransformationMatrix.m[3][1];
	vWorldPos.z = desc.m_CombinedTransformationMatrix.m[3][2];

	return vWorldPos;
}

void CPlayer::StartEffectTrail()
{
	for (_int i = 0; i < TB_END; ++i)
	{
		if (m_pEffectTrail[i] == nullptr)
			continue;

		ETRAILBONE eBone = static_cast<ETRAILBONE>(i);
		_float3 vStart = GetTrailBoneWorldPos(eBone);

		m_vEffectTrailStart[i] = vStart;
		m_bEffectTrailPlay[i] = true;

		m_pEffectTrail[i]->SetFollowTarget(XMLoadFloat3(&m_vEffectTrailStart[i]));
		m_pEffectTrail[i]->SetTargetWorldPosition(vStart);

		if (m_pEffectTrailRoot[i] != nullptr)
			m_pEffectTrailRoot[i]->Restart();
	}
}

void CPlayer::UpdateEffectTrail()
{
	for (_int i = 0; i < TB_END; ++i)
	{
		if (!m_bEffectTrailPlay[i])
			continue;

		if (m_pEffectTrail[i] == nullptr)
			continue;

		if (m_pEffectTrail[i]->GetEffectState() == CEffectPlayable::EFFECT_STATE::STOP)
		{
			m_bEffectTrailPlay[i] = false;
			continue;
		}

		ETRAILBONE eBone = static_cast<ETRAILBONE>(i);
		_float3 vTarget = GetTrailBoneWorldPos(eBone);
		_float3 vSource = m_vEffectTrailStart[i];
		const auto& tTrailDesc = m_pEffectTrail[i]->GetDesc();
		const _float fMaxLength = max(0.f, tTrailDesc.fTargetLineMaxLength);

		if (!m_bEffectTrailDash)
		{
			vSource.x = vTarget.x;
			vSource.z = vTarget.z;

			const _float fDeltaY = vTarget.y - m_vEffectTrailStart[i].y;
			if (fMaxLength > 0.f && abs(fDeltaY) > fMaxLength)
				vSource.y = vTarget.y - ((fDeltaY >= 0.f) ? fMaxLength : -fMaxLength);
		}
		else
		{
			_vector vDashDir = XMVectorSetY(m_vDashDir, 0.f);
			if (XMVectorGetX(XMVector3LengthSq(vDashDir)) < 0.000001f)
				vDashDir = XMVectorSetY(m_vLookDir, 0.f);
			if (XMVectorGetX(XMVector3LengthSq(vDashDir)) < 0.000001f)
				vDashDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
			vDashDir = XMVector3Normalize(vDashDir);

			const _vector vStart = XMLoadFloat3(&m_vEffectTrailStart[i]);
			const _vector vTargetVec = XMLoadFloat3(&vTarget);
			_float fTrailLength = XMVectorGetX(XMVector3Dot(vTargetVec - vStart, vDashDir));
			fTrailLength = max(0.f, fTrailLength);

			if (fMaxLength > 0.f)
				fTrailLength = min(fTrailLength, fMaxLength);

			const _vector vSourceVec = vTargetVec - vDashDir * fTrailLength;
			XMStoreFloat3(&vSource, vSourceVec);
		}

		m_pEffectTrail[i]->SetFollowTarget(XMLoadFloat3(&vSource));
		m_pEffectTrail[i]->SetTargetWorldPosition(vTarget);
	}
}


void CPlayer::StopEffectTrail(_bool bImmediately)
{
	if (!bImmediately)
		return;

	for (_int i = 0; i < TB_END; ++i)
	{
		m_bEffectTrailPlay[i] = false;

		if (m_pEffectTrailRoot[i] != nullptr)
			m_pEffectTrailRoot[i]->Stop();
	}
}

void CPlayer::Set_TransitData()
{
	m_TransitData.pGameInstance = m_pGameInstance;
	m_TransitData.pOwner = this;
	m_TransitData.pBB = m_ctxDesc.pBlackboard;
	m_TransitData.pCurState = &m_iCurState;
	m_TransitData.pInputMove = &m_bMoveKeyInput;
	m_TransitData.pIsJump = &m_bIsJump;
	m_TransitData.pIsDash = &m_bisDash;
	m_TransitData.pIsMagnet = &m_bIsMagnet;
	m_TransitData.pIsAttack = &m_bIsAttack;
}

HRESULT CPlayer::Load_Config()
{
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.RunSpeed", &m_fRunSpeed);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.Gravity", &m_fGravity);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.JumpGravity", &m_fJumpGravity);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.JumpForce", &m_fJumpForce);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.Terminalvelocity", &m_fTerminalvelocity);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.MoveDamp", &m_fMoveDamp);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.RotateDamp", &m_fRotateDamp);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.DashDuration", &m_fDashDuration);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.DashCoolTime", &m_fDashCoolTime);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.DashSpeed", &m_fDashSpeed);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.MagnetDuration", &m_fMagnetDuration);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.MagnetWaitTime", &m_fMagnetWaitTime);

	_float  _offsetY, _offsetZ;

	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.EcoOffsetY", &_offsetY);
	m_pGameInstance->ReadSubConfig(L"PlayerDB.json", L"Locomotion.EcoOffsetZ", &_offsetZ);

	_vector vLook = m_wpMainTransformCom.lock()->Get_LocalState(STATE::LOOK);

	vLook = XMVectorSetY(vLook, 0.f);
	vLook = XMVector3Normalize(vLook);
	auto vRight = m_wpMainTransformCom.lock()->Get_LocalState(STATE::RIGHT);
	auto vUp    = m_wpMainTransformCom.lock()->Get_LocalState(STATE::UP);
	_float3 m_vEcosplashOffset2 = {};
	_float3 vBaseOffset = m_vEcosplashOffset2;
	_float3 vFixedOffset = {};

	if (MakeEcosplashGroundOffset(vBaseOffset, vFixedOffset))
	{
		m_vEcosplashOffset2 = vFixedOffset;
	}
	else
	{
		vBaseOffset.x *= -1.f;

		if (MakeEcosplashGroundOffset(vBaseOffset, vFixedOffset))
		{
			m_vEcosplashOffset2 = vFixedOffset;
		}
	}

	_vector vFinalPos =
		GetPos()
		+ vRight * m_vEcosplashOffset2.x
		+ vUp * m_vEcosplashOffset2.y
		+ vLook * m_vEcosplashOffset2.z;

	m_vEcosplashOffset = vFinalPos;

	m_fAirRunSpeed = m_fRunSpeed * 0.5f;
	m_pMagnetWaitTimer->SetDuration(m_fMagnetWaitTime);
	m_pDashCoolDownTimer->SetDuration(m_fDashCoolTime);

	return S_OK;
}

_bool CPlayer::MakeEcosplashGroundOffset(_float3 vBaseOffset, _float3& vOutOffset)
{
	auto spTransform = Get_MainTransform().lock();
	if (!spTransform)
		return false;

	_vector vPos = GetPos();


	_vector vLook = GetLook();
	vLook = XMVectorSetY(vLook, 0.f);

	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= 0.0001f)
		return false;

	vLook = XMVector3Normalize(vLook);

	_vector vUp = m_wpMainTransformCom.lock()->Get_LocalState(STATE::UP);


	auto vRight = m_wpMainTransformCom.lock()->Get_LocalState(STATE::RIGHT);


	_vector vWorldOffset =
		vRight * vBaseOffset.x +
		vUp * vBaseOffset.y +
		vLook * vBaseOffset.z;

	_vector vRayOrigin = vPos + vWorldOffset;

	RAY vRay;
	XMStoreFloat4(&vRay.vRayOrigin, vRayOrigin);
	vRay.vRayDir = _float4(0.f, -1.f, 0.f, 0.f);

	_float fDist = 10.f;

	_int iCollidorMask = 0;
	iCollidorMask |= (1 << ECollidorType::CT_COMMON_BODY);

	list<pair<_float, WPCollidor>> lstCollidors;

	if (!m_pGameInstance->RayCast_Collidor(iCollidorMask, vRay, fDist, lstCollidors))
		return false;

	if (fDist < -0.1f)
		return false;

	_vector vGroundPos = vRayOrigin + XMVectorSet(0.f, -fDist  +  1.f, -1.5f, 0.f);

	_float fPlayerY = XMVectorGetY(vPos);
	_float fGroundY = XMVectorGetY(vGroundPos);

	vOutOffset.x = vBaseOffset.x;
	vOutOffset.y = fGroundY - fPlayerY + 1.f;
	vOutOffset.z = vBaseOffset.z - 1.5f;

	return true;
}

void CPlayer::SetBoolElement(string _tag, _bool _value)
{
	m_ctxDesc.pBlackboard->SetBoolElement(_tag, _value);
}

void CPlayer::IsAppear(_bool bIsAppear)
{
	m_wpModel.lock()->RenderActive(bIsAppear);
	if (bIsAppear)
	{
		if (auto spModel = m_wpModel.lock()) {
			spModel->GetMeshRenderer(19)->SetActive(false);
			spModel->GetMeshRenderer(20)->SetActive(false);
			spModel->GetMeshRenderer(21)->SetActive(false);
		}
	}
}


void CPlayer::AddJustCnt()
{
	m_iCurJustCnt++;
	m_iCurJustCnt = clamp(m_iCurJustCnt, 0, m_iMaxJustCnt);
	m_pUCGamePlay->BeatUIEffect();
	m_pJustCntTimer->Restart();
	Notice_DSP_Perfect();
}

void CPlayer::EnemyRot()
{
	if (m_vecRotEnemy.empty()) return;

	_vector vPos = m_wpAttackCollider.lock()->Get_Center();

	for (auto& pEnemy : m_vecRotEnemy)
	{
		if (!pEnemy->IsActive())
			continue;
		pEnemy->SetPos(vPos);
	}

}

void CPlayer::RestartMulAtk(_float _duration)
{
	m_pMulAtk_StayTimer->SetDuration(_duration);
	m_pMulAtk_StayTimer->Restart();
	m_pMulAtkTimer->Restart();
}

void CPlayer::ElectricShock()
{
	m_bisElectricShock = true;
	m_wpModel.lock()->RenderActive(false);

	auto pModel = m_wpSkeletonModel.lock();
	pModel->RenderActive(true);
	pModel->GetMeshRenderer(0)->SetActive(false);
	pModel->TransitAnimation(0);
	pModel->AnimReset(0);
}

void CPlayer::EndElectricShock()
{
	m_bisElectricShock = false;
	m_wpModel.lock()->RenderActive(true);
	m_wpSkeletonModel.lock()->RenderActive(false);

	if (auto spModel = m_wpModel.lock()) {
		spModel->GetMeshRenderer(19)->SetActive(false);
		spModel->GetMeshRenderer(20)->SetActive(false);
		spModel->GetMeshRenderer(21)->SetActive(false);
	}
}

void CPlayer::Ignite()
{
	SetBoolElement("Enter_Ignite", true);
	m_bJumpable = false;
	m_iInputWASD = 0;
	m_pIgniteTimer->Restart();
	SetBoolElement("End_Ignite", false);
	m_pGuitar->SetActive(false);
	m_pTrashGuitar->SetActive(false);
	m_PlayerState.m_fCurHp -= 3.f;
	m_pIgniteDamageTimer->Restart();

	m_pBurn1EffectTimer->Restart();
	m_pBurn2EffectTimer->Restart();
	m_pUCEffect->ExtinguishFire();
}
void CPlayer::CheckInputIngnite()
{

	if (!m_pIgniteTimer || !m_pIgniteTimer->IsActive() || m_pIgniteTimer->GetElapsedRatio() < 0.3f) return;
	_bool InputW = m_pGameInstance->IsKeyState(DIK_W, EKEYACTIONSTATE::ENTER);
	_bool InputA = m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::ENTER);
	_bool InputS = m_pGameInstance->IsKeyState(DIK_S, EKEYACTIONSTATE::ENTER);
	_bool InputD = m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::ENTER);

	_bool InputKey = InputW || InputA || InputS || InputD;

	if (InputKey)
		++m_iInputWASD;

	if (m_iInputWASD >= 5)
	{
		m_pIgniteTimer->SetActive(false);
		EndIgnite();
	}

}
void CPlayer::EndIgnite()
{
	m_pUCEffect->ExtinguishFireEnd();
	m_pFlickerTimer->Restart();
	m_bInvincible = true;
	m_bJumpable = true;
	SetBoolElement("End_Ignite", true);
	m_pBurn1EffectTimer->SetActive(false);
	m_pBurn2EffectTimer->SetActive(false);
	m_pIgniteDamageTimer->SetActive(false);
}

void CPlayer::Flicker_RenderOn()
{
	m_pGuitar->SetActive(true);
	m_wpModel.lock()->RenderActive(true);
	if (auto spModel = m_wpModel.lock()) {
		spModel->GetMeshRenderer(19)->SetActive(false);
		spModel->GetMeshRenderer(20)->SetActive(false);
		spModel->GetMeshRenderer(21)->SetActive(false);
	}
}

void CPlayer::Flicker_RenderOff()
{
	m_pGuitar->SetActive(false);
	m_pTrashGuitar->SetActive(false);
	m_wpModel.lock()->RenderActive(false);
}

void CPlayer::Update_Flicker(_float fTimeDelta)
{
	if (!m_pFlickerTimer || !m_pFlickerTimer->IsActive())
		return;

	_float fRatio = (Engine::_float)m_pFlickerTimer->GetElapsedRatio();
	fRatio = clamp(fRatio, 0.f, 1.f);


	_float fCurve = fRatio * fRatio;

	const _float fStartInterval = 0.1f;
	const _float fEndInterval = 0.01f;

	_float fInterval = fStartInterval + (fEndInterval - fStartInterval) * fCurve;

	m_fFlickerAcc += fTimeDelta;

	if (m_fFlickerAcc >= fInterval)
	{
		m_fFlickerAcc = 0.f;
		m_bFlickerRenderOn = !m_bFlickerRenderOn;
	}

	if (m_bFlickerRenderOn)
		Flicker_RenderOn();
	else
		Flicker_RenderOff();
}

void CPlayer::ReflectLaser(FDamageInfo _damageInfo)
{

	if (_damageInfo.eAttackType != EAttackType::LAST_LASER && _damageInfo.eAttackType != EAttackType::LASER)
		return;

	_float3 _vScale = m_pEffect[VFX_LASER]->Get_MainTransform().lock()->Get_Scaled();
	if (_damageInfo.eAttackType == EAttackType::LAST_LASER)
	{
		_vector vTargetPos = m_pMimosa->GetEnemyCenter();
		_vector vDir = XMVector3Normalize(vTargetPos - Get_Center());
		_float fDist = XMVectorGetX(XMVector3Length(vTargetPos - Get_Center()));

		m_pEffect[VFX_LASER]->Set_Pos(Get_Center());
		m_pEffect[VFX_LASER]->Set_Dir(vDir);
		m_pEffect[VFX_LASER]->Get_MainTransform().lock()->Set_Scale(_vScale.x, _vScale.z, fDist);
		m_pEffect[VFX_LASER]->Restart();
	}
	else
	{
		auto spPlayerTransform = m_wpMainTransformCom.lock();
		if (!spPlayerTransform)
			return;

		auto spLaserTransform = m_pEffect[VFX_LASER]->Get_MainTransform().lock();
		if (!spLaserTransform)
			return;

		_vector vStartPos = Get_Center();

		_vector vRight = XMVector3Normalize(spPlayerTransform->Get_LocalState(STATE::RIGHT));
		_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

		const _float fSidePower = 0.25f;
		const _float fLaserDist = 10.f;

		_vector vDir12 = XMVector3Normalize(vUp);
		_vector vDir01 = XMVector3Normalize(vUp + vRight * fSidePower);
		_vector vDir11 = XMVector3Normalize(vUp - vRight * fSidePower);

		_int iRand = Random(0, 2);

		_vector vReflectDir = vDir12;

		if (iRand == 0)
			vReflectDir = vDir12;
		else if (iRand == 1)
			vReflectDir = vDir01;
		else
			vReflectDir = vDir11;

		m_pEffect[VFX_LASER]->Set_Pos(vStartPos);

		_vector vLook = XMVector3Normalize(vReflectDir);

		_vector vBaseUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

		_float fDot = fabsf(XMVectorGetX(XMVector3Dot(vLook, vBaseUp)));

		if (fDot > 0.98f)
			vBaseUp = XMVectorSet(0.f, 0.f, 1.f, 0.f);

		_vector vLaserRight = XMVector3Normalize(XMVector3Cross(vBaseUp, vLook));
		_vector vLaserUp = XMVector3Normalize(XMVector3Cross(vLook, vLaserRight));

		spLaserTransform->Set_LocalState(STATE::RIGHT, vLaserRight);
		spLaserTransform->Set_LocalState(STATE::UP, vLaserUp);
		spLaserTransform->Set_LocalState(STATE::LOOK, vLook);

		spLaserTransform->Set_Scale(_vScale.x, _vScale.z, fLaserDist);

		m_pEffect[VFX_LASER]->Restart();
	}
}

void CPlayer::CheckInteract()
{
	auto listInteract = m_pGameInstance->Get_Objects(
		m_pGameInstance->GetLevelDesc().nextLevelTag, L"Layer_Interact");

	for (auto it = listInteract.begin(); it != listInteract.end();)
	{
		auto pInteract = static_cast<CInteractObject*>(*it);
		auto pos = m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION);
		
		auto objPos = pInteract->Get_MainTransform().lock()->Get_WorldState(STATE::POSITION);

		_float fDist = XMVectorGetX(XMVector3Length(pos - objPos));

		if ((pInteract->GetInteractType() == EInteractType::IT_MIMOSA_EQUALIZER_BARRIER
			|| pInteract->GetInteractType() == EInteractType::IT_MIMOSA_EQUALIZER_SHIELD)
			|| pInteract->GetInteractType() == EInteractType::IT_QTE
			&& pInteract->IsActive())
			++it;
		else if (pInteract->GetInteractType() == EInteractType::IT_NONE
			|| pInteract->GetInteractType() == EInteractType::IT_ROBOARM
			|| pInteract->GetInteractType() == EInteractType::IT_COMMON_SWITCH
			|| pInteract->GetInteractType() == EInteractType::IT_COMBO_SWITCH
			|| pInteract->GetState() == CInteractObject::ESTATE::TRIGGER
			|| fDist > m_fDetectInteractRange)
			it = listInteract.erase(it);
		else
			++it;
	}

	auto pCamPos = m_pGameInstance->Get_CamPosition();
	auto pCamLook = m_pGameInstance->Get_CamLook();

	_vector vCamPos = XMVectorSet(pCamPos->x, pCamPos->y, pCamPos->z, 1.f);
	_vector vCamLook = XMVector3Normalize(XMVectorSet(pCamLook->x, pCamLook->y, pCamLook->z, 0.f));

	const _float fMinDot = -0.2f;
	const _float fDistWhight = 0.6f;
	const _float fDotWhight = 0.4f;
	_float fBestScore = -FLT_MAX;
	_float fBestDot = -FLT_MAX;
	m_pNearInteract = nullptr;
	m_fNearInteractDist = -FLT_MAX;

	for (auto pObj : listInteract)
	{
		auto pCol = ConvertWPComponent<CCollidor>(pObj->Get_Component<CCollidor>());
		auto pos = pCol.lock()->Get_Center();

		_vector vTargetPos = pos;
		_vector vToTarget = vTargetPos - vCamPos;

		_float fDistSq = XMVectorGetX(XMVector3LengthSq(vToTarget));
		vToTarget = XMVector3Normalize(vToTarget);

		_float fDot = XMVectorGetX(XMVector3Dot(vCamLook, vToTarget));

		_float fRadiusSq = m_fDetectInteractRange * m_fDetectInteractRange;
		_float fDistScore = 1.f - (fDistSq / fRadiusSq);

		_float fScore = fDot * fDotWhight + fDistScore * fDistWhight;
		if (fDot < fMinDot)
			continue;

		if (fScore > fBestScore)
		{
			fBestScore = fScore;
			fBestDot = fDot;
			m_pNearInteract = static_cast<CInteractObject*>(pObj);

			auto pos = m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION);
			//auto objPos = m_pNearInteract->Get_MainTransform().lock()->Get_LocalState(STATE::POSITION);
			auto objPos = m_pNearInteract->Get_MainTransform().lock()->Get_WorldState(STATE::POSITION);
			
			if (m_pNearInteract->GetInteractType() == IT_MACARON_SHIELD)
			{
				auto pCol = ConvertWPComponent<CCollidor>(pObj->Get_Component<CCollidor>());
				objPos = pCol.lock()->Get_Center();
			}

			XMStoreFloat4(&m_fInteractPos, objPos);
			m_fNearInteractDist = XMVectorGetX(XMVector3Length(pos - objPos));
		}
	}
	if (fabsf(m_fNearInteractDist) > m_fDetectInteractRange)
	{
		CUC_Interact::INTERACT_PLAYER desc;
		if (m_pUCInteract)
			m_pUCInteract->Set_InteractPlayer(desc);
	}

	if (m_pNearInteract != nullptr && m_pUCInteract)
	{
		CUC_Interact::INTERACT_PLAYER desc;
		desc.fNearDist = m_fNearInteractDist;
		desc.fInteractPos = m_fInteractPos;
		desc.pInteract = m_pNearInteract;
		desc.fCamPos = *pCamPos;
		desc.fDot = fBestDot;
		m_pUCInteract->Set_InteractPlayer(desc);
	}
}

void CPlayer::CheckQTE()
{
	if (!m_pNearInteract) return;

	_bool InputKey = m_pGameInstance->IsKeyState(DIK_G, EKEYACTIONSTATE::ENTER);

	if (InputKey && m_pUCInteract->IsQTEAble())
	{
		static_cast<CQTERhythmHero*>(m_pNearInteract)->Challenge();
	}
}

void CPlayer::Add_MaxHP(_float fHP)
{
	fHP = max(0, fHP);
	m_PlayerState.m_fMaxHp += fHP;
}

void CPlayer::Add_CurHP(_float fHP)
{
	m_PlayerState.m_fCurHp += fHP;
	m_PlayerState.m_fCurHp = min(m_PlayerState.m_fCurHp, m_PlayerState.m_fMaxHp);
	m_f3EmissiveColor = { 0.f,1.f,0.195f };
}

void CPlayer::Add_Gear(_int iGear)
{
	m_PlayerState.m_iGear += iGear;
	m_f3EmissiveColor = { 1.f,0.439f,0.127f };
}

void CPlayer::Unlock_PartnerJam(_int jamIdx)
{
	if (jamIdx > 4 || jamIdx < 0)
		return;

	m_bUnlock_PartnerJam[jamIdx] = true;
}

void CPlayer::SetStore(_bool isStore)
{
	m_bKeyLock = isStore;
	m_pPlayerCam->SetHandle(isStore);
	m_bisStore = isStore;
}

void CPlayer::CheckState()
{
	CheckInteract();
	CheckGimmic();
	CheckParry();
	CheckMagnet();
	CheckQTE();
	if (!m_bMoveKeyInput && !m_bKeyLock)
		CheckCombo();

	if (m_bisRp && m_bisDodgeRp && m_pGameInstance->IsKeyState(DIK_LSHIFT, EKEYACTIONSTATE::ENTER))
	{
		SetBoolElement("RP_Kale_Dodge", true); m_bKaleDodge = true;
	}
	else
	{
		SetBoolElement("RP_Kale_Dodge", false);
	}

	CheckPartnerJam();
}

void CPlayer::OnLanded()
{
	StopEffectTrail(false);

	if (m_bIsJump)
	{
		SetBoolElement("Land_Jump", true);
		PlayRandomSound(L"pl_ch0000_atk_rp_com_step_landing_01_Play", 1, 4);
		ResetCombo();
	}

	SetBoolElement("Enter_Jump", false);
	SetBoolElement("Enter_DoubleJump", false);
	SetBoolElement("Grounded", true);
	SetBoolElement("ArriveMagnet", false);

	m_bIsJump = false;
	m_bIsMagnet = false;
	m_bCanDoubleJump = true;
	m_bIsAirCombo = false;
	//m_bGrounded = true;
	m_fSpeedY = 0.f;
}

void CPlayer::SetPartnerLock(_bool isLock)
{
	pPartners[0]->m_bIsOnLock = isLock;
	pPartners[1]->m_bIsOnLock = isLock;
	pPartners[2]->m_bIsOnLock = isLock;
}

void CPlayer::CheckParry()
{
	_bool InputParry = m_pGameInstance->IsKeyState(DIK_E, EKEYACTIONSTATE::ENTER);

	if (!InputParry || m_bisDie || m_bKeyLock || m_bIsStoreAble)
		return;

	PrintDebug("INPUT PARRY");
	SetBoolElement("Parry", false);
	m_bKaleDodge = false;

	_int iPrevParryIdx = m_iParryIdx;

	if (!m_bGrounded)
	{
		m_iParryIdx = 3;
		m_fSpeedY = max(m_fSpeedY, 0.f);
	}
	else
	{
		do
		{
			m_iParryIdx = Random(0, 2);
		} while (m_iParryIdx == iPrevParryIdx);
	}

	m_upAnimController->SetLayerIdx("ParryIdx", m_iParryIdx);
	SetBoolElement("Parry", true);
	ResetCombo();
}

void CPlayer::CheckMagnet()
{
	_bool InputQ = m_pGameInstance->IsKeyState(DIK_Q, EKEYACTIONSTATE::ENTER);

	if (!InputQ || m_bIsMagnet || m_bKeyLock)
		return;

	m_stBestMagnetTarget = {};
	vector<MAGNET_TARGET_DESC> vecCandidates;


	auto listInteract = m_pGameInstance->Get_Objects(
		m_pGameInstance->GetLevelDesc().nextLevelTag,
		L"Layer_Interact"
	);

	for (auto pObj : listInteract)
	{
		auto pInteract = static_cast<CInteractObject*>(pObj);
		if (!pInteract)
			continue;

		if (pInteract->GetState() != CInteractObject::ESTATE::ACTIVE)
			continue;

		if (pInteract->GetInteractType() != EInteractType::IT_CHAI_MAGNET &&
			pInteract->GetInteractType() != EInteractType::IT_LAST_CHAI_MAGNET &&
			pInteract->GetInteractType() != EInteractType::IT_MAGNETRAIL)
			continue;

		MAGNET_TARGET_DESC desc;
		desc.pTarget = pInteract;
		desc.eType = pInteract->GetInteractType() == IT_MAGNETRAIL ? EMAGNET_TARGET_TYPE::MAGNETRAIL : EMAGNET_TARGET_TYPE::INTERACT;
		desc.bLast = pInteract->GetInteractType() == EInteractType::IT_LAST_CHAI_MAGNET;

		vecCandidates.push_back(desc);
	}


	for (auto pEnemy : m_listEnemy)
	{
		if (!pEnemy)
			continue;


		if (!pEnemy->IsActive())
			continue;

		MAGNET_TARGET_DESC desc;
		desc.pTarget = pEnemy;
		desc.eType = EMAGNET_TARGET_TYPE::ENEMY;
		desc.bLast = false;

		vecCandidates.push_back(desc);
	}

	if (vecCandidates.empty())
		return;

	auto spPlayerTransform = m_wpMainTransformCom.lock();
	if (!spPlayerTransform)
		return;

	auto pCamPos = m_pGameInstance->Get_CamPosition();
	auto pCamLook = m_pGameInstance->Get_CamLook();

	if (!pCamPos || !pCamLook)
		return;

	_vector vCamPos = XMVectorSet(pCamPos->x, pCamPos->y, pCamPos->z, 1.f);
	_vector vCamLook = XMVector3Normalize(XMVectorSet(pCamLook->x, pCamLook->y, pCamLook->z, 0.f));

	const _float fRadiusSq = m_fMagnetScanRadius * m_fMagnetScanRadius;

	const _float fLookWeight = 0.7f;
	const _float fDistWeight = 0.3f;
	const _float fEnemyLookBonus = 0.25f;
	const _float fMinDot = 0.15f;
	const _float fTieEpsilon = 0.0001f;


	_float fBestScore = -FLT_MAX;
	_bool bHasBest = false;


	for (auto& candidate : vecCandidates)
	{
		if (!candidate.pTarget)
			continue;

		auto spTargetTransform = candidate.pTarget->Get_MainTransform().lock();
		if (!spTargetTransform)
			continue;

		_vector vTargetPos = spTargetTransform->Get_LocalState(STATE::POSITION);
		_vector vToTarget = vTargetPos - vCamPos;

		_float fDistSq = XMVectorGetX(XMVector3LengthSq(vToTarget));
		if (fDistSq > fRadiusSq)
			continue;

		if (fDistSq <= 0.0001f)
			continue;

		vToTarget = XMVector3Normalize(vToTarget);

		_float fDot = XMVectorGetX(XMVector3Dot(vCamLook, vToTarget));

		if (fDot < fMinDot)
			continue;

		_float fDistScore = 1.f - (fDistSq / fRadiusSq);

		_float fScore = fDot * fLookWeight + fDistScore * fDistWeight;


		if (candidate.eType == EMAGNET_TARGET_TYPE::ENEMY)
			fScore += fDot * fEnemyLookBonus;

		candidate.fScore = fScore;

		_bool bBetterScore =
			fScore > fBestScore + fTieEpsilon;

		_bool bSameScorePreferInteract =
			fabsf(fScore - fBestScore) <= fTieEpsilon &&
			(candidate.eType == EMAGNET_TARGET_TYPE::INTERACT || candidate.eType == EMAGNET_TARGET_TYPE::MAGNETRAIL) &&
			m_stBestMagnetTarget.eType != EMAGNET_TARGET_TYPE::INTERACT;

		if (!bHasBest || bBetterScore || bSameScorePreferInteract)
		{
			m_stBestMagnetTarget = candidate;
			fBestScore = fScore;
			bHasBest = true;
		}
	}

	if (!bHasBest || !m_stBestMagnetTarget.pTarget)
		return;

	auto spTargetTransform = m_stBestMagnetTarget.pTarget->Get_MainTransform().lock();
	if (!spTargetTransform)
		return;

	m_vMagnetStartPos = spPlayerTransform->Get_LocalState(STATE::POSITION);

	auto targetPos = spTargetTransform->Get_LocalState(STATE::POSITION);
	auto targetPosY = XMVectorGetY(targetPos);

	m_bLastMagnet = m_stBestMagnetTarget.bLast;

	m_vMagnetTargetPos = XMVectorSetY(targetPos, targetPosY);
	if (m_stBestMagnetTarget.eType == INTERACT || m_stBestMagnetTarget.eType == MAGNETRAIL)
		SetBoolElement("Magnet", true);
	else
		SetBoolElement("EnemyMagnet", true);
	SetBoolElement("ArriveMagnet", false);
	SetBoolElement("LastMagnetEnd", m_bLastMagnet);
	SetBoolElement("Land_Jump", false);

	auto targetY = XMVectorGetY(m_vMagnetTargetPos);
	auto startY = XMVectorGetY(m_vMagnetStartPos);

	auto diffY = targetY - startY;

	//PrintDebug("diffY : ", diffY);

	if (fabsf(diffY) <= 10.f)
		m_iMagnetDirIdx = 0;
	else
		m_iMagnetDirIdx = diffY > 0.f ? 1 : 2;

	if (m_iCurState == IDLE)
		m_ibeforeLocoIdx = 0;
	else
		m_ibeforeLocoIdx = (m_fSpeedY > 0.f) ? 2 : 1;

	m_iMagnetIdx = m_iMagnetDirIdx + (m_ibeforeLocoIdx * 3);

	m_upAnimController->SetLayerIdx("MagnetIdx", m_iMagnetIdx);

	//PrintDebug("MagnetIdx : ", m_iMagnetIdx);

	m_bIsMagnet = true;
	m_pMagnetWaitTimer->Restart();
	//m_bGrounded = false;


	auto pCol = ConvertWPComponent<CCollidor>(m_stBestMagnetTarget.pTarget->Get_Component<CCollidor>());
	auto pos = pCol.lock()->Get_Center();

	m_vMagnetTargetPos = pos;

	if (m_pMagnetTrail != nullptr)
	{
		_float3 vTarget = {};
		XMStoreFloat3(&vTarget, m_vMagnetTargetPos);
		m_pMagnetTrail->SetTargetWorldPosition(vTarget);
	}

	if (m_pEffect[VFX_MAGNET_TRAIL] != nullptr)
		m_pEffect[VFX_MAGNET_TRAIL]->Restart();
}

void CPlayer::CheckCombo()
{
	_bool InputX = m_pGameInstance->IsMouseBtnState(EMOUSEBTN::LB, EKEYACTIONSTATE::ENTER);
	_bool InputY = m_pGameInstance->IsMouseBtnState(EMOUSEBTN::RB, EKEYACTIONSTATE::ENTER);
	_bool StayY = m_pGameInstance->IsMouseBtnState(EMOUSEBTN::RB, EKEYACTIONSTATE::STAY);
	_bool InputAttack = InputX || InputY || StayY;
	_bool InputLCtrl = m_pGameInstance->IsKeyState(DIK_LCONTROL, EKEYACTIONSTATE::ENTER);

	_bool bBreakDownTimeOut = false;

	if (m_bBreakDownPending)
	{
		if (m_pBreakDownDelayTimer == nullptr || !m_pBreakDownDelayTimer->IsActive())
			bBreakDownTimeOut = true;
	}

	size_t validComboCount = 0;
	for (auto eCombo : m_vecCombo)
	{
		if (eCombo == COMBO_REST)
			continue;

		++validComboCount;
	}

	if (validComboCount >= 5)
	{
		m_bBreakDownPending = false;
		ResetCombo();
		return;
	}

	if (InputLCtrl)
		m_bInputPartner = true;

	if (m_pRestTimer->IsActive() && InputAttack)
		m_bRestInput = true;

	if (m_vecCombo.size())
		m_bRest = m_bRestAble && m_vecCombo.back() == COMBO_X && (!InputX && !InputY && !StayY) && m_vecCombo.size() <= 2 && !m_pRestTimer->IsActive() && !m_bRestInput;

	if (m_pBeatHitTimer->IsActive())
	{
		auto _ratio = m_pBeatHitTimer->GetElapsedRatio();

		Notice_DSP_Perfect();

		_bool Input = InputX || InputY || m_bInputPartner;
		_bool bMiss = m_eRhythmResult == ERhythmResult::None;

		if (_ratio < 0.5f)
			return;

		if ((0.68 <= _ratio && _ratio <= 0.75) && Input)
			m_eRhythmResult = ERhythmResult::Perfect;

		else if (((0.53f <= _ratio && _ratio <= 0.67f) || (0.76f <= _ratio && _ratio <= 0.79f)) && Input)
			m_eRhythmResult = ERhythmResult::Good;
		else if (_ratio > 0.8 && bMiss)
			m_eRhythmResult = ERhythmResult::Miss;

		if (m_eRhythmResult != ERhythmResult::None && m_bFirstRhythmRes)
		{
			m_bFirstRhythmRes = false;
			auto pos = m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION);
			auto look = m_wpMainTransformCom.lock()->Get_LocalState(STATE::LOOK);
			string _str;

			if (!m_bInputPartner)
			{
				switch (m_eRhythmResult)
				{
				case Engine::ERhythmResult::Miss:
					_str = "MISS";
					break;

				case Engine::ERhythmResult::Good:
					_str = "Good";
					OnBeatHitSuccess();

					switch (m_eBeatHitType)
					{
					case Client::CPlayer::BH_XXXX:
						RestartVFX(VFX_BEATHIT_XXXX, pos, look);
						break;

					case Client::CPlayer::BH_YYY:
						RestartVFX(VFX_BEATHIT_YYY, pos, look);
						PlayRandomSound(L"pl_Ch0000_At_Inst_BeatHit_Wave_YYY_Play", 1, 6);
						m_pPlayerCam->StartCameraRoll(XMConvertToRadians(-15.f), 0.22f, 0.2f, 0.15f);
						break;

					case Client::CPlayer::BH_TUNEUP:
						RestartVFX(VFX_BEATHIT_XX_XXX, pos, look);
						PlayRandomSound(L"pl_Ch0000_At_Inst_BeatHit_Wave_XX_XXX_Play", 1, 5);
						break;

					case Client::CPlayer::BH_AIR_XXXX:
						RestartVFX(VFX_BEATHIT_AIR_XXXX, pos, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_ECOSPLASH:
						RestartVFX_PosBone(VFX_Player_BeatHit_XYXX, GetLook());
						RestartVFX(VFX_Player_BeatHit_XYXX_KRASH, m_vEcosplashOffset, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_BREAKDOWN:
						RestartVFX(VFX_Player_BeatHit_XYY, pos, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_SHRED:
						RestartVFX(VFX_Player_BeatHit_XYmY, pos, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_NONE:
						break;
					}
					break;

				case Engine::ERhythmResult::Perfect:
					_str = "Perfect";
					OnBeatHitSuccess();

					switch (m_eBeatHitType)
					{
					case Client::CPlayer::BH_XXXX:
						RestartVFX(VFX_BEATHIT_XXXX, pos, look);
						break;

					case Client::CPlayer::BH_YYY:
						RestartVFX(VFX_BEATHIT_YYY, pos, look);
						PlayRandomSound(L"pl_Ch0000_At_Inst_BeatHit_Wave_YYY_Play", 1, 6);
						m_pPlayerCam->StartCameraRoll(XMConvertToRadians(-15.f), 0.22f, 0.2f, 0.15f);
						break;

					case Client::CPlayer::BH_TUNEUP:
						RestartVFX(VFX_BEATHIT_XX_XXX, pos, look);
						PlayRandomSound(L"pl_Ch0000_At_Inst_BeatHit_Wave_XX_XXX_Play", 1, 5);
						break;

					case Client::CPlayer::BH_AIR_XXXX:
						RestartVFX(VFX_BEATHIT_AIR_XXXX, pos, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_ECOSPLASH:
						RestartVFX_PosBone(VFX_Player_BeatHit_XYXX, GetLook());
						RestartVFX(VFX_Player_BeatHit_XYXX_KRASH, m_vEcosplashOffset, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_BREAKDOWN:
						RestartVFX(VFX_Player_BeatHit_XYY, pos, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_SHRED:
						RestartVFX(VFX_Player_BeatHit_XYmY, pos, look);
						PlayRandomSound(L"Pl_Ch0000_At_Inst_BeatHit_Air_XXXX_Play", 1, 10);
						break;

					case Client::CPlayer::BH_NONE:
						break;
					}
					break;
				}


			}

			if (m_pUCBeathit != nullptr)
			{
				m_bBreakDownPending = false;
				m_pUCBeathit->Set_BHJudge(m_eRhythmResult, !m_bisRp);
				ResetCombo();
			}

		}


	}

	if (!InputX && !InputY && !m_bRest && !StayY && !bBreakDownTimeOut)
		return;

	if (m_bIsMagnet || m_bisDash || m_pBeatHitTimer->IsActive())
		return;

	m_pTarget = FindTarget();
	_bool Onbeat = m_bGoodTime || m_bPerfectTime;
	m_iInputAttack++;


	if (m_vecCombo.empty() && InputX)
	{
		m_bBreakDownPending = false;

		if (Onbeat)
		{
			m_iJustTimming++;
			PlayRandomSound(L"Pl_at_Input_BestTiming_01_Play", 2, 5);
			AddJustCnt();
			m_p808_Ball->Restart_Onbeat_VFX(m_iCurJustCnt);

		}

		m_bComboAble = false;
		m_bComboXQueued = false;
		m_iCombo_X_Idx = 0;
		m_bIsAttack = true;

		SetBoolElement("ComboX", false);
		SetBoolElement("AttackX", true);

		m_stDamageInfo.fDamage = Calc_Damage(8.5f);
		m_stDamageInfo.eAttackType = EAttackType::NORMAL;

		if (m_bGrounded)
		{
			m_vecCombo.push_back(COMBO_X);
			m_upAnimController->SetLayerIdx("AttackX_Idx", 0);
			WeaponChangeAnim(1);
		}
		else
		{
			m_vecCombo.push_back(COMBO_AIR_X);
			m_upAnimController->SetLayerIdx("AttackX_Idx", 1);
			m_fSpeedY = 0;
			m_stDamageInfo.eAttackType = EAttackType::AIRATK;
			WeaponChangeAnim(9);
		}

		m_pComboTimer->Restart();
		return;
	}

	if (m_vecCombo.empty() && InputY && !m_bMoveKeyInput)
	{
		m_bBreakDownPending = false;

		if (Onbeat)
		{
			m_iJustTimming++;
			PlayRandomSound(L"Pl_at_Input_BestTiming_01_Play", 2, 5);
			AddJustCnt();
			m_p808_Ball->Restart_Onbeat_VFX(m_iCurJustCnt);
		}

		m_bComboAble = false;
		m_bCanMove = false;
		m_bComboXQueued = false;
		m_iCombo_Y_Idx = 0;
		m_bIsAttack = true;

		SetBoolElement("InputKey", false);
		SetBoolElement("ComboY", false);
		SetBoolElement("AttackY", true);

		m_stDamageInfo.fDamage = Calc_Damage(16.f);
		m_stDamageInfo.eAttackType = EAttackType::NORMAL;

		if (m_bGrounded)
		{
			m_vecCombo.push_back(COMBO_Y);
			m_upAnimController->SetLayerIdx("AttackY_Idx", 0);
			WeaponChangeAnim(5);
		}
		else
		{
			m_bIsAirCombo = false;
			m_vecCombo.push_back(COMBO_AIR_Y);
			m_upAnimController->SetLayerIdx("AttackY_Idx", 1);
			m_stDamageInfo.eAttackType = EAttackType::AIRATK;
		}

		m_pComboTimer->Restart();
		return;
	}

	auto _ratio = m_upAnimController->GetAnimNodeRatio(m_upAnimController->GetCurrentAnimNode());

	if (!m_bComboAble && !bBreakDownTimeOut)
		return;

	if (m_bRest)
	{
		RestartVFX(VFX_REST, GetPos());
		m_bRestInput = false;
		m_pComboTimer->Restart();
		m_vecCombo.push_back(COMBO_REST);
		PlayRandomSound(L"pl_ch0000_atk_combo_x_wait_01_Play", 1, 4);
		m_bRest = false;
		DistortionComboY();
	}


	auto vecCheckCombo = m_vecCombo;

	for (auto iter = vecCheckCombo.begin(); iter != vecCheckCombo.end();)
	{
		if (*iter == COMBO_REST)
			iter = vecCheckCombo.erase(iter);
		else
			++iter;
	}

	_int nextComboIdx = -1;
	_int nextYComboIdx = -1;

	const size_t comboCount = m_vecCombo.size();
	const size_t checkComboCount = vecCheckCombo.size();


	if (bBreakDownTimeOut)
	{
		if (checkComboCount >= 3 &&
			vecCheckCombo[0] == COMBO_X &&
			vecCheckCombo[1] == COMBO_Y &&
			vecCheckCombo[2] == COMBO_Y)
		{
			nextYComboIdx = 3;
			m_eBeatHitType = BH_BREAKDOWN;
			m_bBreakDownPending = false;
		}
		else
		{
			m_bBreakDownPending = false;
			return;
		}
	}


	if (InputX && !bBreakDownTimeOut)
	{
		m_bBreakDownPending = false;

		m_stDamageInfo.fDamage = Calc_Damage(8.5f);
		m_stDamageInfo.eAttackType = EAttackType::NORMAL;

		if (checkComboCount == 2 &&
			vecCheckCombo[0] == COMBO_X &&
			vecCheckCombo[1] == COMBO_Y)
		{
			nextComboIdx = 8;
		}
		else if (checkComboCount == 3 &&
			vecCheckCombo[0] == COMBO_X &&
			vecCheckCombo[1] == COMBO_Y &&
			vecCheckCombo[2] == COMBO_X)
		{
			nextComboIdx = 9;
		}
		else if (checkComboCount == 4 &&
			vecCheckCombo[0] == COMBO_X &&
			vecCheckCombo[1] == COMBO_Y &&
			vecCheckCombo[2] == COMBO_X &&
			vecCheckCombo[3] == COMBO_X)
		{
			nextComboIdx = 10;
			m_eBeatHitType = BH_ECOSPLASH;

			m_bEffectTrailDash = false;
			StartEffectTrail();
			SetBoolElement("Enter_Jump", true);
			m_bGrounded = false;
			m_bJumpable = false;
			m_bIsJump = true;
			m_fSpeedY = m_fJumpForce;
			SetBoolElement("Grounded", false);
		}
		else
		{
			switch (comboCount)
			{
			case 1:
				if (m_vecCombo[0] == COMBO_X)
				{
					nextComboIdx = 0;
					WeaponChangeAnim(2);
				}
				else if (m_vecCombo[0] == COMBO_AIR_X)
				{
					nextComboIdx = 13;
					m_fSpeedY = 0;

					m_stDamageInfo.eAttackType = EAttackType::AIRATK;
					WeaponChangeAnim(10);
				}
				break;

			case 2:
				if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_X)
				{
					nextComboIdx = 1;
					WeaponChangeAnim(3);
				}
				else if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_REST)
				{
					nextComboIdx = 3;
					WeaponChangeAnim(8);
				}
				else if (m_vecCombo[0] == COMBO_AIR_X && m_vecCombo[1] == COMBO_AIR_X)
				{
					nextComboIdx = 14;
					m_fSpeedY = 0;

					WeaponChangeAnim(11);
					m_stDamageInfo.eAttackType = EAttackType::AIRATK;
				}
				break;

			case 3:
				if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_X && m_vecCombo[2] == COMBO_X)
				{
					nextComboIdx = 2;
					m_eBeatHitType = BH_XXXX;
					WeaponChangeAnim(4);
				}
				else if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_REST && m_vecCombo[2] == COMBO_X)
				{
					nextComboIdx = 4;
					m_bIsLinkCombo = true;
					SetBoolElement("LinkCombo", true);
					m_stDamageInfo.eAttackType = EAttackType::AIRBORNE;
				}
				else if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_X && m_vecCombo[2] == COMBO_REST)
				{
					nextComboIdx = 5;
				}
				else if (m_vecCombo[0] == COMBO_AIR_X && m_vecCombo[1] == COMBO_AIR_X && m_vecCombo[2] == COMBO_AIR_X)
				{
					nextComboIdx = 15;
					m_fSpeedY = 0;
					m_eBeatHitType = BH_AIR_XXXX;
					WeaponChangeAnim(12);
					m_stDamageInfo.eAttackType = EAttackType::AIRATK;
				}
				break;

			case 4:
				if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_X && m_vecCombo[2] == COMBO_REST && m_vecCombo[3] == COMBO_X)
				{
					nextComboIdx = 6;
				}
				break;

			case 5:
				if (m_vecCombo[0] == COMBO_X && m_vecCombo[1] == COMBO_X && m_vecCombo[2] == COMBO_REST && m_vecCombo[3] == COMBO_X && m_vecCombo[4] == COMBO_X)
				{
					nextComboIdx = 7;
					m_eBeatHitType = BH_TUNEUP;
				}
				break;

			default:
				break;
			}
		}
	}


	if (InputY && !bBreakDownTimeOut)
	{
		m_stDamageInfo.fDamage = Calc_Damage(16.f);
		m_stDamageInfo.eAttackType = EAttackType::NORMAL;


		if (m_bBreakDownPending)
		{
			if (checkComboCount >= 3 &&
				vecCheckCombo[0] == COMBO_X &&
				vecCheckCombo[1] == COMBO_Y &&
				vecCheckCombo[2] == COMBO_Y)
			{
				m_bBreakDownPending = false;

				nextYComboIdx = 13;
				m_eBeatHitType = BH_SHRED;
			}
			else
			{
				m_bBreakDownPending = false;
			}
		}
		else if (checkComboCount == 1 &&
			vecCheckCombo[0] == COMBO_X)
		{
			nextYComboIdx = 2;
		}
		else if (checkComboCount == 2 &&
			vecCheckCombo[0] == COMBO_X &&
			vecCheckCombo[1] == COMBO_Y)
		{
			m_bBreakDownPending = true;

			if (m_pBreakDownDelayTimer != nullptr)
				m_pBreakDownDelayTimer->Restart();

			m_vecCombo.push_back(COMBO_Y);
			m_pComboTimer->Restart();

			return;
		}
		else
		{
			switch (comboCount)
			{
			case 1:
				if (m_vecCombo[0] == COMBO_Y)
				{
					nextYComboIdx = 0;
					WeaponChangeAnim(6);
				}
				else if (m_vecCombo[0] == COMBO_X)
				{
					nextYComboIdx = 2;
				}
				break;

			case 2:
				if (m_vecCombo[0] == COMBO_Y && m_vecCombo[1] == COMBO_Y)
				{
					nextYComboIdx = 1;
					m_eBeatHitType = BH_YYY;
					WeaponChangeAnim(7);
				}
				break;
			}
		}
	}


	if (nextComboIdx == -1 && nextYComboIdx == -1)
		return;

	if (nextComboIdx == 0 || nextComboIdx == 1)
	{
		m_pRestTimer->Restart();
		m_bRestInput = false;
		m_bRestTimerActive = true;
		m_bRestAble = false;
	}


	m_bComboAble = false;
	m_bComboXQueued = true;
	m_iCombo_X_Idx = nextComboIdx;
	m_iCombo_Y_Idx = nextYComboIdx;

	if (Onbeat)
	{
		m_iJustTimming++;
		PlayRandomSound(L"Pl_at_Input_BestTiming_01_Play", 2, 5);
		AddJustCnt();
		m_p808_Ball->Restart_Onbeat_VFX(m_iCurJustCnt);
	}

	if (InputX)
		StartComboX();

	if (InputY || bBreakDownTimeOut)
		StartComboY();
}

void CPlayer::CheckPartnerJam()
{
	auto InputLctrl = m_pGameInstance->IsKeyState(DIK_LCONTROL, EKEYACTIONSTATE::ENTER);


	if (InputLctrl && pPartners[m_iPartnerIdx]->GetCooldownRatio() == 1.f && //&&m_PlayerState.m_iCurEnergy >= 100 &&
		(m_eRhythmResult == ERhythmResult::Good || m_eRhythmResult == ERhythmResult::Perfect && !m_bisRp && m_pTarget && !m_pTarget->IsBoss()))// && pPartners[m_iPartnerIdx]->m_bIsOnBattle == true)
	{

		if (m_bGrounded)
		{
			switch (m_iPartnerIdx)
			{
			case 0:
				if (m_bUnlock_PartnerJam[0])
				{
					Pepp_GroundJam();
					SetPartnerLock();
					m_bIsJam = true;
				}

				break;
			case 1:
				if (m_bUnlock_PartnerJam[2])
				{
					Maca_GroundJam();
					SetPartnerLock();
					m_bIsJam = true;
				}
				break;
			}
		}
		else
		{
			switch (m_iPartnerIdx)
			{
			case 0:
				if (m_bUnlock_PartnerJam[1])
				{
					Pepp_AirJam();
					SetPartnerLock();
					m_bIsJam = true;
				}

				break;
			case 2:
				if (m_bUnlock_PartnerJam[3])
				{
					Kor_AirJam();
					SetPartnerLock();
					m_bIsJam = true;
				}
				break;
			}
		}
	}
}

void CPlayer::CheckGimmic()
{
	if (m_bisGimmic)
	{
		if (m_iPartnerIdx == 1) //Macaron
		{
			auto pState = pPartners[1]->GetPGState();
			if (pState == 3 && !m_b808AnimOnce)
			{
				m_b808AnimOnce = true;
				m_p808_Cat->SetUseOffset(true);
				ChangeAnim_808Cat(2, 0.f, 1.f, false, 0, 5);
			}

			if (pState == 5 || pState == 6)
			{
				if (pState == 5)
				{
					m_upAnimController->SetLayerIdx("Success_Cheer_Idx", 0);
					SetBoolElement("Success_Cheer", true);
					ChangeAnim_808Cat(4);
				}
				else
				{
					m_ctxDesc.pBlackboard->SetElement("PartnerIdx", 1);
					SetBoolElement("Fail_Cheer", true);
					ChangeAnim_808Cat(3);
				}

			}
			else
				return;
		}
		else if (m_iPartnerIdx == 2) // Korsika
		{
			_int Res = pPartners[2]->GetRhythmResult();

			if (Res)
			{
				if (Res != -1)
					ChangeAnim_808Cat(Res + 5);
				else
				{
					ChangeAnim_808Cat(3);
				}

			}





			m_ctxDesc.pBlackboard->SetElement("RhythmResult", Res);
			if (Res == 4)
			{
				m_upAnimController->SetLayerIdx("Success_Cheer_Idx", 1);
				SetBoolElement("Success_Cheer", true);
				ChangeAnim_808Cat(13);
			}
		}
	}



	if (m_bisGimmic || !m_upTransitTimer->IsActive())
		return;

	if (m_upTransitTimer->GetElapsedRatio() == 0)
	{
		Switch808_Cat();
		m_bisGimmic = true;

		if (m_iPartnerIdx == 1)
		{
			ChangeAnim_808Cat(1, 0.f, 0.f);
		}
		else if (m_iPartnerIdx == 2)
		{
			ChangeAnim_808Cat(5, 0.f, 0.f);
		}

	}


	m_bCanMove = false;
	m_bJumpable = false;
	SetBoolElement("MVNT", true);
	if (m_iPartnerIdx)
		m_upAnimController->SetLayerIdx("Enter_CheerIdx", m_iPartnerIdx - 1);
	m_ctxDesc.pBlackboard->SetElement("PartnerIdx", m_iPartnerIdx);
}

void CPlayer::CheckGround(_float fTimeDelta)
{
	auto spTransform = m_wpMainTransformCom.lock();
	if (!spTransform)
		return;

	const _bool  bWasGrounded = m_bGrounded;
	const _float fPrevSpeedY = m_fSpeedY;

	if (m_bIsAirCombo)
	{
		m_bGrounded = false;
		return;
	}

	if (m_fSpeedY > 0.f)
	{
		m_bGrounded = false;
		return;
	}

	_vector vPos = spTransform->Get_LocalState(STATE::POSITION);


	const _float fFootOffset = 0.02f;

	const _float fRayStartUp = 0.2f;
	const _float fGroundSnapDown = 1.f;

	const _float fExtraFallDist = max(0.f, -m_fSpeedY * fTimeDelta);

	_vector vRayOrigin = vPos + XMVectorSet(0.f, fRayStartUp, 0.f, 0.f);
	_vector vRayDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);

	_float fCheckDist =
		fRayStartUp +
		fFootOffset +
		fGroundSnapDown +
		fExtraFallDist;

	RAY vRay;
	XMStoreFloat4(&vRay.vRayOrigin, vRayOrigin);
	XMStoreFloat4(&vRay.vRayDir, vRayDir);

	_int iCollidorMask = 0;
	iCollidorMask |= (1 << ECollidorType::CT_COMMON_BODY);

	list<pair<_float, WPCollidor>> lstCollidors;
	m_pGameInstance->RayCast_Collidor(iCollidorMask, vRay, fCheckDist, lstCollidors);

	lstCollidors.remove_if([=](pair<_float, WPCollidor> it) {
		if (auto pBattleField = dynamic_cast<CBattleField*>(it.second.lock()->GetGameObject())) {
			return true;
		}
		auto pObject = it.second.lock()->GetGameObject();
		if (pObject->GetName().substr(0, 15) == L"KaleMapCollider") {
			PrintDebug("YEs");
			return true;
		}
		return false;
		});

	if (lstCollidors.empty())
	{
		m_bGrounded = false;
		return;
	}

	lstCollidors.sort([](const auto& a, const auto& b)
		{
			return a.first < b.first;
		});

	const _float fHitDist = lstCollidors.front().first;

	_vector vHitPos = vRayOrigin + vRayDir * fHitDist;

	const _float fCurY = XMVectorGetY(vPos);
	const _float fHitY = XMVectorGetY(vHitPos);
	const _float fTargetY = fHitY + fFootOffset;


	const _float fGroundGap = fCurY - fTargetY;

	if (fGroundGap > fGroundSnapDown)
	{
		m_bGrounded = false;
		return;
	}

	vPos = XMVectorSetY(vPos, fTargetY);
	spTransform->Set_LocalState(STATE::POSITION, vPos);

	m_bGrounded = true;
	m_fSpeedY = 0.f;

	const _bool bShouldLand =
		!bWasGrounded ||
		m_bIsJump ||
		!m_bCanDoubleJump;

	if (bShouldLand && fPrevSpeedY <= 0.f)
		OnLanded();
}

void CPlayer::StartComboX()
{
	SetBoolElement("AttackX", false);
	SetBoolElement("ComboX", false);
	SetBoolElement("LinkCombo", true);

	m_upAnimController->SetLayerIdx("ComboX_Idx", m_iCombo_X_Idx);
	SetBoolElement("ComboX", true);

	if (m_bGrounded)
		m_vecCombo.push_back(COMBO_X);
	else
		m_vecCombo.push_back(COMBO_AIR_X);

	m_pComboTimer->Restart();


}
void CPlayer::StartComboY()
{
	SetBoolElement("AttackY", false);
	SetBoolElement("ComboY", false);

	m_upAnimController->SetLayerIdx("ComboY_Idx", m_iCombo_Y_Idx);
	SetBoolElement("ComboY", true);

	if (m_bGrounded)
		m_vecCombo.push_back(COMBO_Y);
	else
		m_vecCombo.push_back(COMBO_AIR_Y);

	m_pComboTimer->Restart();
}

_float CPlayer::Calc_Damage(_float _damage)
{
	if (m_bPerfectTime)
		_damage += (_damage * m_fPerfectDamageMul);
	else if (m_bGoodTime)
		_damage += (_damage * m_fGoodDamageMul);

	return _damage;
}

_bool CPlayer::CheckRest()
{
	for (auto combo : m_vecCombo)
	{
		if (combo == COMBO_REST)
		{
			return  true;
		}
	}
	return false;
}

void CPlayer::ActiveGuitar()
{
	m_pGuitar->SetActive(true);
	if (m_wpAttackCollider.lock())
		m_wpAttackCollider.lock()->SetActive(false);
}

void CPlayer::Fall_To_Die()
{
	m_PlayerState.m_fCurHp -= 5;
	if (m_PlayerState.m_fCurHp <= 0)
	{
		m_PlayerState.m_fCurHp = 0;
		m_bisDie = true;
		SetBoolElement("DIE", true);
	}
	else
	{
		m_pFallRespawnTimer->Restart();
		m_pUCEffect->SetFallScreen();
	}
}


void CPlayer::Move(_float fTimeDelta)
{
	if (m_bisDash || m_bisCutScene) return;
	_bool IsKeyW = m_pGameInstance->IsKeyState(DIK_W, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_W, EKEYACTIONSTATE::STAY);
	_bool IsKeyA = m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::STAY);
	_bool IsKeyS = m_pGameInstance->IsKeyState(DIK_S, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_S, EKEYACTIONSTATE::STAY);
	_bool IsKeyD = m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::STAY);

	_bool IsSpaceInput = m_pGameInstance->IsKeyState(DIK_SPACE, EKEYACTIONSTATE::ENTER);
	_bool IsLShiftInput = m_pGameInstance->IsKeyState(DIK_LSHIFT, EKEYACTIONSTATE::ENTER);
	_bool IsDebugInput = m_pGameInstance->IsKeyState(DIK_G, EKEYACTIONSTATE::ENTER);

	if (IsDebugInput)
		m_bDebugGravity = !m_bDebugGravity;

	m_bMoveKeyInput = (IsKeyW || IsKeyA || IsKeyS || IsKeyD);

	if (m_bMoveKeyInput && m_vecCombo.size())
		ResetCombo();


	auto spTransform = m_wpMainTransformCom.lock();
	if (!spTransform)
		return;
	auto pPos = spTransform->Get_LocalState(STATE::POSITION);

	SetRhythmParryPos();
	if (m_bKeyLock) return;
	if (IsSpaceInput && m_bJumpable && m_bCanMove)
	{
		m_bEffectTrailDash = false;
		StartEffectTrail();
		SetBoolElement("Enter_Jump", true);
		m_bGrounded = false;
		m_bJumpable = false;
		m_bIsJump = true;
		m_fSpeedY = m_fJumpForce;
		SetBoolElement("Grounded", false);
	}
	if (IsSpaceInput && m_iCurState == JUMP && m_bCanDoubleJump)
	{
		m_bEffectTrailDash = false;
		StartEffectTrail();
		m_ctxDesc.pBlackboard->SetBoolElement("Enter_DoubleJump", true);
		m_bCanDoubleJump = false;
		m_fSpeedY = m_fJumpForce;
	}
	if (m_bDebugGravity) //debug
	{
		auto posY = XMVectorGetY(pPos);
		if (posY <= 0.f && m_fSpeedY < 0)
		{
			m_bGrounded = true;
			posY = 0.f;
			XMVectorSetY(pPos, posY);
			spTransform->Set_LocalState(STATE::POSITION, pPos);
		}
	}

	if (!m_bCanMove || m_bisDash || !m_bDashAnimEnd || m_bIsMagnet)
		return;

	_vector vCamDir = XMLoadFloat4(&m_vCamDir);
	vCamDir = XMVectorSetY(vCamDir, 0.f);

	if (XMVectorGetX(XMVector3LengthSq(vCamDir)) < 0.000001f)
	{
		vCamDir = spTransform->Get_LocalState(STATE::LOOK);
		vCamDir = XMVectorSetY(vCamDir, 0.f);
	}

	if (XMVectorGetX(XMVector3LengthSq(vCamDir)) > 0.000001f)
		vCamDir = XMVector3Normalize(vCamDir);
	else
		vCamDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vCamRight = XMVector3Normalize(XMVector3Cross(vUp, vCamDir));

	_vector vInputDir = XMVectorZero();

	if (IsKeyW) vInputDir += vCamDir;
	if (IsKeyS) vInputDir -= vCamDir;
	if (IsKeyD) vInputDir += vCamRight;
	if (IsKeyA) vInputDir -= vCamRight;

	_bool bHasMoveInput = XMVectorGetX(XMVector3LengthSq(vInputDir)) > 0.000001f;

	if (bHasMoveInput)
		vInputDir = XMVector3Normalize(vInputDir);


	_vector vCurrentLook = spTransform->Get_LocalState(STATE::LOOK);
	vCurrentLook = XMVectorSetY(vCurrentLook, 0.f);

	if (XMVectorGetX(XMVector3LengthSq(vCurrentLook)) > 0.000001f)
		vCurrentLook = XMVector3Normalize(vCurrentLook);
	else
		vCurrentLook = vCamDir;

	_float fMoveScale = 1.f;
	_float fRotateDamp = m_fRotateDamp;

	if (bHasMoveInput)
	{
		_vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vCurrentLook));

		_float fFrontDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vInputDir));
		_float fRightDot = XMVectorGetX(XMVector3Dot(vRight, vInputDir));


		const _float fTurnThreshold = 0.80f;

		_bool bTurning = (fFrontDot < fTurnThreshold);

		if (bTurning)
		{
			if (fRightDot >= 0.f)
				m_iRunIdx = 2;
			else
				m_iRunIdx = 1;
		}
		else
		{

			m_iRunIdx = 0;
		}
		if (fFrontDot < -0.2f)
			fRotateDamp = m_fRotateDamp * 1.5f;


	}
	else
	{
		m_iRunIdx = 0;
	}
	_vector vTargetVelocity = XMVectorZero();

	_float fRunSpeed = m_bGrounded == true ? m_fRunSpeed : m_fAirRunSpeed;
	if (bHasMoveInput)
		vTargetVelocity = vInputDir * (fRunSpeed * fMoveScale);

	m_vMoveVelocity = Damp(m_vMoveVelocity, vTargetVelocity, fTimeDelta, m_fMoveDamp);

	if (XMVectorGetX(XMVector3LengthSq(m_vMoveVelocity)) < 0.000001f)
		m_vMoveVelocity = XMVectorZero();

	if (bHasMoveInput)
	{
		m_vLookDir = Damp(m_vLookDir, vInputDir, fTimeDelta, fRotateDamp);

		if (XMVectorGetX(XMVector3LengthSq(m_vLookDir)) > 0.000001f)
		{
			m_vLookDir = XMVector3Normalize(m_vLookDir);
			spTransform->Aim(m_vLookDir);
		}
	}

	_vector vPos = spTransform->Get_LocalState(STATE::POSITION);
	_vector vNewPos = vPos + m_vMoveVelocity * fTimeDelta;
	spTransform->Set_LocalState(STATE::POSITION, vNewPos);

	m_upAnimController->SetLayerIdx("RUN_Idx", m_iRunIdx);
	m_ctxDesc.pBlackboard->SetBoolElement("InputKey", m_bMoveKeyInput);
}
void CPlayer::ApplyGravity(_float fTimeDelta)
{
	auto pTransform = m_wpMainTransformCom.lock();
	if (!pTransform || m_bisDash || m_bIsMagnet || m_bIsAirCombo || m_bisParry)
		return;

	_vector vPos = pTransform->Get_LocalState(STATE::POSITION);
	_float fPosY = XMVectorGetY(vPos);

	if (!m_bGrounded)
	{
		_float fCurGravity = (m_fSpeedY > 0.f) ? m_fJumpGravity : m_fGravity * m_fGravityMul;
		_float fNextPosY = fPosY + m_fSpeedY * fTimeDelta + 0.5f * fCurGravity * fTimeDelta * fTimeDelta;

		m_fSpeedY += fCurGravity * fTimeDelta;
		m_fSpeedY = max(m_fSpeedY, m_fTerminalvelocity);

		vPos = XMVectorSetY(vPos, fNextPosY);
		pTransform->Set_LocalState(STATE::POSITION, vPos);
	}
}
void CPlayer::Dash(_float fTimeDelta)
{
	m_wpModel.lock()->SetDashAfterimage(false);
	if (!m_bCanMove || m_pDashCoolDownTimer->IsActive() || m_bIsMagnet || m_bKeyLock)
		return;

	_bool IsLShiftInput = m_pGameInstance->IsKeyState(DIK_LSHIFT, EKEYACTIONSTATE::ENTER);

	if (IsLShiftInput && !m_bisDash && !m_iDashIdx)
	{
		m_bEffectTrailDash = true;
		StartEffectTrail();
		m_bisDash = true;
		if (!m_bGrounded)
		{
			m_ctxDesc.pBlackboard->SetBoolElement("JumpDash", true);
			m_ctxDesc.pBlackboard->SetBoolElement("Enter_Jump", false);
			m_ctxDesc.pBlackboard->SetBoolElement("Enter_DoubleJump", false);
			m_fSpeedY = 0;
		}
		else
		{
			m_ctxDesc.pBlackboard->SetBoolElement("Dash", true);
			m_iDashIdx = 0;
			m_upAnimController->SetLayerIdx("DashIdx", m_iDashIdx);
			if (m_bRhythmDash)
			{
				AddJustCnt();
				m_p808_Ball->Restart_Onbeat_VFX(m_iCurJustCnt);
				m_pGameInstance->Play_SFX(L"pl_Ch0000_Act_Inst_Evade_01_Play", 1.f);
			}

		}
	}

	if (!m_bisDash)
		return;

	m_wpModel.lock()->SetDashAfterimage(true);

	auto spTransform = m_wpMainTransformCom.lock();
	if (!spTransform)
		return;

	if (m_fElapsedDash <= 0.f)
	{
		_bool IsKeyW = m_pGameInstance->IsKeyState(DIK_W, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_W, EKEYACTIONSTATE::STAY);
		_bool IsKeyA = m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::STAY);
		_bool IsKeyS = m_pGameInstance->IsKeyState(DIK_S, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_S, EKEYACTIONSTATE::STAY);
		_bool IsKeyD = m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::ENTER) || m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::STAY);


		_vector vCamDir = XMLoadFloat4(&m_vCamDir);
		vCamDir = XMVectorSetY(vCamDir, 0.f);

		if (XMVectorGetX(XMVector3LengthSq(vCamDir)) < 0.000001f)
		{
			vCamDir = spTransform->Get_LocalState(STATE::LOOK);
			vCamDir = XMVectorSetY(vCamDir, 0.f);
		}

		if (XMVectorGetX(XMVector3LengthSq(vCamDir)) > 0.000001f)
			vCamDir = XMVector3Normalize(vCamDir);
		else
			vCamDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);

		_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		_vector vCamRight = XMVector3Normalize(XMVector3Cross(vUp, vCamDir));

		_vector vInputDir = XMVectorZero();

		if (IsKeyW) vInputDir += vCamDir;
		if (IsKeyS) vInputDir -= vCamDir;
		if (IsKeyD) vInputDir += vCamRight;
		if (IsKeyA) vInputDir -= vCamRight;

		if (XMVectorGetX(XMVector3LengthSq(vInputDir)) > 0.000001f)
		{
			m_vDashDir = XMVector3Normalize(vInputDir);
		}
		else
		{
			_vector vLook = m_vLookDir;
			vLook = XMVectorSetY(vLook, 0.f);

			if (XMVectorGetX(XMVector3LengthSq(vLook)) > 0.000001f)
				m_vDashDir = XMVector3Normalize(vLook);
			else
				m_vDashDir = vCamDir;
		}

		m_vMoveVelocity = XMVectorZero();
	}

	m_fElapsedDash += fTimeDelta;

	_float t = m_fElapsedDash / m_fDashDuration;
	t = min(max(t, 0.f), 1.f);

	//m_pGameInstance->SetColorReverse(sinf(t* DirectX::XM_PI));

	_float fDashWeight = 1.f - easeOutQuart(t);
	_float fCurDashSpeed = m_fDashSpeed * fDashWeight;

	_vector vDashVelocity = m_vDashDir * fCurDashSpeed;


	if (XMVectorGetX(XMVector3LengthSq(m_vDashDir)) > 0.000001f)
	{
		m_vLookDir = Damp(m_vLookDir, m_vDashDir, fTimeDelta, m_fRotateDamp * 1.5f);

		if (XMVectorGetX(XMVector3LengthSq(m_vLookDir)) > 0.000001f)
		{
			m_vLookDir = XMVector3Normalize(m_vLookDir);
			spTransform->Aim(m_vLookDir);
		}
	}

	_vector vPos = spTransform->Get_LocalState(STATE::POSITION);
	_vector vNewPos = vPos + vDashVelocity * fTimeDelta;
	spTransform->Set_LocalState(STATE::POSITION, vNewPos);

	if (IsLShiftInput && m_bRhythmDash && m_bGrounded)
	{
		m_bNextDashQueued = true;
	}
	if (!m_bRhythmDash)
		m_bNextDashQueued = false;


	if (t >= 1.f)
	{
		if (m_bNextDashQueued)
		{
			m_bNextDashQueued = false;
			m_bChangeDash = false;
			if (m_iDashIdx < 2)
				m_iDashIdx++;
			else
			{
				Reset_Dash();
				return;
			}

			m_fElapsedDash = 0.f;
			m_pGameInstance->Play_SFX(L"pl_Ch0000_Act_Inst_Evade_01_Play", 1.f);
			m_upAnimController->SetLayerIdx("DashIdx", m_iDashIdx);
			m_ctxDesc.pBlackboard->SetBoolElement("Dash", false);
			m_ctxDesc.pBlackboard->SetBoolElement("rhythmDash", true);
			m_bEffectTrailDash = true;
			AddJustCnt();
			m_p808_Ball->Restart_Onbeat_VFX(m_iCurJustCnt);
			StartEffectTrail();
			if (m_iDashIdx == 1)
				RestartVFX(VFX_RHYTHMDASHSWING, GetPos());
			if (m_iDashIdx == 2)
				RestartVFX(VFX_RHYTHMDASHIMPACT, GetPos());
			return;
		}
		Reset_Dash();
	}

}
void CPlayer::StayMagnet(_float fTimeDelta)
{
	if (!m_bIsMagnet || XMVector3Equal(m_vMagnetTargetPos, XMVectorZero()))
	{
		if (m_pEffect[VFX_MAGNET_TRAIL] != nullptr)
			m_pEffect[VFX_MAGNET_TRAIL]->Stop();
		return;
	}

	auto pTransform = m_wpMainTransformCom.lock();
	if (!pTransform)
		return;

	if (!UpdateMagnetTargetPos())
	{
		SetBoolElement("Magnet", false);
		SetBoolElement("EnemyMagnet", false);
		SetBoolElement("ArriveMagnet", false);

		if (m_pEffect[VFX_MAGNET_TRAIL] != nullptr)
			m_pEffect[VFX_MAGNET_TRAIL]->Stop();

		m_bIsMagnet = false;
		m_vMagnetTargetPos = XMVectorZero();
		m_vLastMagnetVel = XMVectorZero();
		m_fMagnetElasedTime = 0.f;
		m_stBestMagnetTarget = {};
		return;
	}

	if (m_pMagnetWaitTimer->IsActive())
	{
		if (m_pMagnetWaitTimer->GetElapsedRatio() == 0)
		{
			RestartVFX(VFX_MAGNET, GetPos());
			if (m_stBestMagnetTarget.eType == INTERACT || m_stBestMagnetTarget.eType == MAGNETRAIL)
				static_cast<CInteractObject*>(m_stBestMagnetTarget.pTarget)->Trigger();
			m_bInvincible = true;
		}


		pTransform->LookAtDamp(m_vMagnetTargetPos, fTimeDelta, m_fMoveDamp * 2, true);
		return;
	}

	_float xPrevRaw = (m_fMagnetDuration > 0.f) ? (m_fMagnetElasedTime / m_fMagnetDuration) : 1.f;
	_float xPrev = clamp(xPrevRaw, 0.f, 1.f);

	if (!m_fMagnetElasedTime)
	{
		m_pPlayerCam->StartCamShake(0.3f, 0.2f, 42.f);
	}
	m_fMagnetElasedTime += fTimeDelta;

	_float tRaw = (m_fMagnetDuration > 0.f) ? (m_fMagnetElasedTime / m_fMagnetDuration) : 1.f;
	_float t = clamp(tRaw, 0.f, 1.f);

	if (m_fMagnetDuration > 0.f && xPrev < 0.999f)
	{
		_float ds = 6.f * xPrev * (1.f - xPrev);
		_vector delta = m_vMagnetTargetPos - m_vMagnetStartPos;
		m_vLastMagnetVel = delta * (ds / m_fMagnetDuration);
	}

	_float s = SmoothStep(t);

	_vector pos = XMVectorLerp(m_vMagnetStartPos, m_vMagnetTargetPos, s);
	pTransform->Set_LocalState(STATE::POSITION, pos);

	auto dist = XMVectorGetX(XMVector3Length(pos - m_vMagnetTargetPos));

	if (fabsf(dist) <= 1.f || t >= 1.f)
	{
		SetBoolElement("Magnet", false);
		SetBoolElement("EnemyMagnet", false);
		SetBoolElement("ArriveMagnet", true);
		m_pPlayerCam->StartCamShake(0.05f, 0.12f, 32.f);
		m_bInvincible = false;
		if (m_stBestMagnetTarget.eType == EMAGNET_TARGET_TYPE::INTERACT)
		{
			m_fSpeedY = m_fJumpForce;
			m_bIsJump = true;
			m_bCanDoubleJump = true;
			m_bGrounded = false;
			static_cast<CInteractObject*>(m_stBestMagnetTarget.pTarget)->Activate();
		}
		else if (m_stBestMagnetTarget.eType == EMAGNET_TARGET_TYPE::MAGNETRAIL)
		{
			SetBoolElement("ArriveMagnet", false);
			StartMagnetRail();

			m_pRail = static_cast<CMagnetRail*>(m_stBestMagnetTarget.pTarget);
			m_pRail->SetPlayerAttach(true);
		}
		else
		{
			auto pEnemy = static_cast<CEnemy*>(m_stBestMagnetTarget.pTarget);
			if (pEnemy->GetIsFly())
			{
				m_fSpeedY = m_fJumpForce * 0.6f;
				SetBoolElement("Enter_Jump", true);
				m_bGrounded = false;
				m_bJumpable = false;
				m_bIsJump = true;
				SetBoolElement("Grounded", false);
				SetBoolElement("Land_Jump", false);
			}
			ResetCombo();
			// Enemy Arrived
		}

		if (m_pEffect[VFX_MAGNET_TRAIL] != nullptr)
			m_pEffect[VFX_MAGNET_TRAIL]->Stop();

		m_bIsMagnet = false;
		m_vMagnetTargetPos = XMVectorZero();
		m_vLastMagnetVel = XMVectorZero();
		m_fMagnetElasedTime = 0.f;
		m_stBestMagnetTarget = {};
	}
}

_bool CPlayer::UpdateMagnetTargetPos()
{
	if (!m_stBestMagnetTarget.pTarget)
		return false;

	if (m_stBestMagnetTarget.eType != EMAGNET_TARGET_TYPE::ENEMY)
		return true;

	if (!m_stBestMagnetTarget.pTarget->IsActive())
		return false;

	auto spTargetTransform = m_stBestMagnetTarget.pTarget->Get_MainTransform().lock();
	if (!spTargetTransform)
		return false;

	_vector vTargetPos = spTargetTransform->Get_LocalState(STATE::POSITION);

	auto wpCol = ConvertWPComponent<CCollidor>(
		m_stBestMagnetTarget.pTarget->Get_Component<CCollidor>()
	);

	if (auto spCol = wpCol.lock())
		vTargetPos = spCol->Get_Center();


	vTargetPos = XMVectorSetW(vTargetPos, 1.f);
	m_vMagnetTargetPos = vTargetPos;

	if (m_pMagnetTrail != nullptr)
	{
		_float3 vTarget = {};
		XMStoreFloat3(&vTarget, m_vMagnetTargetPos);
		m_pMagnetTrail->SetTargetWorldPosition(vTarget);
	}

	return true;
}

void CPlayer::Reset_Dash()
{
	m_bisDash = false;
	m_bNextDashQueued = false;
	m_fElapsedDash = 0.f;
	m_vMoveVelocity = XMVectorZero();
	m_ctxDesc.pBlackboard->SetBoolElement("Dash", false);
	m_iDashIdx = 0;
	m_ctxDesc.pBlackboard->SetBoolElement("rhythmDash", false);
	m_pDashCoolDownTimer->Restart();
	m_upAnimController->SetLayerIdx("DashIdx", m_iDashIdx);
	StopEffectTrail(false);
}
void CPlayer::ResetCombo()
{
	if (m_pComboTimer->IsActive())
		m_pComboTimer->Reset();
	m_vecCombo.clear();

	SetBoolElement("AttackX", false);
	SetBoolElement("AttackY", false);
	SetBoolElement("ComboX", false);
	SetBoolElement("ComboY", false);

	m_bComboAble = false;
	m_bComboXQueued = false;
	m_iCombo_X_Idx = 0;
	m_iCombo_Y_Idx = 0;
	m_bRest = false;
	m_bIsAttack = false;
	//m_bIsAirCombo = false;
	m_bIsLinkCombo = false;
	m_bRestInput = false;
	m_bRestAble = false;

	//m_bCanMove              = true;
}
void CPlayer::PushEnemy(CEnemy* pEnemy)
{
	m_listEnemy.push_back(pEnemy);
}
void CPlayer::ClearEnemy()
{
	m_listEnemy.clear();
}
CEnemy* CPlayer::FindTarget()
{
	CEnemy* PEnemy = nullptr;
	list<CEnemy*> pList = m_listEnemy;

	if (pList.empty())
		return nullptr;

	auto spTransform = m_wpMainTransformCom.lock();
	auto pos = spTransform->Get_LocalState(STATE::POSITION);
	auto Look = spTransform->Get_LocalState(STATE::LOOK);


	_float fRadiusSq = m_fAttackSnapRange * m_fAttackSnapRange;
	_float fBestScore = -FLT_MAX;

	for (auto it = pList.begin(); it != pList.end();)
	{
		auto _mgPos = (*it)->Get_MainTransform().lock()->Get_LocalState(STATE::POSITION);
		auto fDist = XMVectorGetX(XMVector3Length(_mgPos - pos));

		if (fDist > m_fAttackSnapRange)
			it = pList.erase(it);
		else
			++it;
	}
	if (pList.empty())
		return nullptr;


	for (auto it = pList.begin(); it != pList.end(); )
	{
		if (!(*it))
		{
			it = pList.erase(it);
			continue;
		}

		auto enemyTransform = (*it)->Get_MainTransform().lock();
		if (!enemyTransform)
		{
			it = pList.erase(it);
			continue;
		}

		_vector vEnemyPos = enemyTransform->Get_LocalState(STATE::POSITION);
		_vector vToEnemy = vEnemyPos - pos;

		_float fDistSq = XMVectorGetX(XMVector3LengthSq(vToEnemy));
		if (fDistSq > fRadiusSq)
		{
			it = pList.erase(it);
			continue;
		}

		vToEnemy = XMVector3Normalize(vToEnemy);
		_float fDot = XMVectorGetX(XMVector3Dot(Look, vToEnemy));

		_float fDistScore = 1.f - (fDistSq / fRadiusSq);

		_float fScore = fDot;

		if (fScore > fBestScore)
		{
			fBestScore = fScore;
			PEnemy = *it;
		}

		++it;
	}

	return PEnemy;
}
void CPlayer::AttackSnap(_float fTimeDelta)
{
	if (m_pTarget == nullptr)
		return;

	auto targetPos = m_pTarget->GetEnemyCenter();
	auto fDist = XMVectorGetX(XMVector3Length(targetPos - m_wpMainTransformCom.lock()->Get_LocalState(STATE::POSITION)));

	if (fDist > m_fAttackSnapRange)
	{
		m_pTarget = nullptr;
		return;
	}

	m_wpMainTransformCom.lock()->LookAtDamp(targetPos, fTimeDelta, 40.f, true);
	m_wpModel.lock()->SetCurrentDir();
}
void CPlayer::DistortionComboY()
{
	DISTORTION_DESC Desc = {};
	Desc.fDuration = { 0.1f };
	Desc.fRange = { 4.f };
	Desc.fStrength = { 0.2f };
	Desc.vPosition = { Get_MainTransform().lock()->Get_LocalState(STATE::POSITION) };
	m_pGameInstance->SetDistortion(Desc);
}
void CPlayer::PlaySoundXXXX()
{
	if (m_wpBeatHitCollider[BH_XXXX].lock()->IsActive() && !m_bPrevXXXX_Active)
	{
		m_bPrevXXXX_Active = true;
		PlayRandomSound(L"pl_Ch0000_At_Inst_BeatHit_Wave_XXX_Play", 1, 8);

	}
	else if (!m_wpBeatHitCollider[BH_XXXX].lock()->IsActive())
	{
		m_bPrevXXXX_Active = false;
	}

	if (m_wpBeatHitCollider[BH_XXXX_2].lock()->IsActive() && !m_bPrevXXXX2_Active)
	{
		m_bPrevXXXX2_Active = true;
		PlayRandomSound(L"pl_Ch0000_At_Inst_BeatHit_Wave_XXX_Play", 1, 8);
	}
	else if (!m_wpBeatHitCollider[BH_XXXX_2].lock()->IsActive())
	{
		m_bPrevXXXX2_Active = false;
	}


}
_float4 CPlayer::GetCameraSocketPos()
{
	_float4 _vHeadPos = { 0.f, 0.f, 0.f, 1.f };
	if (m_pCameraBone) {
		auto _matBone = m_pCameraBone->GetDesc().m_CombinedTransformationMatrix;
		memcpy(&_vHeadPos, _matBone.m[3], sizeof(_vHeadPos));
	}
	return _vHeadPos;
}

void CPlayer::Switch808_Ball()
{
	if (!m_p808_Cat || !m_p808_Ball) return;

	m_p808_Cat->SetUseOffset(false);
	m_p808_Cat->SetActive(false);
	auto _pos = m_p808_Cat->Get_MainTransform().lock()->Get_LocalState(STATE::POSITION);
	m_p808_Ball->SetPos(_pos);
	m_p808_Ball->RequestSnapFollowPos();
	m_p808_Ball->SetActive(true);

}

void CPlayer::Switch808_Cat()
{
	if (!m_p808_Cat || !m_p808_Ball) return;


	m_p808_Cat->SetActive(true);
	auto _pos = m_p808_Ball->Get_MainTransform().lock()->Get_LocalState(STATE::POSITION);
	m_p808_Cat->SetPos(_pos);
	m_p808_Ball->SetActive(false);
}

void CPlayer::ChangeAnim_808Cat(int NodeIdx, float fBlendTime, float fExitRatio, bool bSync, int BlendType, int iBeatTransitionmask)
{
	if (m_p808_Cat)
		m_p808_Cat->Change_Animation(NodeIdx, fBlendTime, fExitRatio, bSync, BlendType, iBeatTransitionmask);
}

void CPlayer::InCutScene()
{
	IsAppear(false);
	m_bSkipUpdate = true;
	m_pGuitar->SetActive(false);
	m_pTrashGuitar->SetActive(false);
	m_p808_Ball->SetActive(false);
	//m_pUCGamePlay->CutSceneInOut(true);
	Stop_VFX();
	PrintDebug("ENTER CUTSCENE");

	pPartners[0]->m_bIsOnLock = true;
	pPartners[1]->m_bIsOnLock = true;
	pPartners[2]->m_bIsOnLock = true;
}

void CPlayer::OutCutScene()
{
	IsAppear(true);
	m_bSkipUpdate = false;
	ActiveGuitar();
	m_p808_Ball->SetActive(true);
	//m_pUCGamePlay->CutSceneInOut(false);
	PrintDebug("OUT CUTSCENE");

	pPartners[0]->m_bIsOnLock = false;
	pPartners[1]->m_bIsOnLock = false;
	pPartners[2]->m_bIsOnLock = false;
}

void CPlayer::GuitarTrickCutScene()
{
	SetBoolElement("GuitarTrick", true);
	if (!XMVector3Equal(m_vRespawnPos, XMVectorZero()))
		SetPos(m_vRespawnPos);
	m_fElasedBattleTime = 0.f;
	RestartVFX(VFX_BATTLE_START, GetPos(), GetLook());
}
void CPlayer::CutSceneReset()
{
	SetBoolElement("GuitarTrick", false);
}

void CPlayer::CombatStartUI(_bool isActive)
{
	if (m_pUCEffect != nullptr && isActive)m_pUCEffect->CombatStartEffect();
	if (m_pUCGamePlay != nullptr)m_pUCGamePlay->SetScoreCalc(true);
	PrintDebug("COMBAT UI!!!");
}


void CPlayer::Reset_DanceInput()
{
	SetBoolElement("MIMOSADANCE_X", false);
	SetBoolElement("MIMOSADANCE_A", false);
	SetBoolElement("MIMOSADANCE_LB", false);
	SetBoolElement("MIMOSADANCE_RB", false);
	SetBoolElement("MIMOSADANCE_MISS", false);
}

void CPlayer::Enter_MimosaDance()
{
	m_pPlayerCam->SetHandle(true);
	SetBoolElement("MIMOSADANCE_FAIL", false);
	SetBoolElement("MIMOSADANCE_SUCCESS", false);
	auto _pBone = m_wpModel.lock()->FindBoneWithName(L"l_attach_hand_00");
	m_pGuitar->Set_Bone(_pBone);

	m_bMimosaDance = true;
	m_bSkipUpdate = true;
	SetBoolElement("MIMOSA_DANCE", true);
	m_pUCGamePlay->UCRhythmNoteActive(true, 1);

	pPartners[0]->m_bIsOnLock = true;
	pPartners[1]->m_bIsOnLock = true;
	pPartners[2]->m_bIsOnLock = true;

}

void CPlayer::Check_MimosaDance()
{
	if (!m_bMimosaDance)
		return;

	_int CurPhase = m_pUCRhythmNote->Get_CurPhase();
	_int CurTurn = m_pUCRhythmNote->Get_NoteState();    // 0 - Chai, 1 - Mimosa, 2 - Finish



	if (CurTurn == 0 || CurTurn == 1)
	{
		if (m_iPrevMimosaCamPhase != CurPhase ||
			m_iPrevMimosaCamTurn != CurTurn)
		{
			if (m_iPrevMimosaCamPhase != CurPhase)
				m_iMimosaInputNum = 0;

			m_iPrevMimosaCamPhase = CurPhase;
			m_iPrevMimosaCamTurn = CurTurn;

			_int iTurnOffset = (CurTurn == 1) ? 0 : 1;
			_int iCamPhase = (CurPhase - 1) * 2 + iTurnOffset;

			if (CurTurn == 0)
			{
				MimosaCamPlayer(iCamPhase);
			}
			else if (CurTurn == 1)
			{
				MimosaCamMimosa(iCamPhase);
				m_pMimosa->Transit_RB_Phase(CurPhase);
			}
		}
	}
	if (CurTurn == 3)
	{
		End_MimosaDance();
		m_pMimosa->Set_RB_End();
	}


	Reset_DanceInput();

	_bool InputMouse = m_pGameInstance->IsMouseBtnState(EMOUSEBTN::LB, EKEYACTIONSTATE::ENTER); // X
	_bool InputSpace = m_pGameInstance->IsKeyState(DIK_SPACE, EKEYACTIONSTATE::STAY);           // A
	_bool InputQ = m_pGameInstance->IsKeyState(DIK_Q, EKEYACTIONSTATE::STAY);               // LB
	_bool InputLShift = m_pGameInstance->IsKeyState(DIK_LSHIFT, EKEYACTIONSTATE::STAY);          // RB

	_bool CheckAble = m_pUCRhythmNote->CheckInput();
	_bool InputKey = InputMouse || InputSpace || InputQ || InputLShift;

	auto PlaySuccessSfx = [&]()
		{
			++m_iMimosaInputNum;

			wchar_t szSfxTag[128] = {};

			if (CurPhase != 6)
			{
				swprintf_s(
					szSfxTag,
					128,
					L"HBKm_ST08_Boss_Mimosa_Bridge_%d_%02d_Play",
					CurPhase,
					m_iMimosaInputNum
				);
			}
			else if (CurPhase == 6)
			{
				swprintf_s(
					szSfxTag,
					128,
					L"HBKm_ST08_Boss_Mimosa_Bridge_%d_%02d_Play",
					8,
					m_iMimosaInputNum
				);
			}
			
			m_pGameInstance->Stop_Group(ESOUNDTYPE::SFX);
			m_pGameInstance->Play_SFX(szSfxTag, 1.f);

			m_pGameInstance->Play_SFX(L"enm_com_atk_charge_single_01_Play", 1.f);
		};


	if (!CheckAble)
	{
		if (m_pUCRhythmNote->CheckMiss())
		{
			m_pUCRhythmNote->FailHit();
			SetBoolElement("MIMOSADANCE_MISS", true);
			m_pGameInstance->Play_SFX(L"sp_mng_st08_mimosa_bridge_fail_01_Play_01", 1.f);
		}
		return;
	}

	if (InputMouse)
	{
		if (m_pUCRhythmNote->CheckHit(CUC_RhythmNote::INPUTTYPE::MOUSE, 1))
		{
			SetBoolElement("MIMOSADANCE_X", true);
			PlaySuccessSfx();
		}

		else
		{
			m_pUCRhythmNote->FailHit();
			SetBoolElement("MIMOSADANCE_MISS", true);
			m_pGameInstance->Play_SFX(L"sp_mng_st08_mimosa_bridge_fail_01_Play_01", 1.f);
		}
	}
	else if (InputSpace)
	{
		if (m_pUCRhythmNote->CheckHit(CUC_RhythmNote::INPUTTYPE::KEYBOARD, DIK_SPACE))
		{
			PlaySuccessSfx();
			SetBoolElement("MIMOSADANCE_A", true);
		}
		else
		{
			m_pUCRhythmNote->FailHit();
			SetBoolElement("MIMOSADANCE_MISS", true);
			m_pGameInstance->Play_SFX(L"sp_mng_st08_mimosa_bridge_fail_01_Play_01", 1.f);
		}
	}
	else if (InputQ)
	{
		if (m_pUCRhythmNote->CheckHit(CUC_RhythmNote::INPUTTYPE::KEYBOARD, DIK_Q))
		{
			SetBoolElement("MIMOSADANCE_LB", true);
			PlaySuccessSfx();
		}
		else
		{
			m_pUCRhythmNote->FailHit();
			SetBoolElement("MIMOSADANCE_MISS", true);
			m_pGameInstance->Play_SFX(L"sp_mng_st08_mimosa_bridge_fail_01_Play_01", 1.f);
		}
	}
	else if (InputLShift)
	{
		if (m_pUCRhythmNote->CheckHit(CUC_RhythmNote::INPUTTYPE::KEYBOARD, DIK_LSHIFT))
		{
			SetBoolElement("MIMOSADANCE_RB", true);
			PlaySuccessSfx();
		}
		else
		{
			m_pUCRhythmNote->FailHit();
			SetBoolElement("MIMOSADANCE_MISS", true);
			m_pGameInstance->Play_SFX(L"sp_mng_st08_mimosa_bridge_fail_01_Play_01", 1.f);
		}
	}
}

void CPlayer::End_MimosaDance()
{
	m_pUCGamePlay->UCRhythmNoteActive(false, 1);
	m_pPlayerCam->SetHandle(false);
	m_bMimosaDance = false;
	m_bSkipUpdate = false;
	auto _pBone = m_wpModel.lock()->FindBoneWithName(L"Guitar_attach_Socket");
	m_pGuitar->Set_Bone(_pBone);
	SetBoolElement("MIMOSADANCE_SUCCESS", true);

	pPartners[0]->m_bIsOnLock = false;
	pPartners[1]->m_bIsOnLock = false;
	pPartners[2]->m_bIsOnLock = false;
}

void CPlayer::Fail_MimosaDance()
{
	m_bisDie = true;
	SetBoolElement("MIMOSADANCE_FAIL", true);
}

void CPlayer::Mimosa_Grab(CBone* pBone)
{
	if (m_bisParry && pBone)
	{
		m_pMimosa->IsParrying(m_eRhythmResult);
		return;
	}

	m_pMimosaGrabBone = pBone;
	if (pBone)
		SetBoolElement("MimosaGrab", true);
}
void CPlayer::Mimosa_Grab_CamBone(CBone* pCamBone)
{
	m_pPlayerCam->SetTargetBone(pCamBone, 0.4f);
}

void CPlayer::MimosaCamPlayer(int iSequence)
{
	PrintDebug("PLAYERCAM!!!");
	m_pMimosaDanceCamHandler->SetTarget(this, 0);
	m_pMimosaDanceCamHandler->PlayCameraSequence(iSequence, false);
}

void CPlayer::MimosaCamMimosa(int iSequence)
{
	if (!m_pMimosa) return;
	PrintDebug("MIMOSACAM!!!");
	m_pMimosaDanceCamHandler->SetTarget(m_pMimosa, 1);
	m_pMimosaDanceCamHandler->PlayCameraSequence(iSequence, false);
}

void CPlayer::SetMimosa(CEnemy_Mimosa* pMimosa)
{
	m_pMimosa = pMimosa;
	m_listEnemy.push_back(static_cast<CEnemy*>(pMimosa));
}

void CPlayer::SetKale(CEnemy_Kale* pKale)
{
	m_pKale = pKale;
	m_listEnemy.push_back(static_cast<CEnemy*>(pKale));
}

void CPlayer::SetBrotherKale(CEnemy_Kale_Phase4* pKale)
{
	m_pBrotherKale = pKale;
	m_listEnemy.push_back(static_cast<CEnemy*>(pKale));
}

void CPlayer::Pepp_GroundJam()
{
	m_fMulAtk_StayTime = 1.2f;
	m_bInvincible = true;
	m_bKeyLock = true;
	m_pWeaponPartner[0]->SetActive(true);
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"origin");
	SetBoolElement("Enter_PeppGroundJam", true);
	//pPartners[0]->SetGroundJam();
	m_pWeaponPartner[0]->Set_Bone(pBone);
	m_pWeaponPartner[0]->Change_Animation(L"SK_ch1000|CP_Peppermint_atk_AirJam_SwitchKicker");

	m_pWeaponPartnerWeaponKorsica_L->SetActive(false);
	m_pWeaponPartnerWeaponKorsica_R->SetActive(false);
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->SetActive(false);

	auto propBone = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	if (m_pTarget) {
		m_pTarget->SetGamBone(propBone, EGamComboType::PEPPERMINT_GROUND);
	}
}

void CPlayer::Pepp_AirJam()
{
	m_fMulAtk_StayTime = 1.6f;
	m_bInvincible = true;
	m_bKeyLock = true;
	m_pWeaponPartner[0]->SetActive(true);
	m_bIsAirCombo = true;
	SetBoolElement("Enter_PeppAirJam", true);
	//pPartners[0]->SetAirJam();
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"origin");
	m_pWeaponPartner[0]->Set_Bone(pBone);
	m_pWeaponPartner[0]->Change_Animation(L"SK_ch1000|CP_Peppermint_atk_GroundJam_MasterBlaster");

	m_pWeaponPartnerWeaponKorsica_L->SetActive(false);
	m_pWeaponPartnerWeaponKorsica_R->SetActive(false);
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->SetActive(true);
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->Change_Animation(L"SK_wp1030|wp1030_atk-jam-cp_010");

	auto propBone = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	if (m_pTarget)
		m_pTarget->SetGamBone(propBone, EGamComboType::PEPPERMINT_AIR);
	TrashGuitarChangeAnim(L"wp0014_atk-jam-cp_010");
}

void CPlayer::Kor_AirJam()
{
	m_fMulAtk_StayTime = 1.6f;
	m_bInvincible = true;
	m_bKeyLock = true;
	m_pWeaponPartner[2]->SetActive(true);
	m_bIsAirCombo = true;
	SetBoolElement("Enter_KorAirJam", true);
	//pPartners[2]->SetAirJam();
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"origin");
	m_pWeaponPartner[2]->Set_Bone(pBone);
	m_pWeaponPartner[2]->Change_Animation(L"SK_ch4000|CP_Korsica_atk_AirJam_TornadoLift");

	m_pWeaponPartnerWeaponKorsica_L->SetActive(true);
	m_pWeaponPartnerWeaponKorsica_R->SetActive(true);
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->SetActive(false);

	auto propBone = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	if (m_pTarget)
		m_pTarget->SetGamBone(propBone, EGamComboType::KORSICA_AIR);
}

void CPlayer::Maca_GroundJam()
{
	m_fMulAtk_StayTime = 1.2f;
	m_bInvincible = true;
	m_bKeyLock = true;
	m_pWeaponPartner[1]->SetActive(true);
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"origin");
	m_pWeaponPartner[1]->Set_Bone(pBone);
	SetBoolElement("Enter_MacaGroundJam", true);
	m_pWeaponPartner[1]->Change_Animation(L"SK_ch2000|CP_Macaron_atk_AirJam_DoubleBaseDrop");
	//pPartners[1]->SetGroundJam();

	m_pWeaponPartnerWeaponKorsica_L->SetActive(false);
	m_pWeaponPartnerWeaponKorsica_R->SetActive(false);
	m_pWeaponPartnerWeaponPeppermint_MasterBlaster->SetActive(false);

	auto propBone = m_wpModel.lock()->FindBoneWithName(L"prop_01");
	if (m_pTarget)
		m_pTarget->SetGamBone(propBone, EGamComboType::MACARON_GROUND);
}

void CPlayer::StartHibiki()
{
	auto guitarBone = m_pGuitar->GetBone(L"g_07");
	auto& mat = guitarBone->GetDesc().m_CombinedTransformationMatrix;
	_vector vPos = XMVectorSet(mat._41, mat._42, mat._43, 1.f);
	//_vector vLook = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	//vLook = XMVector3Normalize(vLook);
	//RestartVFX(VFX_HIBIKI_GUITAR, vPos, vLook);

	//auto guitarBone = m_pGuitar->GetBone(L"root");
	//auto& mat = guitarBone->GetDesc().m_CombinedTransformationMatrix;
	//_vector vPos = XMVectorSet(mat._41, mat._42, mat._43, 1.f);
	//_vector vLook = XMVectorSet(mat._31, mat._32, mat._33, 0.f);
	//vLook = XMVector3Normalize(vLook);

	m_pPlayerCam->SetHandle(true);
	WAVE_DESC Desc = {};
	Desc.fRange = { 0.5f };
	Desc.fStrength = { 0.2f };
	Desc.fSpeed = { 20.f };
	Desc.fMaxDist = { 10.f };
	Desc.vPosition = vPos;
	m_pGameInstance->AddWave(Desc);
	Desc.fSpeed = { 30.f };
	m_pGameInstance->AddWave(Desc);
	Desc.fSpeed = { 40.f };
	m_pGameInstance->AddWave(Desc);
	Desc.fSpeed = { 50.f };
	m_pGameInstance->AddWave(Desc);

	m_pGameInstance->SetColorReverse(1);
	m_bCanMove = false;
	m_bisHibiki = true;
	m_bSkipUpdate = true;
	SetBoolElement("Enter_Hibiki", true);
	m_pUCGamePlay->UCRhythmNoteActive(true, 0);
	m_pGuitar->Set_GuitarEmissive(3.f);
	m_pGameInstance->ChangeLayerTimeScale(m_pGameInstance->GetCurrentLevelTag(),
		L"Layer_Monster", 0.1f);
}

void CPlayer::CheckHibiKi()
{
	if (!m_bisHibiki) return;



	_bool SuccessHibiki = m_pGameInstance->IsKeyState(DIK_NUMPAD7, EKEYACTIONSTATE::ENTER);
	_bool FailHibiki = m_pGameInstance->IsKeyState(DIK_NUMPAD8, EKEYACTIONSTATE::ENTER);

	_bool LClick = m_pGameInstance->IsMouseBtnState(EMOUSEBTN::LB, EKEYACTIONSTATE::STAY);
	_bool RClick = m_pGameInstance->IsMouseBtnState(EMOUSEBTN::RB, EKEYACTIONSTATE::STAY);

	_int  HibikiRes = m_pUCRhythmNote->Get_HibikiResult();


	if (HibikiRes != 3 && !m_pColorReserveTimer->IsActive() && !m_bHibikiOnce)
	{
		m_bHibikiOnce = true;
		if (HibikiRes == 0 || HibikiRes == 1) Success_Hibiki();
		if (HibikiRes == 2)   Fail_Hibiki();

		m_pColorReserveTimer->Restart();

	}


	_bool Checkable = m_pUCRhythmNote->CheckInput();
	_bool CheckMiss = m_pUCRhythmNote->CheckMiss();

	if (Checkable)
	{
		if (LClick)
			m_pUCRhythmNote->CheckHit(CUC_RhythmNote::INPUTTYPE::MOUSE, 1);
		else if (RClick)
			m_pUCRhythmNote->CheckHit(CUC_RhythmNote::INPUTTYPE::MOUSE, 2);
	}
	else
	{
		if (CheckMiss)
			m_pUCRhythmNote->FailHit();
	}
}

void CPlayer::Success_Hibiki()
{
	m_pGameInstance->Play_SFX(L"HibikiCall");
	SetBoolElement("Success_Hibiki", true);
	SetScoreMul(4.f, 1);
	m_eBeatHitType = SP_HIBIKI_SWING;
	m_bSkipUpdate = false;
	m_pGameInstance->ChangeLayerTimeScale(m_pGameInstance->GetCurrentLevelTag(),
		L"Layer_Monster", 1.f);
}

void CPlayer::Fail_Hibiki()
{
	SetBoolElement("Fail_Hibiki", true);
	m_pGameInstance->ChangeLayerTimeScale(m_pGameInstance->GetCurrentLevelTag(),
		L"Layer_Monster", 1.f);
}

void CPlayer::EndHibiki()
{
	m_bHibikiOnce = false;
	m_bCanMove = true;
	m_bisHibiki = false;
	SetBoolElement("Enter_Hibiki", false);
	SetBoolElement("Fail_Hibiki", false);
	SetBoolElement("Success_Hibiki", false);
	m_bSkipUpdate = false;

	m_pPlayerCam->SetHandle(false);
	m_bInvincible = false;
}

void CPlayer::StartSP808()
{
	Switch808_Cat();
	SetBoolElement("808_SP", true);
	m_p808_Cat->Change_Animation(15, 0.2f, 1.f, false);
	m_pGameInstance->Play_SFX(L"GIGA808CatAtkCall");
	m_pUCEffect->SACut_808Attack_Start();
	SetScoreMul(4.f, 1);
	auto pBone = m_wpModel.lock()->FindBoneWithName(L"prop_02");
	m_pPlayerCam->SetTargetBone(pBone, 0.4f);
	m_bSkipUpdate = true;
}

void CPlayer::EndSP808()
{
	Switch808_Ball();
	SetBoolElement("808_SP", false);
	m_pPlayerCam->SetTargetBone(nullptr, 0.4f);
	m_bSkipUpdate = false;
	m_bInvincible = false;
}

void CPlayer::ChangeSP()
{
	m_iCurSPIdx = (!m_iCurSPIdx) ? 1 : 0;
	m_pUCWidget->SASwap();
}

void CPlayer::HitEnemy()
{
	++m_PlayerState.m_iComboCnt;

	if (m_PlayerState.m_iComboCnt % 10 == 0)
	{
		AddScore(200);
		m_pUCScore->UpdateBonus(3);
	}

	AddScore(20);
	AddEnergy(10);
	AddRankScore(15);

	if (m_bGoodTime)
	{
		AddRankScore(20);
		++m_iJustTimming;
	}

	if (m_bPerfectTime)
	{
		AddRankScore(30);
		++m_iJustTimming;
	}
	m_pAtkCntTimer->Restart();
	m_pPlayerCam->StartCamShake(0.25f, 0.2f, 25.f);
}

void CPlayer::EnemyKill(_int _score, _int _energy, EAttackType eKillType)
{
	AddScore(_score);
	AddEnergy(_energy);

	AddRankScore(30);

	if (eKillType == EAttackType::BEATHIT)
		AddRankScore(40);
	else if (eKillType == EAttackType::FINISHER)
		AddRankScore(60);
	else if (eKillType == EAttackType::AIRATK)
		AddRankScore(20);
}

void CPlayer::AddScore(_int iScore)
{
	m_PlayerState.m_iScore += static_cast<_int>(iScore * m_fScoreMul);

	if (m_PlayerState.m_iScore < 0)
		m_PlayerState.m_iScore = 0;
}
void CPlayer::OnBeatHitSuccess()
{
	AddJustCnt();
	m_p808_Ball->Restart_Onbeat_VFX(m_iCurJustCnt);

	AddScore(100);
	AddEnergy(15);
	AddRankScore(50);

	SetScoreMul(2.f);

	PlayRandomSound(L"Pl_at_Input_BestTiming_01_Play", 2, 5);
	m_pPlayerCam->StartCameraFovKick(XMConvertToRadians(-10.f), 2.f, 0.3f, 0.05f);
}
void CPlayer::OnDamagePenalty()
{
	AddRankScore(-50);
	m_PlayerState.m_iComboCnt = 0;
	if (!m_pUCScore->IsFinalResultEnd())
		SetScoreMul(0.7f, 4);
}

void CPlayer::AddEnergy(_int iEnergy)
{
	m_PlayerState.m_iCurEnergy += iEnergy;

	m_PlayerState.m_iCurEnergy =
		clamp(m_PlayerState.m_iCurEnergy, 0, m_PlayerState.m_iMaxEnergy);
}
void CPlayer::AddRankScore(_int iValue)
{
	m_PlayerState.m_iRankScore += iValue;
	m_PlayerState.m_iRankScore = clamp(m_PlayerState.m_iRankScore, 0, 1000);
}

void CPlayer::UpdateBattleRank()
{
	m_PlayerState.m_iRankScore = clamp(m_PlayerState.m_iRankScore, 0, 1000);

	if (m_PlayerState.m_iRankScore < 200)
		m_PlayerState.m_eRank = RANK_D;
	else if (m_PlayerState.m_iRankScore < 400)
		m_PlayerState.m_eRank = RANK_C;
	else if (m_PlayerState.m_iRankScore < 600)
		m_PlayerState.m_eRank = RANK_B;
	else if (m_PlayerState.m_iRankScore < 800)
		m_PlayerState.m_eRank = RANK_A;
	else
		m_PlayerState.m_eRank = RANK_S;
}
void CPlayer::UpdateTimingRank()
{
	if (m_iInputAttack <= 0)
	{
		m_fJustTimeRatio = 0.f;
		m_PlayerState.m_eTimingRank = RANK_D;
		return;
	}

	m_fJustTimeRatio =
		static_cast<_float>(m_iJustTimming) / static_cast<_float>(m_iInputAttack);

	m_fJustTimeRatio = clamp(m_fJustTimeRatio, 0.f, 1.f);

	if (m_fJustTimeRatio >= 0.9f)
		m_PlayerState.m_eTimingRank = RANK_S;
	else if (m_fJustTimeRatio >= 0.7f)
		m_PlayerState.m_eTimingRank = RANK_A;
	else if (m_fJustTimeRatio >= 0.5f)
		m_PlayerState.m_eTimingRank = RANK_B;
	else if (m_fJustTimeRatio >= 0.2f)
		m_PlayerState.m_eTimingRank = RANK_C;
	else
		m_PlayerState.m_eTimingRank = RANK_D;

	m_PlayerState.m_fTimingRatio = m_fJustTimeRatio;
}

void CPlayer::BattleResultUI()
{
	if (m_pUCGamePlay)
	{
		PrintDebug(L"CPlayer BattleTime : ", m_fElasedBattleTime);
		PrintDebug(L"");
		m_pUCScore->SetCombatTime(m_fElasedBattleTime);
		m_pUCGamePlay->SetScoreCalc(false);
	}
}

void CPlayer::Notice_DSP_Perfect()
{
	m_pGamePlay->SetRhythmBeat(1.f);
}

void CPlayer::SetEmissive()
{
	Engine::CRenderer::SHADERPARAM_DESC desc = {};
	desc.eParamType = { Engine::CRenderer::EPARAMTYPE::PT_RAWDATA };
	desc.iSize = { sizeof(Engine::_float3) };
	desc.sConstantName = { "g_vEmissiveColor" };
	desc.pData = { &m_f3EmissiveColor };
	Engine::_int iNum = { 0 };
	while (auto spMeshRenderer = m_wpModel.lock()->GetMeshRenderer(iNum))
	{
		spMeshRenderer->Add_Parameters(desc);
		++iNum;
	}
}

void CPlayer::UpdateEmissive(Engine::_float fTimeDelta)
{
	if (m_f3EmissiveColor.x > 0.f)
		m_f3EmissiveColor.x -= fTimeDelta;
	if (m_f3EmissiveColor.x < 0.f)
		m_f3EmissiveColor.x = { 0.f };
	if (m_f3EmissiveColor.y > 0.f)
		m_f3EmissiveColor.y -= fTimeDelta;
	if (m_f3EmissiveColor.y < 0.f)
		m_f3EmissiveColor.y = { 0.f };
	if (m_f3EmissiveColor.z > 0.f)
		m_f3EmissiveColor.z -= fTimeDelta;
	if (m_f3EmissiveColor.z < 0.f)
		m_f3EmissiveColor.z = { 0.f };
}

void CPlayer::CalcScore()
{
	UpdateBattleRank();
	UpdateTimingRank();
}

void CPlayer::SetScoreMul(_float _ScoreMul, _int iType)
{
	m_fScoreMul = _ScoreMul;
	m_pScoreMulTimer->Restart();
	if (iType != 0)
		m_pUCScore->UpdateBonus(iType);		// 0: None  1: Reverb  2: Parry  3: Hit  4: Ouch
}

void CPlayer::StartMagnetRail()
{
	m_pEffect[VFX_RAILSPEEDLINE]->Restart();
	m_bIsMagnetRail = true;
	m_bSkipUpdate = true;
	m_pGuitar->SetActive(false);
	m_pTrashGuitar->SetActive(false);
	SetBoolElement("Enter_MagnetRail", true);
	m_pPlayerCam->SetLerpFOV(0.5f, XM_PI / 2.f);
	m_pPlayerCam->SetHandle(true);
}

void CPlayer::StayMagnetRail(_float fTimeDelta)
{
	if (!m_bIsMagnetRail)
		return;

	auto spTransform = m_wpMainTransformCom.lock();
	auto spModel = m_wpModel.lock();

	if (!spTransform || !spModel || !m_pRail)
		return;

	auto pPlayerBone = spModel->FindBoneWithName(L"prop_00");

	auto wpRailModel = ConvertWPComponent<CModel>(
		m_pRail->Get_Component<CModel>().lock()
	);

	auto spRailModel = wpRailModel.lock();

	if (!pPlayerBone || !spRailModel)
		return;

	auto pRailBone = spRailModel->FindBoneWithName(L"origin");

	if (!pRailBone)
		return;

	auto playerBoneDesc = pPlayerBone->GetDesc();
	auto railBoneDesc = pRailBone->GetDesc();

	_matrix matCur = XMLoadFloat4x4(
		&playerBoneDesc.m_CombinedTransformationMatrix
	);

	_matrix matTarget = XMLoadFloat4x4(
		&railBoneDesc.m_CombinedTransformationMatrix
	);

	m_pEffect[VFX_RAILSPEEDLINE]->Set_Pos(GetPos());
	m_pEffect[VFX_RAILSPEEDLINE]->Set_Dir(-GetLook());

	_matrix matCurRot = XMMatrixIdentity();
	_matrix matTargetRot = XMMatrixIdentity();

	matCurRot.r[0] = XMVectorSetW(XMVector3Normalize(matCur.r[0]), 0.f);
	matCurRot.r[1] = XMVectorSetW(XMVector3Normalize(matCur.r[1]), 0.f);
	matCurRot.r[2] = XMVectorSetW(XMVector3Normalize(matCur.r[2]), 0.f);
	matCurRot.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	matTargetRot.r[0] = XMVectorSetW(XMVector3Normalize(matTarget.r[0]), 0.f);
	matTargetRot.r[1] = XMVectorSetW(XMVector3Normalize(matTarget.r[1]), 0.f);
	matTargetRot.r[2] = XMVectorSetW(XMVector3Normalize(matTarget.r[2]), 0.f);
	matTargetRot.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	_vector vDet = XMVectorZero();
	_matrix matInvCurRot = XMMatrixInverse(&vDet, matCurRot);

	if (fabsf(XMVectorGetX(vDet)) < 0.00001f)
		return;

	_matrix matRotDelta = XMMatrixMultiply(
		matInvCurRot,
		matTargetRot
	);

	vDet = XMVectorZero();
	matInvCurRot = XMMatrixInverse(&vDet, matRotDelta);

	if (fabsf(XMVectorGetX(vDet)) < 0.00001f)
		return;

	_vector vRight = spTransform->Get_LocalState(STATE::RIGHT);
	_vector vUp = spTransform->Get_LocalState(STATE::UP);
	_vector vLook = spTransform->Get_LocalState(STATE::LOOK);

	_float3 vScale = spTransform->Get_Scaled();

	vRight = XMVector3TransformNormal(vRight, matRotDelta);
	vUp = XMVector3TransformNormal(vUp, matRotDelta);
	vLook = XMVector3TransformNormal(vLook, matRotDelta);

	vRight = XMVector3Normalize(vRight) * vScale.x;
	vUp = XMVector3Normalize(vUp) * vScale.y;
	vLook = XMVector3Normalize(vLook) * vScale.z;

	vRight = XMVectorSetW(vRight, 0.f);
	vUp = XMVectorSetW(vUp, 0.f);
	vLook = XMVectorSetW(vLook, 0.f);

	spTransform->Set_LocalState(STATE::RIGHT, vRight);
	spTransform->Set_LocalState(STATE::UP, vUp);
	spTransform->Set_LocalState(STATE::LOOK, vLook);


	_vector vMove = matTarget.r[3] - matCur.r[3];
	vMove = XMVectorSetW(vMove, 0.f);

	_vector vPlayerPos = spTransform->Get_LocalState(STATE::POSITION);
	_vector vNewPos = vPlayerPos + vMove;
	vNewPos = XMVectorSetW(vNewPos, 1.f);

	spTransform->Set_LocalState(STATE::POSITION, vNewPos);


	_bool InputR = m_pGameInstance->IsKeyState(DIK_D, EKEYACTIONSTATE::STAY);
	_bool InputL = m_pGameInstance->IsKeyState(DIK_A, EKEYACTIONSTATE::STAY);
	_bool InputDodge = m_pGameInstance->IsKeyState(DIK_SPACE, EKEYACTIONSTATE::STAY);
	_int iInputType = -1;
	auto _ratio = m_pBeatHitTimer->GetElapsedRatio();

	if (m_pBeatHitTimer->IsActive() && _ratio >= 0.5f)
	{

		_bool Input = InputDodge;
		_bool bMiss = m_eRhythmResult == ERhythmResult::None;

		if ((0.68 <= _ratio && _ratio <= 0.75) && Input)
			m_eRhythmResult = ERhythmResult::Perfect;

		else if (((0.53f <= _ratio && _ratio <= 0.67f) || (0.76f <= _ratio && _ratio <= 0.79f)) && Input)
			m_eRhythmResult = ERhythmResult::Good;
		else if (_ratio > 0.8 && bMiss)
			m_eRhythmResult = ERhythmResult::Miss;


		if (m_eRhythmResult != ERhythmResult::None && m_bFirstRhythmRes)
		{
			m_bFirstRhythmRes = false;
			_bool Dodge =
				(m_eRhythmResult == ERhythmResult::Good || m_eRhythmResult == ERhythmResult::Perfect) ? true : false;

			SetBoolElement("MagnetRail_Dodge", Dodge);
			if (Dodge)
			{
				PrintDebug("RAIL DODGE!");
				SetBoolElement("MagnetRail_Dodge", true);
				ChangeAnimRail(4);
				iInputType = 2;
				m_pGameInstance->Play_SFX(L"magnetrail_dodge_Just");
				m_bSuccessRail = true;
			}

			if (m_pUCBeathit != nullptr)
				m_pUCBeathit->Set_BHJudge(m_eRhythmResult, !m_bisRp);
		}
	}
	else
		SetBoolElement("MagnetRail_Dodge", false);


	//if (InputDodge) 
	//{
	//	SetBoolElement("MagnetRail_Dodge", true);
	//	ChangeAnimRail(4);
	//	iInputType = 2;
	//}
	if (InputR)
	{
		ChangeAnimRail(2);
		SetBoolElement("MagnetRail_MoveR", true);
		iInputType = 0;
	}
	else if (InputL)
	{
		ChangeAnimRail(1);
		SetBoolElement("MagnetRail_MoveL", true);
		iInputType = 1;
	}
	else
	{
		//SetBoolElement("MagnetRail_Dodge", false);
		SetBoolElement("MagnetRail_MoveR", false);
		SetBoolElement("MagnetRail_MoveL", false);
	}



	if (m_pRail)
	{
		_bool bCollision = m_pRail->CheckCollision(iInputType);

		if (m_pRail->ConsumeType2RangeTrigger())
		{
			m_pBeatHitTimer->Restart();
			m_pUCEffect->MagnetRailBH();
		}

		if (m_pRail->ConsumeNoneType2RangeTrigger())
		{
			m_pUCEffect->MagnetRailGuide();
		}

		if (bCollision && iInputType != 2 && !m_bSuccessRail)
		{
			PrintDebug("RAIL DamaGE!");
			SetBoolElement("Magnet_Rail_Damage", true);
			ChangeAnimRail(3);
			m_PlayerState.m_fCurHp -= 5;
		}
		else
		{
			SetBoolElement("Magnet_Rail_Damage", false);
		}
	}

	auto pCamTranform = m_pPlayerCam->Get_MainTransform().lock();
	if (!pCamTranform || !m_pRail)
		return;

	_vector vTargetCamPos = m_pRail->GetCamPos();
	vTargetCamPos = XMVectorSetY(vTargetCamPos, XMVectorGetY(vTargetCamPos) - 2.5f);

	_vector vCurCamPos = pCamTranform->Get_LocalState(STATE::POSITION);

	_float fLerpSpeed = 16.f;

	_float fRatio = 1.f - expf(-fLerpSpeed * fTimeDelta);

	_vector vLerpCamPos = XMVectorLerp(vCurCamPos, vTargetCamPos, fRatio);

	pCamTranform->Set_LocalState(STATE::POSITION, vLerpCamPos);
	_vector vCamDir = m_pRail->GetCamDir();

	if (!XMVector3Equal(vCamDir, XMVectorZero()))
	{
		pCamTranform->Aim(vCamDir);
	}

}

void CPlayer::EndMangetRail()
{
	//PrintDebug("ENDRAIL!");
	m_bIsMagnetRail = false;
	m_bSkipUpdate = false;
	ActiveGuitar();
	m_pEffect[VFX_RAILSPEEDLINE]->Stop();
	m_pRail->SetEnd(false);
	m_pRail = nullptr;
	SetBoolElement("ArriveMagnet", true);
	m_pPlayerCam->SetFOV(XM_PI / 3.f);
	m_pPlayerCam->SetHandle(false);
	m_pPlayerCam->SetPitchRoll();
}

void CPlayer::ChangeAnimRail(int NodeIdx, float fBlendTime, float fExitRatio, bool bSync, int BlendType, int iBeatTransitionmask)
{
	if (!m_pRail) return;
	m_pRail->Change_Animation(NodeIdx, fBlendTime, fExitRatio, bSync, BlendType, iBeatTransitionmask);
}

/* QTE Test Code */

void CPlayer::Start_QTE(const QTEDESC& tQTEDesc, function<void()> fnSuccess, function<void()> fnFailed)
{
	if (m_pQTEManager == nullptr)
		return;

	if (m_pQTEManager->IsPlaying())
		return;

	m_bQtePlay = true;
	m_bSkipUpdate = true;
	m_pQTEManager->Start_QTE(tQTEDesc, fnSuccess, fnFailed);
	EndQTE();
	StartQTE();
}

QTEINPUTDESC CPlayer::Make_QTEInputDesc()
{
	QTEINPUTDESC tDesc{};

	const _int iPressIndex = ENUM_TO_UINT(EQTE_Key_State::QTE_KEY_PRESS);
	const _int iHoldIndex = ENUM_TO_UINT(EQTE_Key_State::QTE_KEY_HOLD);
	const _int iReleaseIndex = ENUM_TO_UINT(EQTE_Key_State::QTE_KEY_RELEASE);

	// Mouse LB
	tDesc.bLBStates[iPressIndex] =
		m_pGameInstance->IsMouseBtnState(EMOUSEBTN::LB, EKEYACTIONSTATE::ENTER);

	tDesc.bLBStates[iHoldIndex] =
		m_pGameInstance->IsMouseBtnState(EMOUSEBTN::LB, EKEYACTIONSTATE::STAY);

	tDesc.bLBStates[iReleaseIndex] =
		m_pGameInstance->IsMouseBtnState(EMOUSEBTN::LB, EKEYACTIONSTATE::EXIT);

	// Mouse RB
	tDesc.bRBStates[iPressIndex] =
		m_pGameInstance->IsMouseBtnState(EMOUSEBTN::RB, EKEYACTIONSTATE::ENTER);

	tDesc.bRBStates[iHoldIndex] =
		m_pGameInstance->IsMouseBtnState(EMOUSEBTN::RB, EKEYACTIONSTATE::STAY);

	tDesc.bRBStates[iReleaseIndex] =
		m_pGameInstance->IsMouseBtnState(EMOUSEBTN::RB, EKEYACTIONSTATE::EXIT);

	// DIK_Q
	tDesc.bDIK_QStates[iPressIndex] =
		m_pGameInstance->IsKeyState(DIK_Q, EKEYACTIONSTATE::ENTER);

	tDesc.bDIK_QStates[iHoldIndex] =
		m_pGameInstance->IsKeyState(DIK_Q, EKEYACTIONSTATE::STAY);

	tDesc.bDIK_QStates[iReleaseIndex] =
		m_pGameInstance->IsKeyState(DIK_Q, EKEYACTIONSTATE::EXIT);

	// DIK_E
	tDesc.bDIK_EStates[iPressIndex] =
		m_pGameInstance->IsKeyState(DIK_E, EKEYACTIONSTATE::ENTER);

	tDesc.bDIK_EStates[iHoldIndex] =
		m_pGameInstance->IsKeyState(DIK_E, EKEYACTIONSTATE::STAY);

	tDesc.bDIK_EStates[iReleaseIndex] =
		m_pGameInstance->IsKeyState(DIK_E, EKEYACTIONSTATE::EXIT);

	// DIK_SPACE
	tDesc.bDIK_SPACEStates[iPressIndex] =
		m_pGameInstance->IsKeyState(DIK_SPACE, EKEYACTIONSTATE::ENTER);

	tDesc.bDIK_SPACEStates[iHoldIndex] =
		m_pGameInstance->IsKeyState(DIK_SPACE, EKEYACTIONSTATE::STAY);

	tDesc.bDIK_SPACEStates[iReleaseIndex] =
		m_pGameInstance->IsKeyState(DIK_SPACE, EKEYACTIONSTATE::EXIT);

	return tDesc;
}

void CPlayer::StartQTE()
{
	SetBoolElement("Enter_QTE", true);
	m_pPlayerCam->SetHandle(true);
}

void CPlayer::EndQTE()
{
	SetBoolElement("Enter_QTE", false);
	SetBoolElement("Success_QTE", false);
	SetBoolElement("Fail_QTE", false);
}

void CPlayer::SuccessQTE()
{
	SetBoolElement("Success_QTE", true);
}

void CPlayer::FailQTE()
{
	SetBoolElement("Fail_QTE", true);
}


void CPlayer::Kill_RP_Enemy()
{
	CEnemy* pEnemy = Find_RPEnemy();
	if (!pEnemy) return;

	FDamageInfo _info = {};
	_info.pSourceObj = this;
	_info.eAttackType = EAttackType::FINISHER;

	pEnemy->OnDamage(_info);
}

CEnemy* CPlayer::Find_RPEnemy()
{
	if (m_listEnemy.empty()) return nullptr;

	for (auto pEnemy : m_listEnemy)
	{
		if (pEnemy == nullptr)
			continue;

		if (pEnemy->GetIsPlayRP())
			return pEnemy;
	}

	return nullptr;
}

_bool CPlayer::OnDamage(FDamageInfo _fDamageInfo)
{
	if (m_bisDie || m_bInvincible) return false;
	if (_fDamageInfo.eAttackType == EAttackType::ELECTRIC && m_bisRp) return true;
	if (_fDamageInfo.eAttackType == EAttackType::FIRE && m_pIgniteTimer->IsActive()) return true;

	if (m_bisRp && m_bKaleDodge)
	{
		auto eJustTime = m_bPerfectTime == true ? ERhythmResult::Perfect : ERhythmResult::Good;
		m_pUCBeathit->Set_Parry_Result(eJustTime);
		if (m_bSuccessParry)
			m_pUCScore->UpdateBonus(2);
		m_bSuccessParry = true;
		AddEnergy(10);
		AddRankScore(25);
		AddScore(15);
		m_pAtkCntTimer->Restart();
		auto pEnemy = dynamic_cast<CEnemy*>(_fDamageInfo.pSourceObj);
		if (pEnemy)pEnemy->IsParrying(eJustTime);
		m_wpAttackCollider.lock()->SetActive(false);
		return false;
	}
	if (m_bisParry && _fDamageInfo.eAttackType != EAttackType::ELECTRIC)
	{
		auto eJustTime = m_bPerfectTime == true ? ERhythmResult::Perfect : ERhythmResult::Good;
		m_pUCBeathit->Set_Parry_Result(eJustTime);
		auto pEnemy = dynamic_cast<CEnemy*>(_fDamageInfo.pSourceObj);
		if (pEnemy)pEnemy->IsParrying(eJustTime);
		if (m_bSuccessParry)
			m_pUCScore->UpdateBonus(2);
		m_bSuccessParry = true;
		RestartVFX(VFX_PARRY, GetPos(), GetLook());
		m_pPlayerCam->StartCamShake(0.4f, 0.2f, 30.f);
		AddEnergy(10);
		AddRankScore(25);
		AddScore(15);
		m_pAtkCntTimer->Restart();
		PlayRandomSound(L"pl_ch0000_act_parry_norm_01_Play", 1, 5);

		ReflectLaser(_fDamageInfo);
		m_wpAttackCollider.lock()->SetActive(false);
		return true;
	}
	m_wpAttackCollider.lock()->SetActive(false);
	m_pPlayerCam->StartCamShake(0.2f, 0.1f, 20.f);
	m_bSuccessParry = false;
	RestartVFX(VFX_HIT, Get_Center());
	_int fDamage = (Engine::_int)_fDamageInfo.fDamage;
	m_PlayerState.m_fCurHp -= fDamage;
	OnDamagePenalty();
	m_upAnimController->SetLayerIdx("HIT_Idx", 1);
	if (_fDamageInfo.eAttackType == EAttackType::RP_KNOCKBACK)
	{
		m_upAnimController->SetLayerIdx("HIT_Idx", 0);
		m_bKeyLock = true;
	}
	else if (_fDamageInfo.eAttackType == EAttackType::KNOCKBACK)
	{
		m_upAnimController->SetLayerIdx("HIT_Idx", 0);
	}
	else if (_fDamageInfo.eAttackType == EAttackType::ELECTRIC)
	{
		ElectricShock();
	}
	else if (_fDamageInfo.eAttackType == EAttackType::FIRE)
	{
		Ignite();
	}
	else if (_fDamageInfo.eAttackType == EAttackType::KALE_LASER)
	{
		m_bInvincible = true;
		m_pFlickerTimer->Restart();
		m_upAnimController->SetLayerIdx("HIT_Idx", 0);
	}

	if (m_PlayerState.m_fCurHp <= 0)
	{
		m_PlayerState.m_iCurEnergy = 0;
		m_PlayerState.m_iComboCnt = 0;
		m_PlayerState.m_fCurHp = 0;
		SetBoolElement("DIE", true);
		m_bisDie = true;
		//ClearEnemy();
		m_stBestMagnetTarget = {};
		return false;

	}
	if (_fDamageInfo.eAttackType != EAttackType::GRAB)
		SetBoolElement("HIT", true);
	m_pHitTimer->Restart();

	PrintDebug("CurHP:", m_PlayerState.m_fCurHp);


	return false;
}
