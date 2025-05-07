#pragma once

#include <mutex>

using namespace std;

template<typename T>
class LockStack
{
public:
	LockStack() {}

	// Delete copy constructor and copy assignment operator
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	// Delete move constructor and move assignment operator
	LockStack(LockStack&&) = delete;
	LockStack& operator=(LockStack&&) = delete;

	// Destructor
	~LockStack() = default;

	void Push(T value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();

	}

	bool TryPop(T& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_condVar.wait(lock, [this] {return _stack.empty() == false; });
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	std::stack<T> _stack;
	std::mutex _mutex;
	std::condition_variable _condVar;
};

//template<typename T>
//class LockFreeStack
//{
//	struct Node
//	{
//		Node(const T& value) : data(make_shared<T>(value)), next(nullptr) {}
//
//		shared_ptr<T> data;
//		shared_ptr<Node> next;
//	};
//
//public:
//	LockFreeStack() {}
//	LockFreeStack(const LockFreeStack&) = delete;
//	LockFreeStack& operator=(const LockFreeStack&) = delete;
//	LockFreeStack(LockFreeStack&&) = delete;
//	LockFreeStack& operator=(LockFreeStack&&) = delete;
//	~LockFreeStack() = default;
//
//	void Push(const T& value)
//	{
//		shared_ptr<Node> node = make_shared<Node>(value);
//		node->next = atomic_load(&_head);
//
//		while (atomic_compare_exchange_weak(&_head, &node->next, node) == false)
//		{
//		}
//	}
//
//	shared_ptr<T> TryPop()
//	{
//		shared_ptr<Node> oldHead = atomic_load(&_head);
//
//		while (oldHead && atomic_compare_exchange_weak(&_head, &oldHead, oldHead->next) == false)
//		{
//		}
//
//		if (oldHead == nullptr)
//			return shared_ptr<T>();
//
//		return oldHead->data;
//	}
//
//private:
//	shared_ptr<Node> _head;
//};

template<typename T>
class LockFreeStack
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value)) {}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	LockFreeStack() {}
	LockFreeStack(const LockFreeStack&) = delete;
	LockFreeStack& operator=(const LockFreeStack&) = delete;
	LockFreeStack(LockFreeStack&&) = delete;
	LockFreeStack& operator=(LockFreeStack&&) = delete;
	~LockFreeStack() = default;

	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{
		}
	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = _head;
		while (true)
		{
			// 참조권 획득 (externalCount를 현 시점 기준 +1 한 애가 이김)
			IncreaseHeadCount(oldHead);

			// 최소한 externalCount >= 2 일테니 삭제 x (안전하게 접근할 수 있는)
			Node* ptr = oldHead.ptr;

			if (ptr == nullptr)
				return shared_ptr<T>();

			// 소유권 획득 (ptr->next로 head를 바꿔치기 한 애가 이김)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);

				// external : 1, 참조권
				//	만약에 나만 이걸 사용한다고 하면 1 증가시켜서 2가 됨
				//  만약에 나 이외에 남이 2명 더 들어왔다면 4가 됨

				// internal : 0

				// 나 말고 또 누가 있는가?. (삭제를 하기 위해)
				// 나만 사용하고 있다면 countIncrease 는 0이 될것
				const int32 countIncrease = oldHead.externalCount - 2;
				
				// 만약 countIncrease 가 2, 나 말고 다른 쓰레드가 접근했다면
				// fetch_add는 이전값 0을 내밷고, 2를 저장한다, 그렇게 되면 아래 조건에
				// 적용되지 않기 때문에 삭제하지 않음
				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease)
				{
					delete ptr;
				}

				return res;
			}
			// 위에서 올린 internalCount를 하나씩 줄이고, 0 이되게 만드는
			// 쓰레드가 삭제를 담당한다
			else if (ptr->internalCount.fetch_sub(1) == 1)
			{
				// 참조권은 얻었으나, 소유권은 실패 -> 뒷수습은 내가 한다
				delete ptr;
			}
		}
	}

private:
	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			++newCounter.externalCount;

			if (_head.compare_exchange_strong(oldCounter, newCounter))
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

private:
	atomic<CountedNodePtr> _head;
};