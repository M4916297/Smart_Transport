#include <QCoreApplication>
#include "systemcontrol.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //启动服务器，并开启监听
//    MyTcpServer *server = new MyTcpServer;
//    server->listen(QHostAddress::Any,10086);4

    SystemControl sc;
    sc.systemInit();
    sc.systemStart();

    return a.exec();
}
