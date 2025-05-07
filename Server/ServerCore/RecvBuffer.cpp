#include "pch.h"
#include "RecvBuffer.h"

/* ------------------------------
 * RecvBuffer
 ------------------------------ */
RecvBuffer::RecvBuffer(const int32 bufferSize)
	: _capacity(bufferSize * _bufferCount), _bufferSize(bufferSize)
{
	_buffer.resize(_capacity);
}

void RecvBuffer::Clean()
{
	if (const int32 dataSize = DataSize(); dataSize == 0)
	{
		_readPos = _writePos = 0;
	}
	else
	{
		if (FreeSize() < _bufferSize)
		{
			std::copy(_buffer.begin() + _readPos, _buffer.begin() + _writePos, _buffer.begin());
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

bool RecvBuffer::OnRead(const int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(const int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;

	_writePos += numOfBytes;
	return true;
}
