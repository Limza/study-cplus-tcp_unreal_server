#include "pch.h"
#include "BufferReader.h"

/* ---------------------------------------------------------------
 * BufferReader
 --------------------------------------------------------------- */

BufferReader::BufferReader()
= default;

BufferReader::BufferReader(BYTE* buffer, const uint32 size, const uint32 pos)
	: _buffer(buffer), _size(size)	, _pos(pos)
{
}

BufferReader::~BufferReader()
= default;

bool BufferReader::Peek(void* dest, const uint32 len) const
{
	if (FreeSize() < len)
		return false;

	::memcpy(dest, &_buffer[_pos], len);
	return true;
}

bool BufferReader::Read(void* dest, const uint32 len)
{
	if (Peek(dest, len) == false)
		return false;

	_pos += len;
	return true;
}
