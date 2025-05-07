#pragma once

class SendBufferChunk;

/* --------------------------------
 * SendBuffer
 --------------------------------*/
class SendBuffer
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, int32 allocSize);
	~SendBuffer() = default;
	NON_COPYABLE_CLASS(SendBuffer);

public:
	[[nodiscard]] BYTE* Buffer() const { return _buffer; }
	[[nodiscard]] uint32 AllocSize() const { return _allocSize; }
	[[nodiscard]] uint32 WriteSize() const { return _writeSize; }

	void Close(uint32 writeSize);

private:
	SendBufferChunkRef	_owner;
	BYTE*				_buffer;
	uint32				_allocSize = 0;
	uint32				_writeSize = 0;
};


/* --------------------------------------
 *	SendBufferChunk
 -------------------------------------- */
class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	static constexpr int32 send_buffer_chunk_size = 6000;

public:
	SendBufferChunk();
	~SendBufferChunk();
	NON_COPYABLE_CLASS(SendBufferChunk);

public:
	void			Reset();
	SendBufferRef	Open(uint32 allocSize);
	void			Close(uint32 writeSize);

	bool			IsOpen() const { return _open; }
	BYTE*			Buffer() { return &_buffer[_usedSize]; }
	uint32			FreeSize() const { return static_cast<uint32>(_buffer.size()) - _usedSize; }

private:
	bool	_open = false;
	uint32	_usedSize = 0;

	Array<BYTE, send_buffer_chunk_size>	_buffer = {};
};


 /* --------------------------------------
  *	SendBufferManager
  -------------------------------------- */
class SendBufferManager
{
public:
	SendBufferRef		Open(uint32 size);

private:
	SendBufferChunkRef	Pop();
	void				Push(SendBufferChunkRef buffer);

	static void			PushGlobal(SendBufferChunk* buffer);

private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;
};