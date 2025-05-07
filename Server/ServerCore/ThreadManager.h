#pragma once

#include <thread>
#include <functional>

/* -----------------------------------------------------
 *	ThreadManager
 -----------------------------------------------------*/
class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(const std::function<void(void)>& callback);
	void Join();

	static void InitTls();
	static void DestroyTls();

	static void DoGlobalQueueWork();
	static void DistributeReservedJobs();

private:
	Mutex _lock;
	std::vector<std::thread> _threads;
};

