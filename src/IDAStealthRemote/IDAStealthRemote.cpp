// IDAStealthRemote.cpp : Defines the entry point for the console application.
//

#include "IDAStealthRemote.h"
#include <boost/filesystem.hpp>

using namespace std;

int _tmain(int argc, char* argv[])
{
	try
	{		
		if (!boost::filesystem::exists("HideDebugger.dll"))
		{
			cout << "HideDebugger.dll not found...terminating" << endl;
			return 1;
		}
		cout << "IDAStealth remote server v1.3, Jan Newger, 2009-2010" << endl << endl;
		unsigned short port = 4242;
		if (argc == 2) port = boost::lexical_cast<unsigned short>(argv[1]);
		else cout << "Usage: IDAStealthRemote.exe <port>" << endl
				  << "Using standard port " << port << endl;
		cout << "Starting server..." << endl;

		boost::asio::io_service ioService;
		RemoteStealth::RemoteStealthServer server(ioService, port);
		server.run();
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
	}
	return 0;
}