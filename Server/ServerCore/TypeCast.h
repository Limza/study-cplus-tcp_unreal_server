#pragma once
#include "Types.h"

#pragma region TypeList. ���� ���� Ÿ���� ��� ����Ʈó�� ����
/*
 * �������ڸ� �޴� TypeList���� �� ����ü�� ���� ����
 */
template <typename... T>
struct TypeList;

/*
 * �� ���� Ÿ���� ���� �� �ִ� ����Ʈ.
 * ex => using TL = TypeList<int, float>;
 * TL::Head // int
 * TL::Tail // float
 */
template <typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

/*
 * ���� ���ڸ� �޾� ���� ���� Ÿ���� ���� �� �ִ� ����Ʈ.
 * ex => using TL = TypeList<int, float, double>;
 * 1. Head = int, Tail = TypeList<float, double>
 * 2. Head = float, Tail = TypeList<double>
 * 3. Head = double, Tail = TypeList<>
 */ 
template <typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};
#pragma endregion -------------------------------------------


#pragma region Length. TypeList�� ���̸� ����ϴ� ���ø�
template <typename... TL>
constexpr int32 Length(TypeList<TL...>) {
	return sizeof...(TL);
}
#pragma endregion -------------------------------------------


#pragma region TypeAt. N��° Ÿ���� �����ϴ� ���ø�
/*
 * tuple_element_t. TupleType���� Index ��° Ÿ���� �������� std Ȱ��
 */
template <int32 Index, typename... Types>
struct TypeAt
{
	static_assert(Index < sizeof...(Types), "Index out of range");
	using Result = std::tuple_element_t<Index, std::tuple<Types...>>;
};

// TypeList<Types...>�� �����ϴ� Ư��ȭ �߰�
template <int32 Index, typename... Types>
struct TypeAt<Index, TypeList<Types...>> {
	static_assert(Index < sizeof...(Types), "Index out of range!");
	using Result = std::tuple_element_t<Index, std::tuple<Types...>>;
};
#pragma endregion -------------------------------------------


#pragma region IndexOf. TypeList���� Ư�� Ÿ�� T�� �ε����� ã�´�
template <typename TL, typename T>
struct IndexOf;

template <typename T, typename... Tail>
struct IndexOf<TypeList<T, Tail...>, T> {
	static constexpr int32 value = 0;
};

template <typename T>
struct IndexOf<TypeList<>, T> {
	static constexpr int32 value = -1;
};

/*
 * ��������� Ž��
 * TypeList<Head, Tail...>���� ù ��° Ÿ��(Head)�� T�� �ƴ϶��, Tail...�� ��� �˻�
 * TEMP�� IndexOf<TypeList<Tail...>, T>::VALUE ��
 * ���� TEMP == -1�̸� T�� ����Ʈ�� �����Ƿ� VALUE = -1
 * �׷��� ������ VALUE = 1 + TEMP �� ��, T�� �߰ߵ� ��ġ�� 1�� ���ؼ� ���� �ε����� ��ȯ.
 *
 * using MyList = TypeList<int, float, double>;
 * constexpr int index = IndexOf<MyList, float>::VALUE; // index == 1
 * 
 * 1. Head = int, Tail = TypeList<float, double>, TEMP = IndexOf<TypeList<float, double>, float>::VALUE
 * 2. Head = float, Tail = TypeList<double>, TEMP = 0 (ã��, ù ��° ��ġ), VALUE = 0 (Ž�� ����)
 * 3. TEMP = 0, VALUE = 1 + 0 = 1
 * ��� index == 1
 */
template <typename Head, typename... Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T> {
	static constexpr int32 value = [] {
		constexpr int32 temp = IndexOf<TypeList<Tail...>, T>::value;
		return (temp == -1) ? -1 : 1 + temp;
		}();
};
#pragma endregion -------------------------------------------


#pragma region Conversion. Ÿ�� ��ȯ ���ɼ��� Ȯ��
template <typename From, typename To>
struct Conversion
{
	/*
	 * From �� To �� ��ȯ ����(��� ����) ��� exists = true
	 */
	static constexpr bool exists = std::is_convertible_v<From, To>;
};
#pragma endregion -------------------------------------------

