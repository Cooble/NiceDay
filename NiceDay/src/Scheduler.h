#pragma once
#include "ndpch.h"

typedef std::function<bool()> Task;//return true of should be killed

class Scheduler
{
private:
	int m_current_task_id = 0;
	struct ScheduleTask
	{
		int tickTarget;
		int ticksFromLastTick;
		Task t;

		ScheduleTask(Task& t, int tickPeriod);
	};
	std::vector<ScheduleTask> m_tasks;
public:
	//will be called each eachTicks ticks until kill() is called
	int runTaskTimer(Task t, int eachTicks = 1);
	void update();
	void kill(int task_id);
};
