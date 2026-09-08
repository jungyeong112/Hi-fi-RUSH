#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CBlackBoard final : public CBase
{
public:
	virtual ~CBlackBoard() = default;
protected:
	CBlackBoard();
	CBlackBoard(const CBlackBoard& prototype)				= delete;
	CBlackBoard& operator=(const CBlackBoard& prototype)	= delete;
public:
	HRESULT	AddBoolElement	(const string& _tag);
	HRESULT	AddFloatElement	(const string& _tag);
	HRESULT	AddIntElement	(const string& _tag);

	HRESULT	SetBoolElement	(const string& _tag, bool	_value);
	HRESULT	SetFloatElement	(const string& _tag, float	_value);
	HRESULT	SetIntElement	(const string& _tag, int	_value);

	HRESULT	GetBoolElement	(const string& _tag, bool&	_value) const;
	HRESULT	GetFloatElement	(const string& _tag, float& _value) const;
	HRESULT	GetIntElement	(const string& _tag, int&	_value) const;
public:
	template<typename T>
	HRESULT	AddElement(const string& _tag, T& _value)     { return E_FAIL; }
	template<>
	HRESULT	AddElement(const string& _tag, bool& _value)  { return AddBoolElement(_tag); }
	template<>
	HRESULT	AddElement(const string& _tag, float& _value) { return AddFloatElement(_tag); }
	template<>
	HRESULT	AddElement(const string& _tag, int& _value)   { return AddIntElement(_tag); }


	template<typename T>
	HRESULT	GetElement(const string& _tag, T&		_value) const { return E_FAIL; }
	template<>
	HRESULT	GetElement(const string& _tag, bool&	_value) const { return GetBoolElement	(_tag, _value); }
	template<>
	HRESULT	GetElement(const string& _tag, float&	_value) const { return GetFloatElement	(_tag, _value); }
	template<>
	HRESULT	GetElement(const string& _tag, int&		_value) const { return GetIntElement	(_tag, _value); }

	template<typename T>
	HRESULT	SetElement(const string& _tag, T _value) { return E_FAIL; }
	template<>
	HRESULT	SetElement(const string& _tag, bool		_value) { return SetBoolElement	(_tag, _value); }
	template<>
	HRESULT	SetElement(const string& _tag, float	_value) { return SetFloatElement(_tag, _value); }
	template<>
	HRESULT	SetElement(const string& _tag, int		_value)	{ return SetIntElement	(_tag, _value); }
public:
	static CBlackBoard* Create();

	unordered_map<string, bool>       m_Bools;
	unordered_map<string, float>      m_Floats;
	unordered_map<string, int>        m_Ints;
};
NS_END

