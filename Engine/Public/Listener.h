#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CListener : public CBase
{
private:
	CListener(function<void(float)> funcUpdate, function<void(void)> funcEnter, function<void(void)> funcExit, _bool IsOnce);
	virtual ~CListener() = default;
public:
	static CListener* Create(function<void(float)> funcUpdate, function<void(void)> funcEnter, function<void(void)> funcExit, _bool IsOnce);
	
	bool Update(float _fDeltaTime);

	void Activate(_float duration);
	bool Deactivate();
	bool IsActive();
	void Reset();
private:
	void Enter();
	void Exit();

private:
	bool	m_bIsActive		= { false };
	bool	m_bOnce			= { false };
	_float	m_fDuration		= 0.f;
	_float	m_fElapsedTime	= 0.f;


	function<void(float)>	m_Updatefunc	= nullptr;
	function<void(void)>	m_Enterfunc		= nullptr;
	function<void(void)>	m_Exitfunc		= nullptr;
};

NS_END