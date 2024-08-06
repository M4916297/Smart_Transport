#ifndef SYSTEMCONTROL_H
#define SYSTEMCONTROL_H

#include <QObject>

class MyTcpServer;
//系统控制类
class SystemControl : public QObject
{
    Q_OBJECT
public:
    explicit SystemControl(QObject *parent = nullptr);
    //系统初始化
    void systemInit();
    //系统启动
    void systemStart();

signals:

public slots:

private:
    MyTcpServer *server;
    //工具方法，获取配置文件中port
    int getServerListenPort();
    //服务器绑定的端口
    quint16 port;
};


#endif // SYSTEMCONTROL_H

