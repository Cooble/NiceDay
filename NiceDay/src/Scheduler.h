#pragma once
#include "ndpch.h"
#include "Pool.h"

typedef std::function<bool()> Task;//return true of should be killed
typedef std::function<void()> AfterTask;//called when job isDone

class Scheduler
{
private:
	Pool<JobAssignment> m_job_pool;

	struct ScheduleTask
	{
		int tickTarget;
		int ticksFromLastTick;
		Task t;
		AfterTask afterT;
		JobAssignmentP job = nullptr;

		ScheduleTask(Task&& t, int tickPeriod);
		ScheduleTask(AfterTask&& t, JobAssignmentP job);
	};
	std::vector<ScheduleTask> m_tasks;
	std::vector<ScheduleTask> m_new_tasks;
public:
	Scheduler(int jobPoolSize);
	// will be called each eachTicks ticks until task returns true
	void runTaskTimer(Task&& t, int eachTicks = 1);
	// calls afterTask on main thread when job is done
	// deallocates job
	void callWhenDone(AfterTask&& t, JobAssignmentP assignment);
	void update();
	inline int size() { return m_tasks.size()+m_new_tasks.size(); }
	inline JobAssignmentP allocateJob(){return m_job_pool.allocate();}
	inline void deallocateJob(JobAssignmentP job){return m_job_pool.deallocate(job);}
};
