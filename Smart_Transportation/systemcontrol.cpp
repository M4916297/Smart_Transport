#include "systemcontrol.h"
#include <QSettings>
#include <QDebug>
#include "mytcpserver.h"
#define CONFIGFILE "config.ini"
#define DEFGROUP "set"
#define SERVERPORT "listen_port"

SystemControl::SystemControl(QObject *parent) : QObject(parent)
{
    server = new MyTcpServer;
}

void SystemControl::systemInit()
{
    port = getServerListenPort();
    qDebug() << "system init ...";
}

void SystemControl::systemStart()
{
    bool flag = server->listen(QHostAddress::Any,port);
    qDebug() << "system start, port: " << port
             << (flag ? " success" : " error");
}

//获取ini配置文件中端口值
int SystemControl::getServerListenPort()
{
    //1.实例化对象
    QSettings set(CONFIGFILE,QSettings::IniFormat);
    //2.开始分组【节 section】
    set.beginGroup(DEFGROUP);
    //3.读取配置文件内容
    bool flag = set.allKeys().contains(SERVERPORT);
    if(!flag) {
        //设置默认值为10086
        set.setValue(SERVERPORT,10086);
    }
    //获取端口值
    int port = set.value(SERVERPORT).toInt();
    //4.结束分组
    set.endGroup();
    return port;
}
