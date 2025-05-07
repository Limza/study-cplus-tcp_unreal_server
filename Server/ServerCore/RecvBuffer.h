#pragma once

/* ------------------------------
 * RecvBuffer
 ------------------------------ */
class RecvBuffer
{
	static constexpr int _bufferCount= 10;
public:
	explicit RecvBuffer(int32 bufferSize);
	~RecvBuffer() = default;
	NON_COPYABLE_CLASS(RecvBuffer);

public:
	BYTE* ReadPos() { return &_buffer[_readPos]; }
	BYTE* WritePos() { return &_buffer[_writePos]; }
	[[nodiscard]] int32 DataSize() const { return _writePos - _readPos; }
	[[nodiscard]] int32 FreeSize() const { return _capacity - _writePos; }

public:
	void Clean();
	bool OnRead(int32 numOfBytes);
	bool OnWrite(int32 numOfBytes);

private:
	int32			_capacity = 0;
	int32			_bufferSize = 0;
	int32			_readPos = 0;
	int32			_writePos = 0;
	Vector<BYTE>	_buffer;
};

