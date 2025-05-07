#pragma once

/* ------------------------------------------------------------------------
 * GlobalQueue
 ------------------------------------------------------------------------ */
class GlobalQueue
{
public:
	void		Push(JobQueueRef jobQueue);
	JobQueueRef Pop();

private:
	LockQueue<JobQueueRef> _jobQueues;
};

