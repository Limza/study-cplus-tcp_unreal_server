#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

/* ------------------------------
 *	IocpCore
 ------------------------------ */
IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject) const
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle()
		, _iocpHandle
		, /*key*/0
		, 0);
}

bool IocpCore::Dispatch(const uint32 timeoutMs) const
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;
	const BOOL completionStatus = ::GetQueuedCompletionStatus(_iocpHandle
		, &numOfBytes
		, OUT &key
		, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent)
		, timeoutMs);

	if (completionStatus)
	{
		const IocpObjectRef iocpObject = iocpEvent->GetOwner();
		iocpObject->Dispatch(iocpEvent, static_cast<int32>(numOfBytes));
	}
	else
	{
		if (const int32 errCode = ::WSAGetLastError(); errCode == WAIT_TIMEOUT)
			return false;
		else
		{
			// TODO: 에러 처리
			const IocpObjectRef iocpObject = iocpEvent->GetOwner();
			iocpObject->Dispatch(iocpEvent, static_cast<int32>(numOfBytes));
		}
	}

	return true;
}
