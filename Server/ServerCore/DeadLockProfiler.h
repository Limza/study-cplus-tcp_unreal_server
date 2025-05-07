#pragma once

#include <map>
#include <vector>

/* --------------------------------------
 *		DeadLockProfiler
 -------------------------------------- */
class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(const int32 here);

private:
	std::unordered_map<const char*, int32>	_nameToId; // lock 이름과 id 매핑
	std::unordered_map<int32, const char*>	_idToName; // id와 lock 이름 매핑
	

	// lock 들의 관계, 정점과 간선의 정보를 저장한다
	// 만약 1번 락이 3,5번 락을 바라보고 있다면 (1, {3, 5}) 식으로 저장됨
	std::map<int32, std::set<int32>>		_lockHistory; 

	Mutex _lock;
private:
	std::vector<int32>	_discoveredOrder; // 노드가 발견된 순서를 기록하는 배열
	int32				_discoveredCount = 0; // 노드가 발견된 순서
	std::vector<bool>	_finished; // Dfs(i)가 종료 되었는지 여부
	std::vector<int32>	_parent;
};

