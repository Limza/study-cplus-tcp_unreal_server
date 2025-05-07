#include "pch.h"
#include "DeadLockProfiler.h"

using namespace std;

/* --------------------------------------
 *		DeadLockProfiler
 -------------------------------------- */


void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	// 아이디를 찾거나 발급한다
	int32 lockId;
	if (const auto findIt = _nameToId.find(name); findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}

	// 잡고 있는 락이 있었다면
	if (LLockStack.empty() == false)
	{
		// 기존에 발견되지 않은 케이스라면 데드락 여부 확인

		// 직접 구현한 rwLock에서 재귀적으로 lock을 잡는걸 허용 했으니,
		// lockId가 다를 경우에만 확인한다
		if (const int32 prevId = LLockStack.top(); lockId != prevId)
		{
			// 처음 발견한 간선이라면 history 에 추가, cycle을 확인한다
			if (set<int32>& history = _lockHistory[prevId]; history.find(lockId) == history.end())
			{
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard lock(_lock);

	if (LLockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	if (const int32 lockId = _nameToId[name]; LLockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; ++lockId)
		Dfs(lockId);

	// 연산이 끝났으면 정리한다.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(const int32 here)
{
	// 이미 방문을 한 지점인가?
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;

	// 모든 인전한 정점을 순회한다
	const auto findIt = _lockHistory.find(here);

	// history가 생성이 안된건, 락을 잡은 상태에서 다른 락을 잡은적이 없다
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}

	// 여기서 부턴, 어떤 락을 잡은 뒤 다른 락을 잡은 적이 있음

	const set<int32>& nextSet = findIt->second;
	// 연결된 락을 하나씩 확인한다
	for (const int32 there : nextSet)
	{
		// 아직 방문한 적이 없다면 방문한다
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here가 there보다 먼저 발견되었다면, there는 here의 후손이다(순방향 간선)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		// 순방향이 아니고, Dfs(there)가 아직 종료하지 않았다면, there는 here의 선조이다(역방향 간선)
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
