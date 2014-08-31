#include "RemoteEventListener.h"

IDAStealth::RemoteEventListener::RemoteEventListener()
{
	//boost::thread t(boost::bind(&RemoteEventListener::backgroundThread, this));
}

IDAStealth::RemoteEventListener::~RemoteEventListener()
{
	// exit thread
}

void IDAStealth::RemoteEventListener::backgroundThread()
{
	// create socket, enter infinite loop, read events, set ip and port
}