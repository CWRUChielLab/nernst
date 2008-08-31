/* main.h
 *
 */
#include <QCoreApplication>
#include <QThread>
#include <QEvent>
#include <QSemaphore>
#include <QTime>
#include <QSemaphore>
class WorkerThread;
class MainThread;
class CustomEvent;
class NernstSim;


	


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
		
		// This is a pure virtual function that we have to override.
		// I think all it needs to do is call exec.
		virtual void run();

		unsigned int start_idx1, start_idx2, end_idx1, end_idx2;

		static int inCount[2];
		static int outCount[2];
		static QSemaphore *semaphore[2];
		static QSemaphore *barrier[2];

		static NernstSim *s;
		static struct options *o;
	private:
		void Barrier(void);

};

