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
#include "receiver.h"

void *s_lithreadstart(void* vid)
{
	((Receiver*)vid)->threadstart();
	return NULL;
}


Receiver::Receiver(Sequencer *seq)
{
	sequencer=seq;
	id=0;
	sock=0;
	alive=false;
}

void Receiver::reset(int pos, SWInetSocket *socky)
{
	if (alive)
	{
		logmsgf(LOG_ERROR,"Whoa, receiver is still alive!");
		return;
	}
	id=pos;
	sock=socky;
	alive=true;
	//start a listener thread
	pthread_create(&thread, NULL, s_lithreadstart, this);
}

void Receiver::stop()
{
	pthread_cancel(thread);
	pthread_join(thread, NULL);
	alive=false;
}


Receiver::~Receiver(void)
{
	stop();
}

void Receiver::threadstart()
{
	logmsgf(LOG_DEBUG,"Receive thread %d started", id);
	//get the vehicle description
	int type;
	unsigned int source;
	unsigned int len;
	//security fix: we limit the size of the vehicle name to 128 characters <- from Luigi Auriemma
	if (Messaging::receivemessage(sock, &type, &source, &len, dbuffer, 128)) {sequencer->disconnect(id, "Messaging abuse 1"); return;};
	if (type!=MSG2_USE_VEHICLE) {sequencer->disconnect(id, "Protocol error 1"); return;};
	//security
	dbuffer[len]=0;
	//we queue the use vehicle message for others
	sequencer->queueMessage(id, type, dbuffer, len);
	//get the buffer size, not really usefull but a good way to detect errors
	if (Messaging::receivemessage(sock, &type, &source, &len, dbuffer, 4)) {sequencer->disconnect(id, "Messaging abuse 2"); return;};
	if (type!=MSG2_BUFFER_SIZE) {sequencer->disconnect(id, "Protocol error 2"); return;};
	unsigned int buffersize=*((unsigned int*)dbuffer);
	if (buffersize>MAX_MESSAGE_LENGTH) {sequencer->disconnect(id, "Memory error from client"); return;};
	//notify the client of all pre-existing vehicles
	sequencer->notifyAllVehicles(id);
	//okay, we are ready, we can receive data frames
	sequencer->enableFlow(id);
	logmsgf(LOG_DEBUG,"Slot %d is switching to FLOW", id);
	while (1)
	{
		if (Messaging::receivemessage(sock, &type, &source, &len, dbuffer, MAX_MESSAGE_LENGTH)) {sequencer->disconnect(id, "Game connection closed"); return;};
		if (type!=MSG2_VEHICLE_DATA && type!=MSG2_CHAT && type!=MSG2_FORCE) {sequencer->disconnect(id, "Protocol error 3"); return;};
		sequencer->queueMessage(id, type, dbuffer, len);
	}
}
