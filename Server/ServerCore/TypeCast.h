#pragma once
#include "Types.h"

#pragma region TypeList. 여러 개의 타입을 묶어서 리스트처럼 관리
/*
 * 가변인자를 받는 TypeList에서 빈 구조체를 위해 선언
 */
template <typename... T>
struct TypeList;

/*
 * 두 개의 타입을 묶을 수 있는 리스트.
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
 * 가변 인자를 받아 여러 개의 타입을 받을 수 있는 리스트.
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


#pragma region Length. TypeList의 길이를 계산하는 템플릿
template <typename... TL>
constexpr int32 Length(TypeList<TL...>) {
	return sizeof...(TL);
}
#pragma endregion -------------------------------------------


#pragma region TypeAt. N번째 타입을 추출하는 템플릿
/*
 * tuple_element_t. TupleType에서 Index 번째 타입을 가져오는 std 활용
 */
template <int32 Index, typename... Types>
struct TypeAt
{
	static_assert(Index < sizeof...(Types), "Index out of range");
	using Result = std::tuple_element_t<Index, std::tuple<Types...>>;
};

// TypeList<Types...>을 지원하는 특수화 추가
template <int32 Index, typename... Types>
struct TypeAt<Index, TypeList<Types...>> {
	static_assert(Index < sizeof...(Types), "Index out of range!");
	using Result = std::tuple_element_t<Index, std::tuple<Types...>>;
};
#pragma endregion -------------------------------------------


#pragma region IndexOf. TypeList에서 특정 타입 T의 인덱스를 찾는다
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
 * 재귀적으로 탐색
 * TypeList<Head, Tail...>에서 첫 번째 타입(Head)이 T가 아니라면, Tail...을 계속 검색
 * TEMP는 IndexOf<TypeList<Tail...>, T>::VALUE 값
 * 만약 TEMP == -1이면 T는 리스트에 없으므로 VALUE = -1
 * 그렇지 않으면 VALUE = 1 + TEMP → 즉, T가 발견된 위치에 1을 더해서 실제 인덱스를 반환.
 *
 * using MyList = TypeList<int, float, double>;
 * constexpr int index = IndexOf<MyList, float>::VALUE; // index == 1
 * 
 * 1. Head = int, Tail = TypeList<float, double>, TEMP = IndexOf<TypeList<float, double>, float>::VALUE
 * 2. Head = float, Tail = TypeList<double>, TEMP = 0 (찾음, 첫 번째 위치), VALUE = 0 (탐색 종료)
 * 3. TEMP = 0, VALUE = 1 + 0 = 1
 * 결과 index == 1
 */
template <typename Head, typename... Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T> {
	static constexpr int32 value = [] {
		constexpr int32 temp = IndexOf<TypeList<Tail...>, T>::value;
		return (temp == -1) ? -1 : 1 + temp;
		}();
};
#pragma endregion -------------------------------------------


#pragma region Conversion. 타입 변환 가능성을 확인
template <typename From, typename To>
struct Conversion
{
	/*
	 * From 이 To 로 변환 가능(상속 관계) 라면 exists = true
	 */
	static constexpr bool exists = std::is_convertible_v<From, To>;
};
#pragma endregion -------------------------------------------

