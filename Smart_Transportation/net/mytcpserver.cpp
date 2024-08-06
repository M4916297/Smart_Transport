#include "mytcpserver.h"
#include <QThread>
#include "mytcpsocket.h"
#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent) : QTcpServer(parent)
{

}

//销毁socket
void MyTcpServer::socketDisconnectedSlot()
{
    //获取通信的套接字对象
    MyTcpSocket *socket = qobject_cast<MyTcpSocket *>(sender());
    //断开信号与槽，防止二次析构
    disconnect(socket, nullptr, nullptr, nullptr);
    disconnect(this, nullptr, socket, nullptr);
    //获取socket所在的子线程
    QThread *th = socket->thread();
    //删除释放socket资源
    socket->deleteLater();
    qDebug() << "socket 销毁成功!";
    connect(th, &QThread::finished,
            this, &MyTcpServer::threadFinishedSlot);
    //结束子线程
    th->quit();
}

//销毁子线程
void MyTcpServer::threadFinishedSlot()
{
    QThread *th = qobject_cast<QThread *>(sender());
    disconnect(th, nullptr, nullptr, nullptr);
    th->deleteLater();
    qDebug() << "th子线程 销毁成功!";
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "客户端连接成功, incomingConnection: " << socketDescriptor;

    //1.创建socket
    MyTcpSocket *socket = new MyTcpSocket;
    if(socket == nullptr)
        return;
    socket->setSocketDescriptor(socketDescriptor);

    //客户端断开连接处理
    connect(socket, &MyTcpSocket::disconnected,
            this, &MyTcpServer::socketDisconnectedSlot);

    //绑定信号和槽，实现 json指令转发
    connect(socket,&MyTcpSocket::sendDataToAll,
            this,&MyTcpServer::sendDataToAll);
    connect(this,&MyTcpServer::sendDataToAll,
            socket,&MyTcpSocket::serverSendDataToAllSlot);



    //2.创建子线程
    QThread *th = new QThread;
    if(th == nullptr){
        socket->disconnectFromHost();
        socket->waitForDisconnected(500);
        socket->deleteLater();
        return;
    }

    //将套接字对象移入子线程
    socket->moveToThread(th);
    //直接启动子线程
    th->start();
}
