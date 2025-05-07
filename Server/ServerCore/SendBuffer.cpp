#include "pch.h"
#include "SendBuffer.h"

/* --------------------------------
 * SendBuffer
 --------------------------------*/

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, const int32 allocSize)
	: _owner(owner), _buffer(buffer), _allocSize(allocSize)
{
	
}

void SendBuffer::Close(const uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}


/* --------------------------------------
 *	SendBufferChunk
 -------------------------------------- */

SendBufferChunk::SendBufferChunk()
= default;

SendBufferChunk::~SendBufferChunk()
= default;

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(const uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= send_buffer_chunk_size);
	ASSERT_CRASH(_open == false);

	if (allocSize > FreeSize())
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(const uint32 writeSize)
{
	ASSERT_CRASH(_open == true);
	_open = false;
	_usedSize += writeSize;
}


/* --------------------------------------
 *	SendBufferManager
 -------------------------------------- */

SendBufferRef SendBufferManager::Open(const uint32 size)
{
	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop(); // WRITE_LOCK;
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	// 다 썻으면 버리고 새거로 교체
	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop(); // WRITE_LOCK;
		LSendBufferChunk->Reset();
	}

	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false)
		{
			auto sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	return {xnew<SendBufferChunk>(), PushGlobal};
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	std::cout << "PushGlobal SendBufferChunk" << '\n';
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}
