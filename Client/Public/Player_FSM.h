#pragma once


#include "FSM.h"
#include "Client_Defines.h" 
#include "Player.h"


// Speed
// Kep Press
//

//Idle
//Run
//Sprint
//
//Jump
//AIR
// 
//DASH
//ROLL


NS_BEGIN(Client)
class CPlayer_FSM final : public CFSM
{
public:
	virtual ~CPlayer_FSM() = default;
private:
	CPlayer_FSM();
	CPlayer_FSM(const CPlayer_FSM& Prototype) = delete;
	CPlayer_FSM& operator=(const CPlayer_FSM& Prototype) = delete;
public:
	void Connect_Data(CPlayer::Transit_DATA* pData) { m_pData = pData; }
	static unique_ptr<CPlayer_FSM> Create(void* pArg);
	HRESULT Late_Initialize();
private:
	HRESULT Initialize(void* pArg) override;
private:
	CPlayer::Transit_DATA* m_pData = nullptr;
private:
	void    Set_Timer();
	HRESULT Set_State();

private:
	HRESULT Set_IDLE_State();
	HRESULT Set_WALK_State();
	HRESULT Set_RUN_State();
	HRESULT Set_JUMP_State();
	HRESULT Set_DASH_State();
	HRESULT Set_MAGNET_State();
	HRESULT Set_COMBO_State();
};

NS_END

