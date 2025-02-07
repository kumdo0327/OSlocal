#include "cpu_scheduler.h"


void CpuScheduler::StepForward()
{

	if (!isRunning) {

		if (!pQ)
			return;

		if (pQ->Empty() && wQ.Empty()) {

			pQ = nullptr;
			return;
		}
		wQ.Clear();
		ganttChart.clear();
		currentProcess.SetPid("");
		currentProcess.SetBurstTime(0);
		time = 0;
		isRunning = true;
	}


	// Dispatch process from waitingQueue to CPU
	if (!wQ.Empty() || currentProcess.GetBurstTime() > 0) {

		// Preemptive
		if (isPreemptive) {

			// Set currentProcess
			if (currentProcess.GetBurstTime() <= 0) {

				currentProcess = wQ.Top();
				wQ.Pop();
			}
			else if (wQ.HasMorePriority(wQ.Top(), currentProcess)) {

				Process prevProcess = currentProcess;
				currentProcess = wQ.Top();
				wQ.Pop();
				wQ.Push(prevProcess);
			}

			// Increase time
			if (!pQ->Empty() && currentProcess.GetBurstTime() > pQ->Top().GetArrivalTime() - time) {

				double oldTime = time;
				time = pQ->Top().GetArrivalTime();
				currentProcess -= time - oldTime;
			}
			else {

				time += currentProcess.GetBurstTime();
				currentProcess.SetBurstTime(0);
			}
		}

		// Round-Robin
		else if (isRoundRobin) {

			if (currentProcess.GetBurstTime() > 0)
				wQ.Push(currentProcess);
			currentProcess = wQ.Top();
			wQ.Pop();

			if (currentProcess.GetBurstTime() > timeQuantum) {

				currentProcess -= timeQuantum;
				time += timeQuantum;
			}
			else {

				time += currentProcess.GetBurstTime();
				currentProcess.SetBurstTime(0);
			}
		}

		// Non-preemptive and no time-quantum
		else {

			currentProcess = wQ.Top();
			time += currentProcess.GetBurstTime();
			currentProcess.SetBurstTime(0);
			wQ.Pop();
		}
	}
	// If waitingQueue is empty, empty process
	else {

		currentProcess.SetPid("");
		currentProcess.SetBurstTime(0);
		time = pQ->Top().GetArrivalTime();
	}

	
	// Process queue to waiting queue
	while (!pQ->Empty() && time >= pQ->Top().GetArrivalTime()) {

		wQ.Push(pQ->Top());
		pQ->Pop();
	}

	
	// push to Gantt chart
	if (ganttChart.empty())
		ganttChart.emplace_back(currentProcess.GetPid(), time);
	else {

		if (ganttChart.rbegin()->first == currentProcess.GetPid())
			ganttChart.rbegin()->second = time;
		else
			ganttChart.emplace_back(currentProcess.GetPid(), time);
	}


	if (pQ->Empty() && wQ.Empty() && currentProcess.GetBurstTime() <= 0) {

		isRunning = false;
		pQ = nullptr;
	}
}

void CpuScheduler::Reset()
{
	isRunning = false;
	pQ = nullptr;
	wQ.Clear();
	ganttChart.clear();
}