#pragma region TypeCast
/*
 * ���� ���� Ÿ������ ��ȯ. Int2Type<N>�� ����ϸ�
 * N �̶�� ���� Ÿ�� ��ü�� ��ȯ. ������ �������� ��� ����
 */
template <int32 V>
struct Int2Type
{
	enum : int8 { VALUE = V };
};

template <typename TL>
class TypeConversion
{
public:
	enum : int8
	{
		// TypeList ����ü�� ���Ե� Ÿ���� ���� ����
		LENGTH = Length(TL{})
	};

	TypeConversion()
	{
		MakeTable(Int2Type<0>(), Int2Type<0>());
	}

	// ��ȯ�� �������� ��� ���̺� ����
	template<int32 I, int32 J>
	static void MakeTable(Int2Type<I>, Int2Type<J>)
	{
		using FromType = typename TypeAt<I, TL>::Result;
		using ToType = typename TypeAt<J, TL>::Result;
		if (Conversion<const FromType*, const ToType*>::exists)
			s_convert[I][J] = true;
		else
			s_convert[I][J] = false;

		MakeTable(Int2Type<I>(), Int2Type<J + 1>());
	}

	// J�� length�� �����ϸ� i�� ������Ű�� �ٽ� J=0 ���� ����
	template<int32 I>
	static void MakeTable(Int2Type<I>, Int2Type<LENGTH>)
	{
		MakeTable(Int2Type<I + 1>(), Int2Type<0>());
	}

	// I�� length�� �����ϸ� ���̺� ���� ����
	template<int32 J>
	static void MakeTable(Int2Type<LENGTH>, Int2Type<J>)
	{	}

	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	static bool s_convert[LENGTH][LENGTH];
};

template <typename TL>
bool TypeConversion<TL>::s_convert[LENGTH][LENGTH];

template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value))
		return static_pointer_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

template<typename To, typename From>
bool CanCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, std::remove_pointer_t<To>>::value);
}

#pragma endregion -------------------------------------------


#define DECLARE_TL			using TL = TL; int32 _typeId
#define INIT_TL(Type)		_typeId = IndexOf<TL, Type>::value







#pragma region Study Length TypeList�� ���̸� ����ϴ� ���ø�
////	* �⺻ ���ø�, ������ ���� �ȵ�
////	* �����Ϸ��� �Ϲ����� Length<T> ���ø��� ã�� ���ϸ� Ư��ȭ�� Length<TypeList<Ts...>>�� ����� �� ����
//template <typename T>
//struct Length;
//
////	* �� TypeList, �⺻ �� 0, TypeList<> �� ������ 0
//template <>
//struct Length<TypeList<>>
//{
//	enum : int8 { VALUE = 0 };
//};
//
////	* �ּ��� �ϳ��� Ÿ�� T�� ���� Ÿ�� ����Ʈ U...
////	* VALUE�� ���� ����(1) + ���� ����Ʈ�� ����(��� ȣ��)
////	*
////	* 1. Length<TypeList<int, float, double>>::VALUE = 1 + Length<TypeList<float, double>>::VALUE
////	* 2. Length<TypeList<float, double>>::VALUE = 1 + Length<TypeList<double>>::VALUE
////	* 3. Length<TypeList<double>>::VALUE = 1 + Length<TypeList<>>::VALUE
////	* 4. Length<TypeList<>>::VALUE = 0
////	* ���̴� 3
//template <typename T, typename... U>
//struct Length<TypeList<T, U...>>
//{
//	enum : int8 { VALUE = 1 + Length <TypeList<U...>>::VALUE };
//};
#pragma endregion

