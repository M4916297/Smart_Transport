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

public slots:
   //红绿灯 闪烁定时器
   void lightChangeTimerTimeoutSlot();

   //网络传输相关槽函数
   void clientConnectedSlot();

   void clientReadyReadSlot();

protected:
   //重写绘图事件
   void paintEvent(QPaintEvent *e);

   //发送每个信号灯当前的颜色 给服务器
   void sendLightStatusToServer(int lightId,int color);

   //处理协议数据
   void handleFrame(const QByteArray &data);


private:
    Ui::MainPage *ui;

    //水平 竖直方向 灯的颜色
    int hcolor = 0, vcolor = 0;

    //水平方向 竖直方向 灯亮颜色列表[0,0,0,0,3,0,3,0]
    //后期可以0.5s定时器         绿灯常亮2s、闪烁2s
    QList<int> hcArr,vcArr;
    //与定时器对应的 元素索引
    int index = 0;
    //亮灯定时器
    QTimer *lightChangeTimer;

    //路口及交通灯编号
    int crossingId = -1;
    int leftId = -1,rightId = -1,upId = -1,downId = -1;
    //服务器IP及端口
    QString serverIp;
    int serverPort;

    //获取初始化配置文件内容
    int getLeftId();
    int getRightId();
    int getUpId();
    int getDownId();
    int getCrossingId();
    int getGreenTime();
    QString getServerIp();
    int getServerPort();

    //网络传输套接字
    QTcpSocket *client;

    //工作模式[默认0-auto] 其他值：1-水平方向通行 2-垂直方向通行 3-禁行4-夜间
    int mode = 0;
};

#endif // MAINPAGE_H

