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

int
main( int argc, char *argv[] )
{
   QCoreApplication *app;

   struct options *o;
   o = parseOptions( argc, argv );
   fprintf(stderr, "threads = %d\n", o->threads);

   //FIXME multithread gui case.
   if( o->use_gui && o->threads == 1)
   {
      //Single-threaded gui.
      app = safeNew( QApplication( argc, argv ) );
      NernstGUI gui( o );
      gui.show();
      return app->exec();
   } else if (o->threads > 1){
      //Multi-threaded console.
	// Create the main GUI thread and a custom event.
	MainThread *app = safeNew( MainThread( argc, argv ) );	
	app->o = o;
	o->s = app->s = safeNew( NernstSim( o ) );
	app->run();
	// Start the thread running.
	// return app->exec();
   }else{
      //Single-threaded non-gui.
      app = safeNew( QCoreApplication( argc, argv ) );
      NernstSim sim( o );
      sim.runSim();
      return 0;
   }
}

/* main.cpp
 *
 */


#define BLR_NTHREADS 4
#define BLR_NROUNDS 4

//===========================================================================
// MainThread
//===========================================================================


void
MainThread::run( ){
	int i;
	nWorkers = o->threads;
	
	s->initNernstSim();
	s->qtime->start();

	// Allocate the arrays for worker threads and messages.

	worker = (class WorkerThread **)
		 malloc( sizeof(class WorkerThread *) * nWorkers );


	inCount[0] = inCount[1] = 0;
	outCount[0]= outCount[1]= 0;
	semaphore[0] = safeNew( QSemaphore(1) );
	semaphore[1] = safeNew( QSemaphore(1) );
	barrier[0]   = safeNew( QSemaphore(0) );
	barrier[1]   = safeNew( QSemaphore(0) );


	for(i=0; i<nWorkers; i++){
		// Create the worker.
		worker[i] = safeNew( WorkerThread( i, this ) );

		// Set indicies.
		worker[i]->start_idx1 = (i * o->x * o->y)/nWorkers;
		worker[i]->end_idx1   = (i * o->x * o->y)/nWorkers + (o->x * o->y)/(2 * nWorkers);
		worker[i]->start_idx2 = (i * o->x * o->y)/nWorkers + (o->x * o->y)/(2 * nWorkers);
		worker[i]->end_idx2   = ( (i+1) * o->x * o->y)/nWorkers; 
			
		// Set up synchronization.
		worker[i]->inCount  = &inCount[0];
		worker[i]->outCount = &outCount[0];
		worker[i]->semaphore  = &semaphore[0];
		worker[i]->barrier    = &barrier[0];
		worker[i]->nWorkers   = nWorkers;
		worker[i]->iters      = o->iters;
		worker[i]->s          = s;

	
		// Begin the thread w/ an event queue.
		worker[i]->start();
	}


	
	for(i=0; i<nWorkers; i++){
		worker[i]->wait();
	}
	s->elapsed += s->qtime->elapsed() / 1000.0;
	s->completeNernstSim();
	exit(0);
	
}


//===========================================================================
// WorkerThread
//===========================================================================

void
WorkerThread::run(){
	volatile int i=0;
	
	for(i=0; i<iters; i++){
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

		if( id == 0 ){ s->moveAtoms_poretransport( id, id ); }
		Barrier();
	}
	

}



void
WorkerThread::Barrier(){
	/*
	 * rendevous
	 *
	 * mutex.wait()
	 * 	count++
	 * mutex.signal()
	 *
	 * if( count==n ){
	 * 	barrier.signal()
	 * }
	 *
	 * barrier.wait()
	 * barrier.signal()
	 *
	 * critical point
	 */

	// Initialized with semaphore released and barrier acquired.
	// inCount = outCount = 0;
	
	// Need a double latch.
	for(int i=0; i<2; i++){
		// Stack up threads at the barrier.aquire until the last
		// one arrives.
		semaphore[i]->acquire();
		inCount[i]++;

		if( inCount[i]==nWorkers ){
			barrier[i]->release();
		}
		semaphore[i]->release();

		// Turnstile to let threads through one at a time.
		barrier[i]->acquire();
		barrier[i]->release();

		// The last thread through should reset the state.
		semaphore[i]->acquire();
		outCount[i]++;

		if( outCount[i]==nWorkers ){
			barrier[i]->acquire();
			inCount[i] = outCount[i] = 0;
		}
		semaphore[i]->release();
	}
}
 


