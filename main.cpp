
#include "server_app.h"
#include "server_conf.h"
#include "QCoreApplication"

ServerApp* Server = nullptr;

QObject* GetServerApp()
{
	return Server;
}

int main(int argv, char** argc)
{
    QCoreApplication app(argv, argc);
    Server = new ServerApp();
    if(Server)
		Server->Start();
    QCoreApplication::exec();
	return 0;
}