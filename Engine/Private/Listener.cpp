#include "Listener.h"

CListener::CListener(function<void(float)> funcUpdate, function<void(void)> funcEnter, function<void(void)> funcExit, _bool IsOnce)
	:	m_Updatefunc(funcUpdate),
		m_Enterfunc	(funcEnter),
		m_Exitfunc	(funcExit),
		m_bOnce		(IsOnce)
{
}

CListener* CListener::Create(function<void(float)> funcUpdate, function<void(void)> funcEnter, function<void(void)> funcExit, _bool IsOnce)
{
	CListener* pListener = new CListener(funcUpdate, funcEnter, funcExit, IsOnce);
	return pListener;
}

void CListener::Enter()
{
	if (m_Enterfunc) m_Enterfunc();
}

bool CListener::Update(float _fDeltaTime)
{
	if (!m_bIsActive) return false;
	if (m_Updatefunc) m_Updatefunc(_fDeltaTime);
	m_fElapsedTime += _fDeltaTime;
	if (m_fDuration > 0.0001f && m_fElapsedTime > m_fDuration) return Deactivate();
	return false;
}

void CListener::Exit()
{
	if (m_Exitfunc) m_Exitfunc();
}

void CListener::Activate(_float duration)
{
	m_fElapsedTime	= 0.f;
	m_fDuration		= duration;

	if (!m_bIsActive) Enter();
	m_bIsActive = true;
}

bool CListener::Deactivate()
{
	if (m_bIsActive) Exit();
	m_bIsActive = false;

	return m_bOnce;
}

bool CListener::IsActive()
{
	return m_bIsActive;
}

void CListener::Reset()
{
	m_bIsActive		= false;
	m_fElapsedTime	= 0.f;
}
