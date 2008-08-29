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
	app->s = safeNew( NernstSim( o ) );
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
	int i;

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
		// Begin the thread w/ an event queue.
		worker[i]->start();
	}

	s->initNernstSim();
	s->qtime->start();

	// Start the iterations.
	QCoreApplication::postEvent( 
		worker[0], 
		safeNew(  initIterationEvent( 0, app, worker[0] )  ) 
	);



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

/* This is just a little baroque.  Here's what happens.
 *
 * MainThread::LocalInit performs all simulation initialization and ends by
 * sending a initIteration event to thread0.
 *
 * Thread0 performs per-iteration initialization and sends a initIterationAck
 * event to MainThread.
 *
 * MainThread sends a runIteration event to all child threads.  
 *
 * All child threads will send a runIterationAck event to MainThread.
 *
 * When MainThread has received all of the Acks, it will send a finalizeIteration
 * event to thread0.
 *
 * When thread0 has finished finalization, it will send a finalizeIterationAck
 * event to MainThread.
 *
 * Mainthread will decrement the iteration count and, if zero, perform 
 * simulation finalization and send Quit events to all threads.
 *
 * Threads will post an ack and then call exit.
 *
 * Mainthread, after collecting all acks, will exit.  
 */

// Received by worker0.
void 
moveAtoms_prep__Event::work(){
	// Worker0 performs PREP.
	app->s->moveAtoms_prep();
	QCoreApplication::postEvent( safeNew( moveAtoms_prep__EventAck( id, app, worker ) ) );
}

// Received by MainThread.
void
moveAtoms_prep__EventAck::work(){
	int i;
	// Have all workers start on STAKECLAIM.
	for(i=0; i<app->nWorkers; i++){
		QCoreApplication::postEvent( safeNew( moveAtoms_stakeclaim__Event( i, app, app->worker[i] ) ) );
	}
}

// Received by all workers.
void
moveAtoms_stakeclaim__Event::work(){
	app->s->moveAtoms_stakeclaim( id, id );	//FIXME start_idx, end_idx
	QCoreApplication::postEvent( safeNew( moveAtoms_stakeclaim__EventAck( id, app, worker ) ) );
}

// Received by MainThread.
void
moveAtoms_stakeclaim__EventAck::work(){
	int i;
	static int w = 0;
	w++;
	if(w == app->nWorkers){
		w = 0;
		for(i=0; i<app->nWorkers; i++){
			QCoreApplication::postEvent( safeNew( moveAtoms_move__Event( i, app, app->worker[i] ) ) );
		}
	}
}

// Received by all workers.
void
moveAtoms_move__Event::work(){
	app->s->moveAtoms_move( id, id );	//FIXME start_idx, end_idx
	QCoreApplication::postEvent( safeNew( moveAtoms_move__EventAck( id, app, worker ) ) );
}

// Received by MainThread.
void
moveAtoms_move__EventAck::work(){
	int i;
	static int w = 0;
	w++;
	if(w == app->nWorkers){
		w = 0;
		for(i=0; i<app->nWorkers; i++){
			QCoreApplication::postEvent( safeNew( moveAtoms_poretransport__Event( i, app, app->worker[i] ) ) );
		}
	}
}


// Received by all workers.
void
moveAtoms_poretransport__Event::work(){
	app->s->moveAtoms_poretransport( id, id );	//FIXME start_idx, end_idx
	QCoreApplication::postEvent( safeNew( moveAtoms_poretransport__EventAck( id, app, worker ) ) );
}


// Received by MainThread.
void
moveAtoms_poretransport__EventAck::work(){
	int i;
	static int w = 0;
	w++;
	if(w == app->nWorkers){
		w = 0;
		if( --(app->o->iters) ){
			QCoreApplication::postEvent( safeNew( moveAtoms_prep__Event( 0, app, app->worker[0] ) ) );
		}else{
			for(i=0; i<app->nWorkers; i++){
				QCoreApplication::postEvent( safeNew( QuitEvent( i, app, app->worker[i] ) ) );
			}
		}
	}
}

// Received by all workers.
void
QuitEvent::work(){
	fprintf(stdout, "Worker %d quitting.\n", id);
	QuitAckEvent *event = safeNew( QuitAckEvent( id, app, worker ) );
	QCoreApplication::postEvent( worker, event );

	worker->exit(0);

}

// Recieved by MainThread.
void
QuitAckEvent::work(){
	worker->wait();
	fprintf(stdout, "Main received notice worker %d is finished.\n", id);
	app->nWorkers--;
	if(! app->nWorkers ){
		app->s->elapsed += qtime->elapsed() / 1000.0;
		app->s->completeNernstSim();
		app->exit(0);
	}
}











//===========================================================================
// CustomEvent
//===========================================================================

void
CustomEvent::work(){
	fprintf(stdout, "Why is CustomEvent::work() getting called?\n");
}

