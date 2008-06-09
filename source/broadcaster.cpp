/*
This file is part of "Rigs of Rods Server" (Relay mode)
Copyright 2007 Pierre-Michel Ricordel
Contact: pricorde@rigsofrods.com
"Rigs of Rods Server" is distributed under the terms of the GNU General Public License.

"Rigs of Rods Server" is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

"Rigs of Rods Server" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "broadcaster.h"
#include "logger.h"
#include "SocketW.h"

void *s_brthreadstart(void* vid)
{
    STACKLOG;
	((Broadcaster*)vid)->threadstart();
}
Broadcaster::Broadcaster()
:   id( 0 ), sock( NULL ), running( false )
{
    STACKLOG;
}

Broadcaster::~Broadcaster()
{
    STACKLOG;
}

void Broadcaster::reset(int uid, SWInetSocket *socky,
		void (*disconnect_func)(int, char*),
		int (*sendmessage_func)(SWInetSocket*, int, unsigned int,
				unsigned int, char*) )
{
    STACKLOG;
	if( running )
	{
		Logger::log(LOG_ERROR,"Whoa, broadcaster is still alive!");
		return;
	}
	
	id          = uid;
	sock        = socky;
	running     = true;
	disconnect  = disconnect_func;
	sendmessage = sendmessage_func;
	while( !msg_queue.empty() )
		msg_queue.pop();

	// we've got a new client, release the signal
	//start a listener thread
	pthread_create(&thread, NULL, s_brthreadstart, this);
}

void Broadcaster::stop()
{
    STACKLOG;

    queue_mutex.lock();
	running = false;
	queue_cv.signal();
	queue_mutex.unlock();
	pthread_join( thread, NULL );
}

void Broadcaster::threadstart()
{
    STACKLOG;
	queue_entry_t msg;
	Logger::log( LOG_DEBUG, "broadcaster thread %d owned by uid %d", ThreadID::getID(), id);
	while( running )
	{
		{   // define a new scope and use a scope lock
			MutexLocker scoped_lock( queue_mutex );
			while( msg_queue.empty() && running) {
				queue_mutex.wait( queue_cv );
			}
			if( !running ) return;
			
			//pop stuff
			msg = msg_queue.front();
			msg_queue.pop();
		}   // unlock the mutex
		
		//Send message
		// TODO WARNING THE SOCKET IS NOT PROTECTED!!!
		if( sendmessage( sock, msg.type, msg.uid, msg.datalen, msg.data ) )
		{
			disconnect(id, "Broadcaster: Send error");
			return;
		}
	}
}

//this is called all the way from the receiver threads, we should process this swiftly
//and keep in mind that it is called crazily and concurently from lots of threads
//we MUST copy the data too
//also, this function can be called by threads owning clients_mutex !!!
void Broadcaster::queueMessage(unsigned int uid, int type, char* data,
		unsigned int len)
{
    STACKLOG;
	// for now lets just queue msgs in the order received to make things simple
	queue_entry_t msg = { uid, type, "", len };
	memset( msg.data, 0, MAX_MESSAGE_LENGTH );
	memcpy( msg.data, data, len );

	MutexLocker scoped_lock( queue_mutex );
	msg_queue.push( msg );
	//signal the thread that new data is waiting to be sent
	queue_cv.signal();
	
}