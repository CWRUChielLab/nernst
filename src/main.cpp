/* main.cpp
 *
 *
 * Copyright (c) 2008, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
 *    Catherine Kehl, Jocelyn Eckert, and Dr. Hillel J. Chiel
 *
 * This file is part of Nernst Potential Simulator.
 * 
 * Nernst Potential Simulator is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * Nernst Potential Simulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nernst Potential Simulator.  If not, see
 * <http://www.gnu.org/licenses/>.
 */


#include <QApplication>
#include <stdlib.h>
#include <malloc.h>
#include "main.h"
#include "options.h"
#include "sim.h"
#include "gui.h"
#include "safecalls.h"
using namespace SafeCalls;


int WorkerThread::inCount[2];
int WorkerThread::outCount[2];
QSemaphore* WorkerThread::semaphore[2];
QSemaphore* WorkerThread::barrier[2];
NernstSim*  WorkerThread::s;
struct options* WorkerThread::o;

int
main( int argc, char *argv[] )
{
	QCoreApplication *app;
	int i;
	class WorkerThread **worker = NULL;
	class NernstSim *s = NULL;
	struct options *o;
	int nWorkers=0;
	o = parseOptions( argc, argv );
	nWorkers = o->threads;

	if( o->use_gui && o->threads == 1) {
	//Single-threaded gui.
		app = safeNew( QApplication( argc, argv ) );
		NernstGUI gui( o );
		gui.show();
		return app->exec();

	} else if (o->threads > 1){
	//Multi-threaded console.

		// Make room for workers.
		worker = (class WorkerThread **)
			 malloc( sizeof(class WorkerThread *) * nWorkers );

		// Create the workers.
		for(i=0; i<nWorkers; i++){
			worker[i] = safeNew( WorkerThread( i, NULL ) );
		}

		// Populate the static variables.
		worker[0]->o = o;
		worker[0]->s = s        = safeNew( NernstSim( o ) );
		worker[0]->inCount[0]   = worker[0]->inCount[1] = 0;
		worker[0]->outCount[0]  = worker[0]->outCount[1]= 0;
		worker[0]->semaphore[0] = safeNew( QSemaphore(1) );
		worker[0]->semaphore[1] = safeNew( QSemaphore(1) );
		worker[0]->barrier[0]   = safeNew( QSemaphore(0) );
		worker[0]->barrier[1]   = safeNew( QSemaphore(0) );

		// Initialization.
		s->initNernstSim();
		s->qtime->start();

		// Start the workers.
		for(i=0; i<nWorkers; i++){

			// Set indicies.
			worker[i]->start_idx1 = (i * o->x * o->y)/nWorkers;
			worker[i]->end_idx1   = (i * o->x * o->y)/nWorkers + (o->x * o->y)/(2 * nWorkers);
			worker[i]->start_idx2 = (i * o->x * o->y)/nWorkers + (o->x * o->y)/(2 * nWorkers);
			worker[i]->end_idx2   = ( (i+1) * o->x * o->y)/nWorkers; 

			// Begin the thread w/ an event queue.
			worker[i]->start();
		}

		// Wait for the workers to finish.
		for(i=0; i<nWorkers; i++){
			worker[i]->wait();
		}
		
		// Cleanup.
		s->elapsed += s->qtime->elapsed() / 1000.0;
		s->completeNernstSim();
		return 0;

	}else{
	//Single-threaded console.
		app = safeNew( QCoreApplication( argc, argv ) );
		NernstSim sim( o );
		sim.runSim();
		return 0;
	}
}


//===========================================================================
// WorkerThread
//===========================================================================

void
WorkerThread::run(){
	int i=0;

	for(i=0; i<o->iters; i++){
		if(id == 0){  s->moveAtoms_prep(id, id);	}
		Barrier();

		s->moveAtoms_stakeclaim( start_idx1, end_idx1 );	
		Barrier();
	
		s->moveAtoms_stakeclaim( start_idx2, end_idx2 ); 
		Barrier();

		s->moveAtoms_move( start_idx1, end_idx1 ); 
		Barrier();

		s->moveAtoms_move( start_idx2, end_idx2); 
		Barrier();

		if( id == 0 ){ 
			s->moveAtoms_poretransport( id, id ); 
			s->currentIter++;
		}
		Barrier();
	}
	

}



void
WorkerThread::Barrier(){

	// Initialized with semaphore released and barrier acquired.
	// inCount = outCount = 0;
	
	// Need a double latch.
	for(int i=0; i<2; i++){
		// Stack up threads at the barrier.aquire until the last
		// one arrives.
		semaphore[i]->acquire();
		inCount[i]++;

		if( inCount[i]==o->threads ){
			barrier[i]->release();
		}
		semaphore[i]->release();

		// Turnstile to let threads through one at a time.
		barrier[i]->acquire();
		barrier[i]->release();

		// The last thread through should reset the state.
		semaphore[i]->acquire();
		outCount[i]++;

		if( outCount[i]==o->threads ){
			barrier[i]->acquire();
			inCount[i] = outCount[i] = 0;
		}
		semaphore[i]->release();
	}
}
 


