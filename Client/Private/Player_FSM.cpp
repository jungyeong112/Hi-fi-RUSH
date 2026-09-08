#include "Player_FSM.h"
#include "Player.h"
#include "GameInstance.h"

CPlayer_FSM::CPlayer_FSM()
	: CFSM(CPlayer::PLAYERSTATE::PS_END)
{
}

HRESULT CPlayer_FSM::Initialize(void* pArg)
{
	CHKFAIL(__super::Initialize(pArg))

	return S_OK;
}

void CPlayer_FSM::Set_Timer()
{

}

HRESULT CPlayer_FSM::Set_State()
{
	HRESULT hr;

	hr = Set_IDLE_State();
	CHKFAIL(hr);

	hr = Set_WALK_State();
	CHKFAIL(hr);

	hr = Set_RUN_State();
	CHKFAIL(hr);

	hr = Set_JUMP_State();
	CHKFAIL(hr);

	hr = Set_DASH_State();
	CHKFAIL(hr);

	hr = Set_MAGNET_State();
	CHKFAIL(hr);

	hr = Set_COMBO_State();
	CHKFAIL(hr);

	return S_OK;
}

HRESULT CPlayer_FSM::Set_IDLE_State()
{
	using PS = CPlayer::PLAYERSTATE;
	HRESULT hr;

	hr = AddStateFunc(CPlayer::PLAYERSTATE::IDLE, EKEYACTIONSTATE::ENTER, [=](_float fTimeDelta)
		{
			*m_pData->pCurState = CPlayer::PLAYERSTATE::IDLE;
			m_pData->pOwner->WeaponChangeAnim(0);
			//PrintDebug("Chai : ENTER_IDLE");
		});
	CHKFAIL(hr);


	//hr = AddStateFunc(CPlayer::PLAYERSTATE::IDLE, EKEYACTIONSTATE::STAY, [=](_float fTimeDelta)
	//	{
 //			PrintDebug("Chai");
	//	});
	//CHKFAIL(hr);



	return S_OK;
}

HRESULT CPlayer_FSM::Set_WALK_State()
{
	return S_OK;
}

HRESULT CPlayer_FSM::Set_RUN_State()
{
	using PS = CPlayer::PLAYERSTATE;
	HRESULT hr;


	hr = AddTransitionFunc(
		PS::IDLE, PS::RUN,
		[=]() {
			return *m_pData->pInputMove;
				
		; },
		0.1f
	);
	CHKFAIL(hr);

	hr = AddTransitionFunc(
		PS::RUN, PS::IDLE,
		[=]() {
			return !*m_pData->pInputMove;

			; },
		0.1f
	);
	CHKFAIL(hr);



	hr = AddStateFunc(CPlayer::PLAYERSTATE::RUN, EKEYACTIONSTATE::ENTER, [=](_float fTimeDelta)
		{
			*m_pData->pCurState = CPlayer::PLAYERSTATE::RUN;
			//PrintDebug("Chai : ENTER_Run");
		});
	CHKFAIL(hr);




	return S_OK;
}

HRESULT CPlayer_FSM::Set_JUMP_State()
{
	using PS = CPlayer::PLAYERSTATE;
	HRESULT hr;

	hr = AddTransitionFunc(
		PS::IDLE, PS::JUMP,
		[=]() {
			return *m_pData->pIsJump;

			; },
		0.1f
	);
	CHKFAIL(hr);

	hr = AddTransitionFunc(
		PS::RUN, PS::JUMP,
		[=]() {
			return *m_pData->pIsJump;
			; },
		0.1f
	);
	CHKFAIL(hr);


	hr = AddTransitionFunc(
		PS::JUMP, PS::IDLE,
		[=]() {
			return !*m_pData->pIsJump;

			; },
		0.1f
	);
	CHKFAIL(hr);



	hr = AddStateFunc(CPlayer::PLAYERSTATE::JUMP, EKEYACTIONSTATE::ENTER, [=](_float fTimeDelta)
		{
			*m_pData->pCurState = CPlayer::PLAYERSTATE::JUMP;
			//PrintDebug("Chai : ENTER_JUMP");
		});
	CHKFAIL(hr);
	
	return S_OK;
}