#pragma region Study TypeAt
//template <typename TypeList, int32 Index>
//struct TypeAt;
//
////	* Ư��ȭ ���ø�, index == 0�� �� ����
////	* index == 0, �̸� Head�� ��ȯ
////	* using MyTypes = TypeList<int, float, double>;
////	* using FirstType = TypeAt<MyTypes, 0>::Result; // int
////	*
////	* ó���� �⺻ ���ø�
////	* template <typename TypeList, int32 Index>
////	* struct TypeAt;
////	* �� ã����, ������ ���� ������, Ư��ȭ�� ������ ã�Ƽ� �Ʒ� ���ø� ���
////	* struct TypeAt<TypeList<int, float, double>, 0>
////	* {
////	*		using Result = int;
////	* };
////	* ���� ���� �ڵ尡 �����ǰ� Result�� int
//template <typename Head, typename... Tail>
//struct TypeAt<TypeList<Head, Tail...>, 0>
//{
//	using Result = Head;
//};
//
////	* TypeAt<TypeList<int, float, double>, 1>
////	* Head = int, Tail = TypeList<float, double>, index = 1
////	* Head = float, Tail = TypeList<double>, index = 0, �� ���ø� ���, float ��ȯ
//template <typename Head, typename... Tail, int32 Index>
//struct TypeAt<TypeList<Head, Tail...>, Index>
//{
//	using Result = typename TypeAt<TypeList<Tail...>, Index - 1>::Result;
//};
#pragma endregion

#pragma region Study IndexOf. TypeList���� Ư�� Ÿ�� T�� �ε����� ã�´�
//template <typename TL, typename T>
//struct IndexOf;
//
///*
// * TypeList�� ù ��° Ÿ��(T)�� ã���� �ϴ� Ÿ�� (T)�� ���ٸ� �ε����� 0
// */
//template <typename... Tail, typename T>
//struct IndexOf<TypeList<T, Tail...>, T>
//{
//	enum : int8 { VALUE = 0 };
//};
//
///*
// * TypeList�� ��� ������ T�� �����Ƿ� -1 ��ȯ
// */
//template <typename T>
//struct IndexOf<TypeList<>, T>
//{
//	enum : int8 { VALUE = -1 };
//};
//
///*
// * ��������� Ž��
// * TypeList<Head, Tail...>���� ù ��° Ÿ��(Head)�� T�� �ƴ϶��, Tail...�� ��� �˻�
// * TEMP�� IndexOf<TypeList<Tail...>, T>::VALUE ��
// * ���� TEMP == -1�̸� T�� ����Ʈ�� �����Ƿ� VALUE = -1
// * �׷��� ������ VALUE = 1 + TEMP �� ��, T�� �߰ߵ� ��ġ�� 1�� ���ؼ� ���� �ε����� ��ȯ.
// *
// * using MyList = TypeList<int, float, double>;
// * constexpr int index = IndexOf<MyList, float>::VALUE; // index == 1
// * 1. Head = int, Tail = TypeList<float, double>, TEMP = IndexOf<TypeList<float, double>, float>::VALUE
// * 2. Head = float, Tail = TypeList<double>, TEMP = 0 (ã��, ù ��° ��ġ), VALUE = 0 (Ž�� ����)
// * 3. TEMP = 0, VALUE = 1 + 0 = 1
// * ��� index == 1
// */
//template <typename Head, typename... Tail, typename T>
//struct IndexOf<TypeList<Head, Tail...>, T>
//{
//private :
//	enum : int8 { TEMP = IndexOf<TypeList<Tail...>, T>::VALUE };
//
//public:
//	enum : int8 { VALUE = (TEMP == -1) ? -1 : 1 + TEMP };
//};
#pragma endregion -------------------------------------------

#pragma region Study Conversion. Ÿ�� ��ȯ ���ɼ��� Ȯ��
///*
// * 1. MakeFrom() �� From Ÿ���� ��ü�� ����
// * 2. Test(MakeFrom()) ȣ��:
// *		Test(const To&)�� To Ÿ���� ���� �� �ִٸ� ����ȴ�.
// *		��, From�� To�� ��ӹ޾Ҵٸ�
// *		�ڽ� Ŭ������ ��ü�� �θ� Ŭ������ ������ ���� �� �����ϱ�
// *		Test(const To&) ȣ�� -> Small(int8) ��ȯ
// *		��ӹ��� ���ϸ� (Test...) ȣ�� -> Big(int32) ��ȯ
// */
//template <typename From, typename To>
//class Conversion
//{
//public:
//	using Small = int8;
//	using Big = int32;
//
//	static Small Test(const To&) { return 0; }
//	static Big Test(...) { return 0; }
//	static From MakeFrom() { return 0; }
//
//public:
//	enum
//	{
//		exists = sizeof(Test(MakeFrom())) == sizeof(Small)
//	};
//};
#pragma endregion-------------------------------------------
