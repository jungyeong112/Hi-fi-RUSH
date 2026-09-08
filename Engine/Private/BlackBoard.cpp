#include "BlackBoard.h"

CBlackBoard::CBlackBoard()
{
}

HRESULT CBlackBoard::AddBoolElement(const string& _tag)
{
	if (m_Bools.count(_tag)) {
		//MSG_BOXL((L"already has same tag : " + s2ws(_tag)).c_str());
		return S_OK;
	}
	m_Bools[_tag] = false;
	return S_OK;
}

HRESULT CBlackBoard::AddFloatElement(const string& _tag)
{
	if (m_Floats.count(_tag)) {
		//MSG_BOXL((L"already has same tag : " + s2ws(_tag)).c_str());
		return S_OK;
	}
	m_Floats[_tag] = 0.f;
	return S_OK;
}

HRESULT CBlackBoard::AddIntElement(const string& _tag)
{
	if (m_Ints.count(_tag)) {
		//MSG_BOXL((L"already has same tag : " + s2ws(_tag)).c_str());
		return S_OK;
	}
	m_Ints[_tag] = 0;
	return S_OK;
}

HRESULT CBlackBoard::SetBoolElement(const string& _tag, bool _value)
{
	if (!m_Bools.count(_tag)) {
		MSG_BOXL((L"does not have tag : " + s2ws(_tag)).c_str());
		return E_FAIL;
	}
	m_Bools[_tag] = _value;
	return S_OK;
}

HRESULT CBlackBoard::SetFloatElement(const string& _tag, float _value)
{
	if (!m_Floats.count(_tag)) {
		MSG_BOXL((L"does not have tag : " + s2ws(_tag)).c_str());
		return E_FAIL;
	}
	m_Floats[_tag] = _value;
	return S_OK;
}

HRESULT CBlackBoard::SetIntElement(const string& _tag, int _value)
{
	if (!m_Ints.count(_tag)) {
		MSG_BOXL((L"does not have tag : " + s2ws(_tag)).c_str());
		return E_FAIL;
	}
	m_Ints[_tag] = _value;
	return S_OK;
}

HRESULT CBlackBoard::GetBoolElement(const string& _tag, bool& _value) const
{
	if (!m_Bools.count(_tag)) {
		MSG_BOXL((L"does not have tag : " + s2ws(_tag)).c_str());
		return E_FAIL;
	}
	_value = m_Bools.find(_tag)->second;
	return S_OK;
}

HRESULT CBlackBoard::GetFloatElement(const string& _tag, float& _value) const
{
	if (!m_Floats.count(_tag)) {
		MSG_BOXL((L"does not have tag : " + s2ws(_tag)).c_str());
		return E_FAIL;
	}
	_value = m_Floats.find(_tag)->second;
	return S_OK;
}

HRESULT CBlackBoard::GetIntElement(const string& _tag, int& _value) const
{
	if (!m_Ints.count(_tag)) {
		MSG_BOXL((L"does not have tag : " + s2ws(_tag)).c_str());
		return E_FAIL;
	}
	_value = m_Ints.find(_tag)->second;
	return S_OK;
}

CBlackBoard* CBlackBoard::Create()
{
	return new CBlackBoard;
}
