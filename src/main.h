/* main.h
 *
 */
#include <QCoreApplication>
#include <QThread>
#include <QEvent>
#include <QSemaphore>
#include <QTime>

class WorkerThread;
class MainThread;
class CustomEvent;
class NernstSim;

// Kinds of events.
enum{
	NOOP,
	PREP,
	PREP_ACK,
	STAKE1,
	STAKE1_ACK,
	STAKE2,
	STAKE2_ACK,
	MOVE1,
	MOVE1_ACK,
	MOVE2,
	MOVE2_ACK,
	TRANSPORT1,
	TRANSPORT1_ACK,
	TRANSPORT2,
	TRANSPORT2_ACK,
	QUIT,
	QUIT_ACK,
	NUM_CMDS
};

//===========================================================================
// Parent events
//===========================================================================
class CustomEvent : public QEvent {
	public:
		CustomEvent():QEvent(QEvent::User){}
		virtual void work();
};	

//===========================================================================
// Events sent by main, received by workers.  Be sure to include app and thread ptrs.
//===========================================================================
class WorkEvent : public CustomEvent {
	public:
		int id;
		class MainThread *app;
		class WorkerThread *worker;
		int cmd;
		WorkEvent( int param_id, MainThread *param_app, WorkerThread *param_worker, int param_cmd ) :
			CustomEvent(), id( param_id ), app(param_app), worker(param_worker), cmd(param_cmd){}
		virtual void work();
};
/*
class QuitEvent : public CustomEvent {
	public:
		int id;
		class MainThread *app;
		class WorkerThread *worker;
		QuitEvent( int param_id, MainThread *param_app, WorkerThread *param_worker ) :
			CustomEvent(), id( param_id ), app(param_app), worker(param_worker){}
		virtual void work();
};
*/
//===========================================================================
// Events sent by workers, received by main.
//===========================================================================
/*
class WorkAckEvent : public CustomEvent {
	public:
		int id;
		class MainThread *app;
		class WorkerThread *worker;
		WorkAckEvent( int param_id, MainThread *param_app, WorkerThread *param_worker ) :
			CustomEvent(), id( param_id ), app(param_app), worker(param_worker){}
		virtual void work();
};
		
class QuitAckEvent : public CustomEvent {
	public:
		int id;
		class MainThread *app;
		class WorkerThread *worker;
		QuitAckEvent( int param_id, MainThread *param_app, WorkerThread *param_worker ) :
			CustomEvent(), id( param_id ), app(param_app), worker(param_worker){}
		virtual void work();
};
*/


class MainThread : public QCoreApplication {
	public:
		//-----------------------------------------------------------
		// Functions
		//-----------------------------------------------------------
		
		// Subclass QCoreApplication so we can override the
		// event processing.
		MainThread( int param_argc, char **param_argv ) :
			QCoreApplication( param_argc, param_argv ), 
			argc(param_argc), argv(param_argv){}
		
		// Override the custom event queue so we can grab our own 
		// events.
		void customEvent( QEvent *e );

		// Initialize here rather in the constructor.  Makes life 
		// just a tiny bit easier.
		void LocalInit( class MainThread *app );

		//-----------------------------------------------------------
		// Variables
		//-----------------------------------------------------------

		int argc;
		char **argv;
		struct options *o;
		NernstSim *s;
		
		// Assume that number of workers will be fetched off of the 
		// commmand line.
		int nWorkers;

		// Our worker threads.
		class WorkerThread **worker;

	private:

	
};


class WorkerThread : public QThread {
	public:
		// What this ought to do is:
		// 1.  Provide the only constructor for class WorkerThread.
		// 2.  Set param_parent to 0 if it is not provided.
		// 3.  Pass param_parent along to the parent class (QThread).
		// 4.  Set this->id = param_id.
		int id;
		WorkerThread(int param_id, QObject *param_parent=0) : 
			QThread( param_parent ), id( param_id ){}
		
		// Override the custom event queue so we can grab our own events.
		void customEvent( QEvent *e );

		// This is a pure virtual function that we have to override.
		// I think all it needs to do is call exec.
		virtual void run();
};

