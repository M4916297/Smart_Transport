#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);

    //重写设置操作符
    bool setSocketDescriptor(qintptr socketDescriptor,
            QAbstractSocket::SocketState socketState = ConnectedState,
            QIODevice::OpenMode openMode = ReadWrite);

    //将QHostAddress转换成IPv4字符串
    QString hostAddressToIPv4String(QHostAddress addr);

    void handleFrame(const QByteArray &data);

signals:
    //发送指令给服务器 clientType为接收者身份，data为指令数据
    void sendDataToAll(int clientType, QByteArray data);

public slots:
    void readyReadSlot();

    //发送指令给客户端
    void serverSendDataToAllSlot(int clientType,
                                 QByteArray data);

private:
    //IP地址
    QString ip;
    //端口号
    quint16 port;
    //套接字标识符
    qintptr socketDes;

    //socket对应的身份标志
    int clientType = -1;

};

#endif // MYTCPSOCKET_H
