#ifndef __SCHED_RR__
#define __SCHED_RR__

#include <vector>
#include <queue>
#include "basesched.h"

class SchedRR : public SchedBase {

	public:
		SchedRR(std::vector<int> argn);
        ~SchedRR();
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(int cpu, const enum Motivo m);

private:
		int next(int cpu);
		int SiguienteTarea();
		int cantCores;
		std::queue<int> colaDeTareas;
		std::vector<int> quantum;
		std::vector<int> cantTicks;
};

#endif
