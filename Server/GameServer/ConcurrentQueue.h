#pragma once

#include <mutex>

using namespace std;

template<typename T>
class LockQueue
{
public:
	LockQueue() {}
	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_queue.push(std::move(value));
		_condVar.notify_one();

	}

	bool TryPop(T& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_queue.empty())
			return false;

		value = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_condVar.wait(lock, [this] {return _queue.empty() == false; });
		value = std::move(_queue.front());
		_queue.pop();
	}

private:
	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _condVar;
};


//template<typename T>
//class LockFreeQueue
//{
//	struct Node
//	{
//		shared_ptr<T> data;
//		Node* next = nullptr;
//	};
//
//public:
//	LockFreeQueue() :
//		_head(new Node) // 처음 생성할 때 _head & _tail에 dummy 노드 넣어줌
//		, _tail(_head)
//	{}
//	LockFreeQueue(const LockFreeQueue&) = delete;
//	LockFreeQueue& operator=(const LockFreeQueue&) = delete;
//
//	void Push(const T& value)
//	{
//		shared_ptr<T> newData = make_shared<T>(value);
//
//		Node* dummy = new Node();
//		Node* oldTail = _tail;
//		oldTail->data.swap(newData);
//		oldTail->next = dummy;
//
//		_tail = dummy;
//	}
//
//	shared_ptr<T> TryPop()
//	{
//		Node* oldHead = PopHead();
//		if (oldHead == nullptr)
//			return shared_ptr<T>();
//
//		shared_ptr<T> res(oldHead->data);
//		delete oldHead;
//		return res;
//	}
//
//private:
//	Node* PopHead()
//	{
//		Node* oldHead = _head;
//		if (oldHead == _tail)
//			return nullptr;
//
//		_head = oldHead->next;
//		return oldHead;
//	}
//
//private:
//	// [data][data][data][data]
//	// [head]			 [tail]
//	// 위 처럼 head, tail이 앞과 끝을 바라보는 형식이 아닌
//
//	// [dummy]
//	// [head][tail]
//	// head, tail 이 빈 더미 노드를 가리킨 상태에서 시작한다
//
//	Node* _head = nullptr;
//	Node* _tail = nullptr;
//};

template<typename T>
class LockFreeQueue
{
	struct Node;
	struct CountedNodePtr
	{
		int32 externalCount; // 참조권
		Node* ptr = nullptr;
	};

	struct NodeCounter
	{
		uint32 internalCount : 30; // 참조권 반환 관련
		uint32 externalCountRemaining : 2; // Push & Pop 다중 참조권 관련
	};

	struct Node
	{
		Node()
		{
			NodeCounter newCount;
			newCount.internalCount = 0;
			newCount.externalCountRemaining = 2;
			count.store(newCount);

			next.ptr = nullptr;
			next.externalCount = 0;
		}

		void ReleaseRef()
		{
			// TODO
		}

		atomic<T*> data;
		atomic<NodeCounter> count;
		CountedNodePtr next;
	};

public:
	LockFreeQueue() :
		_head(new Node) // 처음 생성할 때 _head & _tail에 dummy 노드 넣어줌
		, _tail(_head)
	{}
	LockFreeQueue(const LockFreeQueue&) = delete;
	LockFreeQueue& operator=(const LockFreeQueue&) = delete;

	void Push(const T& value)
	{
		unique_ptr<T> newData = make_unique<T>(value);
		CountedNodePtr dummy;
		dummy.ptr = new Node;
		dummy.externalCount = 1;

		CountedNodePtr oldTail = _tail.load();

		while (true)
		{
			// 참조권 획득 (externalCount를 현시점 기준 +1 한 애가 이김)
			IncreaseExternalCount(_tail, oldTail);

			// 소유권 획득 (data를 먼저 교환한 애가 이김)
			T* oldData = nullptr;
			if (oldTail.ptr->data.compare_exchange_strong(oldData, newData.get()))
			{
				oldTail.ptr->next = dummy;
				oldTail = _tail.exchange(dummy);

				newData.release();
				break;
			}
		}

		/*shared_ptr<T> newData = make_shared<T>(value);

		Node* dummy = new Node();
		Node* oldTail = _tail;
		oldTail->data.swap(newData);
		oldTail->next = dummy;

		_tail = dummy;*/
	}

	shared_ptr<T> TryPop()
	{
		/*Node* oldHead = PopHead();
		if (oldHead == nullptr)
			return shared_ptr<T>();

		shared_ptr<T> res(oldHead->data);
		delete oldHead;
		return res;*/
	}

private:
	static void IncreaseExternalCount(atomic<CountedNodePtr>& counter, CountedNodePtr& OldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = OldCounter;
			++newCounter.externalCount;

			if (counter.compare_exchange_strong(OldCounter, newCounter))
			{
				OldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

private:
	// [data][data][data][data]
	// [head]			 [tail]
	// 위 처럼 head, tail이 앞과 끝을 바라보는 형식이 아닌

	// [dummy]
	// [head][tail]
	// head, tail 이 빈 더미 노드를 가리킨 상태에서 시작한다

	atomic<CountedNodePtr> _head;
	atomic<CountedNodePtr> _tail;
};