#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);

signals:
    //转发指令
    void sendDataToAll(int clientType, QByteArray data);

public slots:
    //客户端断开连接槽函数
    void socketDisconnectedSlot();
    //销毁子线程槽函数
    void threadFinishedSlot();

protected:
    //一旦有客户端被调用,
    void incomingConnection(qintptr socketDescriptor);

};

#endif // MYTCPSERVER_H
