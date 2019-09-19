#include "ndpch.h"
#include "Scheduler.h"

Scheduler::ScheduleTask::ScheduleTask(Task& t, int tickPeriod):
	tickTarget(tickPeriod),
	ticksFromLastTick(0)
{
	this->t = t;
}

int Scheduler::runTaskTimer(Task&& t, int eachTicks)
{
	m_tasks.emplace_back(std::forward<Task>(t), eachTicks);
	return m_tasks.size()-1;
}

void Scheduler::update()
{
	std::vector<int> toRemove;
	int size = m_tasks.size();
	for(int i  =0;i< size;++i)
	{
		auto& task = m_tasks[i];
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

void Scheduler::kill(int task_id)
{//maybe todo add smart id which will consist of index in array as well as some number to determine if id is valid
	//is not working
	m_tasks.erase(m_tasks.begin() + task_id);
}