#pragma region TypeCast
/*
 * 정수 값을 타입으로 변환. Int2Type<N>을 사용하면
 * N 이라는 값이 타입 자체로 변환. 컴파일 시점에서 사용 가능
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
		// TypeList 구조체에 포함된 타입의 개수 저장
		LENGTH = Length(TL{})
	};

	TypeConversion()
	{
		MakeTable(Int2Type<0>(), Int2Type<0>());
	}

	// 변환이 가능한지 모두 테이블에 저장
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

	// J가 length에 도달하면 i를 증가시키고 다시 J=0 부터 시작
	template<int32 I>
	static void MakeTable(Int2Type<I>, Int2Type<LENGTH>)
	{
		MakeTable(Int2Type<I + 1>(), Int2Type<0>());
	}

	// I가 length에 도달하면 테이블 생성 종료
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







#pragma region Study Length TypeList의 길이를 계산하는 템플릿
////	* 기본 템플릿, 없으면 빌드 안됨
////	* 컴파일러는 일반적인 Length<T> 템플릿을 찾지 못하면 특수화된 Length<TypeList<Ts...>>도 사용할 수 없슴
//template <typename T>
//struct Length;
//
////	* 빈 TypeList, 기본 값 0, TypeList<> 를 넣으면 0
//template <>
//struct Length<TypeList<>>
//{
//	enum : int8 { VALUE = 0 };
//};
//
////	* 최소한 하나의 타입 T와 남은 타입 리스트 U...
////	* VALUE는 현재 원소(1) + 남은 리스트의 길이(재귀 호출)
////	*
////	* 1. Length<TypeList<int, float, double>>::VALUE = 1 + Length<TypeList<float, double>>::VALUE
////	* 2. Length<TypeList<float, double>>::VALUE = 1 + Length<TypeList<double>>::VALUE
////	* 3. Length<TypeList<double>>::VALUE = 1 + Length<TypeList<>>::VALUE
////	* 4. Length<TypeList<>>::VALUE = 0
////	* 길이는 3
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
////	* 특수화 템플릿, index == 0일 때 동작
////	* index == 0, 이면 Head를 반환
////	* using MyTypes = TypeList<int, float, double>;
////	* using FirstType = TypeAt<MyTypes, 0>::Result; // int
////	*
////	* 처음에 기본 템플릿
////	* template <typename TypeList, int32 Index>
////	* struct TypeAt;
////	* 을 찾지만, 구현이 없기 때문에, 특수화된 버전을 찾아서 아래 템플릿 사용
////	* struct TypeAt<TypeList<int, float, double>, 0>
////	* {
////	*		using Result = int;
////	* };
////	* 위와 같이 코드가 생성되고 Result는 int
//template <typename Head, typename... Tail>
//struct TypeAt<TypeList<Head, Tail...>, 0>
//{
//	using Result = Head;
//};
//
////	* TypeAt<TypeList<int, float, double>, 1>
////	* Head = int, Tail = TypeList<float, double>, index = 1
////	* Head = float, Tail = TypeList<double>, index = 0, 위 템플릿 사용, float 반환
//template <typename Head, typename... Tail, int32 Index>
//struct TypeAt<TypeList<Head, Tail...>, Index>
//{
//	using Result = typename TypeAt<TypeList<Tail...>, Index - 1>::Result;
//};
#pragma endregion

#pragma region Study IndexOf. TypeList에서 특정 타입 T의 인덱스를 찾는다
//template <typename TL, typename T>
//struct IndexOf;
//
///*
// * TypeList의 첫 번째 타입(T)이 찾고자 하는 타입 (T)과 같다면 인덱스는 0
// */
//template <typename... Tail, typename T>
//struct IndexOf<TypeList<T, Tail...>, T>
//{
//	enum : int8 { VALUE = 0 };
//};
//
///*
// * TypeList가 비어 있으면 T가 없으므로 -1 반환
// */
//template <typename T>
//struct IndexOf<TypeList<>, T>
//{
//	enum : int8 { VALUE = -1 };
//};
//
///*
// * 재귀적으로 탐색
// * TypeList<Head, Tail...>에서 첫 번째 타입(Head)이 T가 아니라면, Tail...을 계속 검색
// * TEMP는 IndexOf<TypeList<Tail...>, T>::VALUE 값
// * 만약 TEMP == -1이면 T는 리스트에 없으므로 VALUE = -1
// * 그렇지 않으면 VALUE = 1 + TEMP → 즉, T가 발견된 위치에 1을 더해서 실제 인덱스를 반환.
// *
// * using MyList = TypeList<int, float, double>;
// * constexpr int index = IndexOf<MyList, float>::VALUE; // index == 1
// * 1. Head = int, Tail = TypeList<float, double>, TEMP = IndexOf<TypeList<float, double>, float>::VALUE
// * 2. Head = float, Tail = TypeList<double>, TEMP = 0 (찾음, 첫 번째 위치), VALUE = 0 (탐색 종료)
// * 3. TEMP = 0, VALUE = 1 + 0 = 1
// * 결과 index == 1
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

#pragma region Study Conversion. 타입 변환 가능성을 확인
///*
// * 1. MakeFrom() 이 From 타입의 객체를 만듬
// * 2. Test(MakeFrom()) 호출:
// *		Test(const To&)는 To 타입을 받을 수 있다면 실행된다.
// *		즉, From이 To를 상속받았다면
// *		자식 클래스의 객체는 부모 클래스의 참조를 받을 수 있으니까
// *		Test(const To&) 호출 -> Small(int8) 반환
// *		상속받지 못하면 (Test...) 호출 -> Big(int32) 반환
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
