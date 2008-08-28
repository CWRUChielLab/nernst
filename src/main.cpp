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
      //Multi-threaded non-gui.
	// Create the main GUI thread and a custom event.
	MainThread *app = safeNew( MainThread( argc, argv ) );	
	app->o = o;
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

        fprintf(stderr, "threads = %d\n", o->threads);
	nWorkers = o->threads;
	
	// Allocate the arrays for worker threads and messages.

	worker = (class WorkerThread **)
		 malloc( sizeof(class WorkerThread *) * nWorkers );

	event = (class WorkEvent **)
		malloc( sizeof(class WorkEvent *) * nWorkers );

	// Fill in the arrays.

	for(i=0; i<nWorkers; i++){
		worker[i] = safeNew( WorkerThread( i, app ) );
		event[i]  = safeNew( WorkEvent( i, app, worker[i] ) );

		// Prime the worker message queue.
		
		QCoreApplication::postEvent( worker[i], event[i] );

		// Begin the thread w/ an event queue.

		worker[i]->start();
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
	WorkAckEvent *reply = safeNew( WorkAckEvent( id, app, worker ) );
	fprintf(stdout, "Worker %d received msg.\n", id);
	QCoreApplication::postEvent( app, reply );
}

void
WorkAckEvent::work(){
	fprintf(stdout, "Main received notice from worker %d.\n", id);
	QuitEvent *event = safeNew( QuitEvent( id, app, worker ) );
	QCoreApplication::postEvent( worker, event );

}

void
QuitEvent::work(){
	fprintf(stdout, "Worker %d quitting.\n", id);
	QuitAckEvent *event = safeNew( QuitAckEvent( id, app, worker ) );
	QCoreApplication::postEvent( worker, event );

	worker->exit(0);

}

void
QuitAckEvent::work(){
	worker->wait();
	fprintf(stdout, "Main received notice worker %d is finished.\n", id);
	app->nWorkers--;
	if(! app->nWorkers ){
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

