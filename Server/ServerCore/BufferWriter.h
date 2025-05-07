#pragma once

/* ---------------------------------------------------------------
 * BufferWriter
 --------------------------------------------------------------- */
class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();
	NON_COPYABLE_CLASS(BufferWriter);

	[[nodiscard]] BYTE*		Buffer() const { return _buffer; }
	[[nodiscard]] uint32	Size() const { return _size; }
	[[nodiscard]] uint16	WriteSize() const { return _pos; }
	[[nodiscard]] uint32	FreeSize() const { return _size - _pos; }

	template<typename T>
	bool Write(T* src) { return Write(src, sizeof(T)); }
	bool Write(const void* src, uint32 len);

	template<typename T>
	[[nodiscard]] T* Reserve();

	template<typename T>
	BufferWriter& operator<<(T&& src);

private:
	BYTE*	_buffer = nullptr;
	uint32	_size = 0;
	uint16	_pos = 0;
};

template<typename T>
T* BufferWriter::Reserve()
{
	if (FreeSize() < sizeof(T))
		return nullptr;

	T* ptr = reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return ptr;
}

template <typename T>
BufferWriter& BufferWriter::operator<<(T&& src)
{
	using DataType = std::remove_reference_t<T>;
	*reinterpret_cast<DataType*>(&_buffer[_pos]) = std::forward<DataType>(src);
	_pos += sizeof(T);
	return *this;
}