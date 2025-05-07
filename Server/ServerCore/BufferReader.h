#pragma once

/* ---------------------------------------------------------------
 * BufferReader
 --------------------------------------------------------------- */
class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();
	NON_COPYABLE_CLASS(BufferReader);

	[[nodiscard]] BYTE*		Buffer() const { return _buffer; }
	[[nodiscard]] uint32	Size() const { return _size; }
	[[nodiscard]] uint32	ReadSize() const { return _pos; }
	[[nodiscard]] uint32	FreeSize() const { return _size - _pos; }

	template<typename T>
	bool Peek(T* dest) {return Peek(dest, sizeof(T));	}
	bool Peek(void* dest, uint32 len) const;

	template<typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); }
	bool Read(void* dest, uint32 len);

	template<typename T>
	BufferReader& operator>>(OUT T& dest);

private:
	BYTE*	_buffer = nullptr;
	uint32	_size = 0;
	uint32	_pos = 0;
};

template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& dest)
{
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return *this;
}