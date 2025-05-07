#pragma once

/* ------------------------------
 *	IocpObject
 ------------------------------ */
class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	IocpObject() = default;
	virtual ~IocpObject() = default;
	NON_COPYABLE_CLASS(IocpObject);

	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) = 0;
};

/* ------------------------------
 *	IocpCore
 ------------------------------ */
class IocpCore
{
public:
	IocpCore();
	~IocpCore();
	NON_COPYABLE_CLASS(IocpCore);

	[[nodiscard]] HANDLE GetHandle() const { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject) const;
	bool Dispatch(uint32 timeoutMs = INFINITE) const;

private:
	HANDLE _iocpHandle;
};
