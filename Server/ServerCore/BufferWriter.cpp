#include "pch.h"
#include "BufferWriter.h"

BufferWriter::BufferWriter()
= default;

BufferWriter::BufferWriter(BYTE* buffer, const uint32 size, const uint32 pos)
	: _buffer(buffer), _size(size), _pos(pos)
{
}

BufferWriter::~BufferWriter()
= default;

bool BufferWriter::Write(const void* src, const uint32 len)
{
	if (FreeSize() < len)
		return false;

	::memcpy(&_buffer[_pos], src, len);
	_pos += len;
	return true;
}
