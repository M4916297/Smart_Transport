#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class MainPage;
}

class MainPage : public QWidget
{
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = nullptr);
    ~MainPage();

    //方法封装：设置大灯及双闪状态
    void setLightState(bool flag);
    void setAlarmLightState(bool flag);
    //拆分json指令
    void handleFrame(const QByteArray &data);
    //设置车辆锁定状态
    void setLockState(bool flag);
    void sendCarStatusToServer(int type,bool sw);
    //车辆启动，启动定时器，更新位置
    void carStart(bool sw);

public slots:
    //客户端连接到服务器
    void clientConnectedSlot();
    //接收服务器发送指令信息并处理
    void clientReadyReadSlot();
    //车辆运行槽函数
    void carRunTimerTimeoutSlot();

private:
    Ui::MainPage *ui;
    //大灯开启状态
    bool lightState = false;
    //双闪开启状态
    bool alarmLightState = false;
    //双闪定时器
    QTimer *alarmLightTimer;
    //
    QTcpSocket *client;

    //车辆及网络配置 基本信息
    QString serverIp;
    quint16 serverPort;
    int carId;
    QString carKey;
    //工具方法(只在类内调用): 配置文件读取
    QString getServerIp();
    int getServerPort();
    int getCarId();
    QString getCarKey();
    //解锁状态: 默认上锁
    bool lockState = true;
    //车辆行驶道路列表
    QStringList carRunList;
    QStringList getCarRunList();
    //车辆运行定时器
    QTimer *carRunTimer;
    //carRunList索引【用来获取roadId】
    int roadIndex = 0;
    //车辆在路上所处的位置【取值：0-100】
    int roadPos = 0;
    //记录每条路上 红绿灯情况<路id,红绿灯颜色>
    QHash<int,int> roadColorHash;

protected:
    //绘图事件
    void paintEvent(QPaintEvent *e);
};

#endif // MAINPAGE_H
