#include "sched_lottery_base.h"
#include <cstdlib>

using namespace std;

SchedLotteryBase::SchedLotteryBase(vector<int> argn) : quantum(argn[1]), semilla(argn[2]), cantTicks(0)
{
	std::srand(semilla);
}

SchedLotteryBase::~SchedLotteryBase()
{

}

void SchedLotteryBase::load(int pid)
{
  	
  	tareasYTickets.insert(tareasYTickets.begin(),std::pair<int,int>(pid, 0));

  	//Actualizamos la cantidad de tickets de todos

  	redistribuirTickets();
}

void SchedLotteryBase::load(int pid,int deadline)
{
	tareasYTickets.insert(tareasYTickets.begin(),std::pair<int,int>(pid, 0));

  	//Actualizamos la cantidad de tickets de todos

  	redistribuirTickets();
}

void SchedLotteryBase::unblock(int pid)
{
	tareasYTickets.insert(tareasYTickets.begin(),std::pair<int,int>(pid, 0));

	redistribuirTickets();
}

int SchedLotteryBase::tick(int cpu, const enum Motivo motivo)
{

	int tareaActual = current_pid(cpu);

		if (motivo == TICK)
		{
			//Si estoy en la IDLE me la salteo
			if (tareaActual == IDLE_TASK && !tareasYTickets.empty()) 
			{
				return loteria();
			}

			cantTicks++;
			
			if (cantTicks == quantum)
			{//si se cumple el quantum se resetean la cuenta de ticks y elijo otra tarea	
				cantTicks = 0;
				return loteria();
			}
			// si no me pase del quantum sigue la actual
			return tareaActual;
		}
		else if (motivo == EXIT || motivo == BLOCK)
		{
			cantTicks = 0;

			std::list<std::pair<int, int> >::iterator it = tareasYTickets.begin();

			while(it->first != tareaActual)
			{
				it++;
			}

			tareasYTickets.erase(it);

			if(tareasYTickets.empty())
			{
				return IDLE_TASK;
			}

			redistribuirTickets();

			return loteria();
			
		}
}
	

int SchedLotteryBase::loteria()
{
	int ticketGanador = rand() % 100;

	int duenioTicketGanador;

		std::list<std::pair<int,int> >::iterator it = tareasYTickets.begin();

		if(ticketGanador == 0)
		{
			return tareasYTickets.begin()->first;
		}

		int sumaParcial = 0;

  		while (it!=tareasYTickets.end() && (sumaParcial < ticketGanador))
  		{
  			sumaParcial += it->second;

  			if(ticketGanador <= sumaParcial)
  			{
  				duenioTicketGanador = it->first;
  				
  					return duenioTicketGanador;
  				
  			}

  			++it;
  		}
}


// FUNCION REDISTRIBUIR TICKETS!

void SchedLotteryBase::redistribuirTickets()
{
	// Dividimos los 100 tickets por la cantidad de Tareas

	int cantTareas = tareasYTickets.size();

  	int ticketsCadaUno = 100 / cantTareas;

  	int resto = 100 % cantTareas; // Al resto lo repartimos entre todas

  	
  	std::list<std::pair<int,int> >::iterator it =tareasYTickets.begin();

  	for (; it!=tareasYTickets.end(); ++it)
  	{
  		it->second =ticketsCadaUno;

  		//reparto el resto, un ticket para cada uno hasta que se termine

  		if(resto>0)
  		{
  			it->second++;
  			resto--;
  		}

  	}

}