HRESULT CPlayer_FSM::Set_DASH_State()
{
	using PS = CPlayer::PLAYERSTATE;
	HRESULT hr;

	hr = AddForceTransitionFunc(
		PS::DASH,
		[=]() {
			return *m_pData->pIsDash;
			; },
		0.1f
    );
	CHKFAIL(hr);

	hr = AddTransitionFunc(
		PS::DASH, m_iPrevState,
		[=]() {
			return !*m_pData->pIsDash;
			; },
		0.1f
	);
	CHKFAIL(hr);



	hr = AddStateFunc(CPlayer::PLAYERSTATE::DASH, EKEYACTIONSTATE::ENTER, [=](_float fTimeDelta)
		{
			*m_pData->pCurState = CPlayer::PLAYERSTATE::DASH;
			//PrintDebug("Chai : ENTER_Dash");
		});
	CHKFAIL(hr);

	return S_OK;
}

HRESULT CPlayer_FSM::Set_MAGNET_State()
{

	using PS = CPlayer::PLAYERSTATE;
	HRESULT hr;

	hr = AddForceTransitionFunc(
		PS::MAGNET,
		[=]() {
			return *m_pData->pIsMagnet;
			; },
		-0.1f
	);
	CHKFAIL(hr);

	hr = AddTransitionFunc(
		PS::MAGNET, PS::JUMP,
		[=]() {
			return *m_pData->pIsJump;
			; },
		0.1f
	);
	CHKFAIL(hr);



	hr = AddStateFunc(CPlayer::PLAYERSTATE::MAGNET, EKEYACTIONSTATE::ENTER, [=](_float fTimeDelta)
		{
			*m_pData->pCurState = CPlayer::PLAYERSTATE::MAGNET;
		});
	CHKFAIL(hr);

	return S_OK;
}

HRESULT CPlayer_FSM::Set_COMBO_State()
{

	using PS = CPlayer::PLAYERSTATE;
	HRESULT hr;

	hr = AddForceTransitionFunc(
		PS::COMBO,
		[=]() {
			return *m_pData->pIsAttack;
			; },
		-0.1f
	);
	CHKFAIL(hr);

	hr = AddTransitionFunc(
		PS::COMBO, PS::JUMP,
		[=]() {
			return !*m_pData->pIsAttack && *m_pData->pIsJump;
			; },
		0.1f
	);

	hr = AddTransitionFunc(
		PS::COMBO, PS::IDLE,
		[=]() {
			return !*m_pData->pIsAttack && !*m_pData->pIsJump;
			; },
		0.1f
	);
	CHKFAIL(hr);



	hr = AddStateFunc(CPlayer::PLAYERSTATE::COMBO, EKEYACTIONSTATE::ENTER, [=](_float fTimeDelta)
		{
			*m_pData->pCurState = CPlayer::PLAYERSTATE::COMBO;
		});
	CHKFAIL(hr);

	return S_OK;
}

unique_ptr<CPlayer_FSM> CPlayer_FSM::Create(void* pArg)
{
	unique_ptr<CPlayer_FSM> upFSM = unique_ptr<CPlayer_FSM>(new CPlayer_FSM());
	if (FAILED(upFSM->Initialize(pArg))) {
		MSG_BOX("Fail to Create CPlayer_FSM");
		return nullptr;
	}
	return upFSM;
}

HRESULT CPlayer_FSM::Late_Initialize()
{
	Set_Timer();

	HRESULT hr;
	hr = Set_State();
	CHKFAIL(hr);

	m_iCurState = *m_pData->pCurState;

	return S_OK;
}
