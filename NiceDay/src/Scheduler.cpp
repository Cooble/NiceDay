#include "ndpch.h"
#include "Scheduler.h"

Scheduler::ScheduleTask::ScheduleTask(Task&& t, int tickPeriod):
	tickTarget(tickPeriod),
	ticksFromLastTick(0)
{
	this->t = std::forward<Task>(t);
}

Scheduler::ScheduleTask::ScheduleTask(AfterTask&& t, JobAssignmentP job):
job(job)
{	
	this->afterT = std::forward<AfterTask>(t);
}

Scheduler::Scheduler(int jobPoolSize)
	:m_job_pool(jobPoolSize)
{
}

void Scheduler::runTaskTimer(Task&& t, int eachTicks)
{
	m_new_tasks.emplace_back(std::forward<Task>(t), eachTicks);
}

void Scheduler::callWhenDone(AfterTask&& t, JobAssignmentP assignment)
{
	ASSERT(assignment != nullptr,"Assignment cannot be nullptr");
	m_new_tasks.emplace_back(std::forward<AfterTask>(t), assignment);
}

void Scheduler::update()
{
	for (auto& m_new_task : m_new_tasks)
		m_tasks.push_back(std::move((m_new_task)));

	m_new_tasks.clear();

	std::vector<int> toRemove;
	for(int i  =0; i< m_tasks.size();++i)
	{
		auto& task = m_tasks[i];
		if(task.job)
		{
			if (task.job->isDone()) {
				task.afterT();
				toRemove.push_back(i);
				deallocateJob(task.job);
			}
			continue;
		}
		++task.ticksFromLastTick;
		if (task.ticksFromLastTick >= task.tickTarget) {
			task.ticksFromLastTick = 0;
			if (task.t())
				toRemove.push_back(i);
		}
	}
	
	for (int j = toRemove.size() - 1; j >= 0; --j)
		m_tasks.erase(m_tasks.begin() + toRemove[j]);
	
}
