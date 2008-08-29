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
	app->LocalInit(app);
	// Start the thread running.
	return app->exec();
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
MainThread::customEvent(QEvent *e){
	// The interesting work has been moved to the various 
	// subclasses of CustomEvent.
	((class CustomEvent *)e)->work();
}

void
MainThread::LocalInit( class MainThread *app ){
	int i, rpt;
	nWorkers = o->threads;
	
	// Allocate the arrays for worker threads and messages.

	worker = (class WorkerThread **)
		 malloc( sizeof(class WorkerThread *) * nWorkers );


	// Fill in the arrays.

		// Prime the worker message queue.
		//QCoreApplication::postEvent( worker[i], event[i] );


	for(i=0; i<nWorkers; i++){
		// Create the worker.
		worker[i] = safeNew( WorkerThread( i, app ) );

		// Set indicies.
		worker[i]->start_idx1 = (i * o->x * o->y)/nWorkers;
		worker[i]->end_idx1   = (i * o->x * o->y)/nWorkers + (o->x * o->y)/(2 * nWorkers);
		worker[i]->start_idx2 = (i * o->x * o->y)/nWorkers + (o->x * o->y)/(2 * nWorkers);
		worker[i]->end_idx2   = ( (i+1) * o->x * o->y)/nWorkers; 
		/*
		fprintf(stderr, "worker %d covers %u to %u and %u to %u \n",
			i,
			worker[i]->start_idx1,
                        worker[i]->end_idx1,
                        worker[i]->start_idx2,
                        worker[i]->end_idx2
			); 
		*/ 
			

	
		// Begin the thread w/ an event queue.
		worker[i]->start();
	}

	s->initNernstSim();
	s->qtime->start();

	// Start the iterations.
	for(i=0; i<nWorkers; i++){
		QCoreApplication::postEvent( 
			worker[i], 
			safeNew(  WorkEvent( i, app, worker[i], PREP )  ) 
		);
	}



}


//===========================================================================
// WorkerThread
//===========================================================================

void
WorkerThread::customEvent(QEvent *e){
	// The interesting work has been moved to the various 
	// subclasses of CustomEvent.
	((class CustomEvent *)e)->work();
}

void
WorkerThread::run(){
	exec();
}




//===========================================================================
// Several events
//===========================================================================



void
WorkEvent::work(){
	int i;
	static int w = 0;
	static int iters = -1;
	switch(cmd){
		//===============================================================================================================================
		case PREP:{
			if(id == 0){  app->s->moveAtoms_prep(id, id);	}
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, PREP_ACK ) ) );
			break;
		}
		case PREP_ACK:{
			w++;
			if( w == app->nWorkers ){
				w=0;
				for(i=0; i<app->nWorkers; i++){
					QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], STAKE1 ) ) );
				}
			}
			break;
		}

		//===============================================================================================================================
		case STAKE1:{
			if( id == 0 ){ app->s->moveAtoms_stakeclaim( id, id ); }
			//app->s->moveAtoms_stakeclaim( worker->start_idx1, worker->end_idx1 );	
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, STAKE1_ACK ) ) );
			break;
		}
		case STAKE1_ACK:{
			w++;
			if( w == app->nWorkers ){
				w=0;
				for(i=0; i<app->nWorkers; i++){
					QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], STAKE2 ) ) );
				}
			}
			break;
		}

		//===============================================================================================================================
		case STAKE2:{
			//if( id == 0 ){ app->s->moveAtoms_stakeclaim( worker->start_idx2, worker->end_idx2 ); }
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, STAKE2_ACK ) ) );
			break;
		}
		case STAKE2_ACK:{
			w++;
			if( w == app->nWorkers ){
				w=0;
				for(i=0; i<app->nWorkers; i++){
					QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], MOVE1 ) ) );
				}
			}
			break;
		}

		//===============================================================================================================================
		case MOVE1:{
			if( id == 0 ){ app->s->moveAtoms_move( id, id ); }	
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, MOVE1_ACK ) ) );
			break;
		}
		case MOVE1_ACK:{
			w++;
			if( w == app->nWorkers ){
				w=0;
				for(i=0; i<app->nWorkers; i++){
					QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], MOVE2 ) ) );
				}
			}
			break;
		}

		//===============================================================================================================================
		case MOVE2:{
			//if( id == 0 ){ app->s->moveAtoms_move( id, id ); }	
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, MOVE2_ACK ) ) );
			break;
		}
		case MOVE2_ACK:{
			w++;
			if( w == app->nWorkers ){
				w=0;
				for(i=0; i<app->nWorkers; i++){
					QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], TRANSPORT1 ) ) );
				}
			}
			break;
		}

		//===============================================================================================================================
		case TRANSPORT1:{
			if( id == 0 ){ app->s->moveAtoms_poretransport( id, id ); }
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, TRANSPORT1_ACK ) ) );
			break;
		}
		case TRANSPORT1_ACK:{
			w++;
			if( w == app->nWorkers ){
				w=0;
				for(i=0; i<app->nWorkers; i++){
					QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], TRANSPORT2 ) ) );
				}
			}
			break;
		}

		//===============================================================================================================================
		case TRANSPORT2:{
			//if( id == 0 ){ app->s->moveAtoms_poretransport( id, id ); }	
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, TRANSPORT2_ACK ) ) );
			break;
		}
		case TRANSPORT2_ACK:{
			w++;
			if( w == app->nWorkers ){
				if( iters == -1 ){
					iters = app->o->iters;
				}
				iters--; app->s->currentIter++;
				w=0;
				for(i=0; i<app->nWorkers; i++){
					if( iters ){
						QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], PREP ) ) );
					}else{
						QCoreApplication::postEvent( app->worker[i], safeNew( WorkEvent( i, app, app->worker[i], QUIT ) ) );
					}
				}
			}
			break;
		}
		//===============================================================================================================================

		case QUIT:{
			QCoreApplication::postEvent( app, safeNew( WorkEvent( id, app, worker, QUIT_ACK ) ) );
			worker->exit(0);
			break;
		}
		case QUIT_ACK:{
			worker->wait();
			w++;
			if( w == app->nWorkers ){
				app->s->elapsed += app->s->qtime->elapsed() / 1000.0;
				app->s->completeNernstSim();
				app->exit(0);
			}
			break;
		}


		default:{
			fprintf(stderr, "%s::%d bad event %d\n", __FILE__, __LINE__, cmd);
			app->exit(EXIT_FAILURE);
		}
	}
}


//===========================================================================
// CustomEvent
//===========================================================================

void
CustomEvent::work(){
	fprintf(stderr, "Why is CustomEvent::work() getting called?\n");
}

