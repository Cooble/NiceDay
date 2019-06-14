#include "ndpch.h"
#include "Scheduler.h"

Scheduler::ScheduleTask::ScheduleTask(Task& t, int tickPeriod):
	tickTarget(tickPeriod),
	ticksFromLastTick(0)
{
	this->t = t;
}

int Scheduler::runTaskTimer(Task t, int eachTicks)
{
	m_tasks.emplace_back(t, eachTicks);
	return m_tasks.size()-1;
}

void Scheduler::update()
{
	std::set<int> toRemove;
	for(int i  =0;i<m_tasks.size();++i)
	{
		auto& task = m_tasks[i];
		++task.ticksFromLastTick;
		if (task.ticksFromLastTick >= task.tickTarget) {
			task.ticksFromLastTick = 0;
			if (task.t())
				toRemove.insert(i);
		}
	}
	for(int i:toRemove)
		m_tasks.erase(m_tasks.begin() + i);
	
}

void Scheduler::kill(int task_id)
{//maybe todo add smart id which will consist of index in array as well as some number to determine if id is valid
	m_tasks.erase(m_tasks.begin() + task_id);
}
