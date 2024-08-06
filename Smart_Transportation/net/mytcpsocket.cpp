#include "mytcpsocket.h"
#include <QDebug>
#include <QHostAddress>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

MyTcpSocket::MyTcpSocket(QObject *parent) : QTcpSocket(parent)
{
    connect(this,&MyTcpSocket::readyRead,
            this,&MyTcpSocket::readyReadSlot);
}

void MyTcpSocket::readyReadSlot()
{
    //读取josn指令
    QByteArray bytearray = this->readAll();
    //josn样例{"type": 1000,"client_type": 2}
    qDebug()<<"readAll:"<<bytearray;

    int count=0;
    for(int i=0;i<bytearray.size();i++)
    {
        if(bytearray.at(i)=='{')
        {
            count++;
        }
        if(bytearray.at(i)=='}')
        {
            count--;
            if(count==0)
            {
                QByteArray jsonData = bytearray.mid(0,i+1);
                qDebug()<<"一条json指令"<<jsonData;

                //3.逐行指令解析
                handleFrame(jsonData);

                bytearray = bytearray.mid(i+1);
                i = -1;
            }
        }
    }
}

//发送json指令(来自server转发) 给 客户端
void MyTcpSocket::serverSendDataToAllSlot(int clientType, QByteArray data)
{
    //形参 clientType：需要接收【转发指令】的客户端
    //当前客户端身份标识 this->clientType;
    if(this->clientType != clientType)
        return;

    //发送指令 给客户端
    this->write(data);
}

bool MyTcpSocket::setSocketDescriptor(qintptr socketDescriptor, SocketState socketState, OpenMode openMode)
{
    bool ok = QTcpSocket::setSocketDescriptor(socketDescriptor,
                                              socketState,
                                              openMode);

    if(!ok)
        return false;

    port = this->peerPort();
    socketDes = socketDescriptor;
    ip = hostAddressToIPv4String(this->peerAddress());

    qDebug()<<"port:"<<port<<endl<<"IP"<<ip<<endl<<"socketDes"<<socketDes;

    return true;
}

//工具函数：得到IPv4地址
QString MyTcpSocket::hostAddressToIPv4String(QHostAddress addr)
{
    QString ip = "";

    //获取协议类型
    QAbstractSocket::NetworkLayerProtocol pt = addr.protocol();

    if(pt == QAbstractSocket::IPv4Protocol) {
        ip = addr.toString();
    }else if(pt == QAbstractSocket::IPv6Protocol) {
        quint32 ipNum = addr.toIPv4Address();
        ip = QString("%1.%2.%3.%4")
                .arg((ipNum >> 24) & 0xFF)
                .arg((ipNum >> 16) & 0xFF)
                .arg((ipNum >> 8) & 0xFF)
                .arg(ipNum & 0xFF);
    }

    return ip;
}

//json指令解析
void MyTcpSocket::handleFrame(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    int type = obj.value("type").toInt(-1);

    if(type== 1000)
    {
        //解析client_type的值
        clientType = obj.value("client_type").toInt();
        qDebug()<<"client_type:"<<clientType;

    }else if(type >= 2001 && type <= 2008) {
        //转发给 2、3
        emit sendDataToAll(2,data);
        emit sendDataToAll(3,data);
    }else if(type == 3001) {
        //转发给 0、2、3
        emit sendDataToAll(0,data);
        emit sendDataToAll(2,data);
        emit sendDataToAll(3,data);
    }else if(type == 4001) {
        //转发给 2、3
        emit sendDataToAll(2,data);
        emit sendDataToAll(3,data);
    }else if(type >= 5001 && type <= 5004) {
        //转发给 0
        emit sendDataToAll(0,data);
    }else if(type == 6001) {
        //转发给 1
        emit sendDataToAll(1,data);
    }
}
