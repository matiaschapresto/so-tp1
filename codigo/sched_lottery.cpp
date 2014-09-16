#include "sched_lottery.h"
#include <cstdlib>

using namespace std;

SchedLottery::SchedLottery(vector<int> argn) : quantum(argn[1]), semilla(argn[2]), cantTicks(0)
{
	
}

bool compare(const pair<int,int>&i, const pair<int,int>&j)
{
	//funcion comparadora para elsort
	return i.first<j.first;
}

SchedLottery::~SchedLottery() 
{

}

void SchedLottery::load(int pid) 
{
	//agregamos a la lista de tareas
  	
  	tareasYTickets.insert(tareasYTickets.begin(),std::pair<int,int>(pid, 0));

  	//Actualizamos la cantidad de tickets de todos

  	redistribuirTickets();
}

void SchedLottery::load(int pid,int deadline) 
{
	//agregamos a la lista de tareas

	tareasYTickets.insert(tareasYTickets.begin(),std::pair<int,int>(pid, 0));

  	//Actualizamos la cantidad de tickets de todos

  	redistribuirTickets();
}

void SchedLottery::unblock(int pid) 
{

	//volvemos a agregar a la lista de tareas

	tareasYTickets.insert(tareasYTickets.begin(),std::pair<int,int>(pid, 0));

	//redistribuimos los tickets y guardamos la cantidad que quedo para cada uno

	int ticketsOriginales = redistribuirTickets();

	//buscamos a ver si necesita recibir tickets compensatorios la tarea

	std::map<int,int>::iterator itBlock = aCompensar.find(pid);

	if(itBlock != aCompensar.end())
	{//si necesita

		//tomamos el multiplicador de los tickets
		int multiplicador = itBlock->second;
		//calculamos cuantos tickets mas se agregaron a la cantidad total de tickets
		int ticketsAgregados = ticketsOriginales*multiplicador - ticketsOriginales;
		//multiplicamos los tickets
		((tareasYTickets.begin())->second) *= multiplicador;
		//avisamos que una tarea fue compensada, para que en el proximo tick sea descompensada
		compensada = true;
		//la borramos del diccionario de tareas a compensar
		aCompensar.erase(itBlock);
		//recalculamos la cantidad de tickets
		cantTickets += ticketsAgregados;
	}
	//ordenamos la lista
	tareasYTickets.sort(compare);
}

int SchedLottery::tick(int cpu, const enum Motivo motivo) 
{

	int tareaActual = current_pid(cpu);

		if (motivo == TICK)
		{
			//Si estoy en la IDLE me la salteo
			if (tareaActual == IDLE_TASK && !tareasYTickets.empty()) 
			{
				return loteria();
			}
			//paso un tick mas
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
			if((motivo == BLOCK) && (cantTicks < quantum))
			{//si se bloqueo y no consumio todo su quantum, agregar al diccionaro de tareas a compensar
				compensar(tareaActual, cantTicks);
			}

			cantTicks = 0;
			//borramos la tarea de la lista de tareas
			std::list<std::pair<int, int> >::iterator it = tareasYTickets.begin();

			while(it != tareasYTickets.end() && it->first != tareaActual)
			{
				it++;
			}

			tareasYTickets.erase(it);
			//si se terminaron volvemos a IDLE
			if(tareasYTickets.empty())
			{
				return IDLE_TASK;
			}
			//como hay una tarea menos se necestan redistribuir los tickets para asegurar probabilidad equitativas
			redistribuirTickets();

			return loteria();
			
		}
}
	

int SchedLottery::loteria()
{//algoritmo explicado en el informe en la seccion loteria
	std::srand(semilla); 

	int ticketGanador = rand() % cantTickets;

	int duenioTicketGanador;

		std::list<std::pair<int,int> >::iterator it = tareasYTickets.begin();

		int sumaParcial = 0;

  		while (it!=tareasYTickets.end() && (sumaParcial < ticketGanador))
  		{
  			sumaParcial += it->second;

  			if(ticketGanador <= sumaParcial)
  			{
  				duenioTicketGanador = it->first;
  					break;
  					  				
  			}

  			++it;
  		}

  		if(compensada)
  		{//si hubo una tarea compensada en el anterior tick, se deben redistribuir los tickets
  			compensada = false;
  			redistribuirTickets();
  		}

  		return duenioTicketGanador;
}


int SchedLottery::redistribuirTickets()
{
	// Dividimos los 100 tickets por la cantidad de Tareas

	cantTickets = 100;

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
  	return ticketsCadaUno;
}

void SchedLottery::compensar(int tarea, int ticks)
{
	//si uso 0 ticks, asumo que uso uno para poder dividir
	if(ticks == 0)
		ticks =1;

	//calculo la fraccion de quantum que uso el proceso
	double fraccionQuantum = (double) ticks / quantum;
	// obtengo el multiplicador de los tickets haciendo 1/fraccion
	int multiplicador = (int) 1/fraccionQuantum;
	//lo inserto en el diccionario de tareas a compensar cuadno se desbloqueen
	aCompensar.insert(std::pair<int,int>(tarea,multiplicador));	
}

