#include "mainpage.h"
#include "ui_mainpage.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#define CONFIGFILENAME "config.ini"
#define DEFGROUP        "set"
//路口四个方向交通灯编号【注意：其与路的编号相同】
#define LEFTID          "left_id"
#define RIGHTID         "right_id"
#define UPID            "up_id"
#define DOWNID          "down_id"
//路口编号
#define CROSSINGID      "crossing_id"
//绿灯时长
#define GREENTIME       "green_time"
//网络相关
#define SERVERIP        "server_ip"
#define SERVERPORT      "server_port"

MainPage::MainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPage)
{
    ui->setupUi(this);

    crossingId = getCrossingId();
    leftId = getLeftId();
    rightId = getRightId();
    upId = getUpId();
    downId = getDownId();
    serverIp = getServerIp();
    serverPort = getServerPort();
    //假设greenTime为5s(2s常亮 3s闪烁)，黄灯固定3s闪烁，红灯则为gt+yt
    int gt = getGreenTime();

    //1.水平方向亮灯标记数组hcArr 填充数据
    //绿灯前gt-3s常亮
    for(int i = 0; i < (gt - 3)*2; i++)
        hcArr.append(0);

    //绿灯3s闪烁
    for(int i = 0; i < 6; i++) {
        if(i % 2 == 0)
            hcArr.append(0);
        else
            hcArr.append(3);
    }
    //黄灯3s闪烁
    for(int i = 0; i < 6; i++) {
        if(i % 2 == 0)
            hcArr.append(2);
        else
            hcArr.append(3);
    }
    //红灯常亮gt+3s
    for(int i = 0; i < (gt+3)*2; i++)
        hcArr.append(1);

    //2.竖直方向亮灯标记数组vcArr 填充数据
    // 亮灯过程和 水平方向相反
    //红灯常亮gt+3s
    for(int i = 0; i < (gt+3)*2; i++)
        vcArr.append(1);

    //绿灯前gt-3s常亮
    for(int i = 0; i < (gt - 3)*2; i++)
        vcArr.append(0);

    //绿灯3s闪烁
    for(int i = 0; i < 6; i++) {
        if(i % 2 == 0)
            vcArr.append(0);
        else
            vcArr.append(3);
    }
    //黄灯3s闪烁
    for(int i = 0; i < 6; i++) {
        if(i % 2 == 0)
            vcArr.append(2);
        else
            vcArr.append(3);
    }

    //3.亮灯信号和槽函数
    lightChangeTimer = new QTimer(this);
    connect(lightChangeTimer,&QTimer::timeout,
            this,&MainPage::lightChangeTimerTimeoutSlot);

    //4.启动定时器
    lightChangeTimer->start(500);

    //5.实例化套接字对象
    client = new QTcpSocket(this);

    connect(client,&QTcpSocket::connected,
            this,&MainPage::clientConnectedSlot);

    connect(client,&QTcpSocket::readyRead,
            this,&MainPage::clientReadyReadSlot);

    //设置title
    setWindowTitle(QString("模拟路口: %1-未连接").arg(crossingId));

    //连接到服务器
    client->connectToHost(serverIp,serverPort);
}

MainPage::~MainPage()
{
    delete ui;
}

void MainPage::clientConnectedSlot()
{
    setWindowTitle(QString("模拟路口: %1-已连接").arg(crossingId));
    //上报个人信息到服务器: 信号灯为1
    QJsonObject obj;
    obj.insert("type",1000);
    obj.insert("client_type",1);
    client->write(QJsonDocument(obj).toJson());
}

//核心方法                             灯编号[和roadId一致]
void MainPage::sendLightStatusToServer(int lightId, int color)
{
    //如果当前道路没有信号灯，则不上报
    if(lightId == -1)
        return;
    QJsonObject obj;
    obj.insert("type",3001);
    obj.insert("light_id",crossingId);
    obj.insert("road_id",lightId);
    obj.insert("color",color);
    client->write(QJsonDocument(obj).toJson());
    //延迟等待，保证数据发送完全
    client->waitForBytesWritten(200);
}


void MainPage::paintEvent(QPaintEvent *)
{
    QPixmap img(":/images/background.jpg");

    QPainter painter;

    //1.往图片上绘制 红绿灯
    painter.begin(&img);
    //四种颜色的画笔
    QPen rpen, gpen, ypen, wpen;
    QBrush rbrush, gbrush, ybrush, wbrush;
    rpen.setColor(Qt::red);
    rbrush.setColor(Qt::red);
    rbrush.setStyle(Qt::SolidPattern);
    gpen.setColor(Qt::green);
    gbrush.setColor(Qt::green);
    gbrush.setStyle(Qt::SolidPattern);
    ypen.setColor(Qt::yellow);
    ybrush.setColor(Qt::yellow);
    ybrush.setStyle(Qt::SolidPattern);
    wpen.setColor(Qt::white);
    wbrush.setColor(Qt::white);
    wbrush.setStyle(Qt::SolidPattern);

    //红绿灯在图片上位置固定如下：
    //left  53, 78, 12, 68
    //right 165, 78, 12, 68
    //up    80, 48, 68, 12
    //down  80, 163, 68, 12

    //水平方向方向画笔画刷准备
    painter.setPen(hcolor == 0 ? gpen :
                                 hcolor == 1 ? rpen :
                                               hcolor == 2 ? ypen :
                                                             wpen);
    painter.setBrush(hcolor == 0 ? gbrush :
                                   hcolor == 1 ? rbrush :
                                                 hcolor == 2 ? ybrush :
                                                               wbrush);
    //left 绘制左边路口交通灯
    if(leftId != -1)
        painter.drawRect(53, 78, 12, 68);

    //right
    if(rightId != -1)
        painter.drawRect(165, 78, 12, 68);

    //竖直方向方向画笔画刷准备
    painter.setPen(vcolor == 0 ? gpen :
                                 vcolor == 1 ? rpen :
                                               vcolor == 2 ? ypen :
                                                             wpen);
    painter.setBrush(vcolor == 0 ? gbrush :
                                   vcolor == 1 ? rbrush :
                                                 vcolor == 2 ? ybrush :
                                                               wbrush);
    //up 绘制上边路口交通灯
    if(upId != -1)
        painter.drawRect(80, 48, 68, 12);

    //down
    if(downId != -1)
        painter.drawRect(80, 163, 68, 12);

    painter.end();

    //2.绘制图片到窗体上
    painter.begin(this);
    QPixmap img2 = img.scaled(this->width(),this->height(),
                              Qt::KeepAspectRatio);
    painter.drawPixmap((this->width() - img2.width())/2,
                       (this->height() - img2.height())/2,
                       img2);
    painter.end();
}

int MainPage::getLeftId()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 左侧信号灯编号 与roadId一致
    if(!set.allKeys().contains(LEFTID)){
        set.setValue(LEFTID,-1);
    }
    int leftId = set.value(LEFTID).toInt();

    set.endGroup();

    return leftId;
}

int MainPage::getRightId()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 左侧信号灯编号 与roadId一致
    if(!set.allKeys().contains(RIGHTID)){
        set.setValue(RIGHTID,-1);
    }
    int rightId = set.value(RIGHTID).toInt();

    set.endGroup();

    return rightId;
}

int MainPage::getUpId()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 左侧信号灯编号 与roadId一致
    if(!set.allKeys().contains(UPID)){
        set.setValue(UPID,-1);
    }
    int upId = set.value(UPID).toInt();

    set.endGroup();

    return upId;
}

int MainPage::getDownId()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 左侧信号灯编号 与roadId一致
    if(!set.allKeys().contains(DOWNID)){
        set.setValue(DOWNID,-1);
    }
    int downId = set.value(DOWNID).toInt();

    set.endGroup();

    return downId;
}

int MainPage::getCrossingId()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 左侧信号灯编号 与roadId一致
    if(!set.allKeys().contains(CROSSINGID)){
        set.setValue(CROSSINGID,-1);
    }
    int crossingId = set.value(CROSSINGID).toInt();

    set.endGroup();

    return crossingId;
}

int MainPage::getGreenTime()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 绿灯时长
    if(!set.allKeys().contains(GREENTIME)){
        set.setValue(GREENTIME,5);
    }
    int greenTime = set.value(GREENTIME).toInt();
    //至少5s
    if(greenTime < 5)
        greenTime = 5;

    set.endGroup();

    return greenTime;
}

void MainPage::clientReadyReadSlot()
{
    QByteArray data = client->readAll();
    //拆分收到的 json数据,有可能多条完整数据,例如：
    // {"type": 1000,"client_type": 2}{"type": 2001}
    int count = 0;
    for(int i = 0; i < data.length(); i++) {
        if(data.at(i) == '{') {
            count++;
        }else if(data.at(i) == '}') {
            count--;
            if(count == 0) {
                QByteArray temp = data.mid(0,i+1);
                //处理一条完整的数据
                handleFrame(temp);
                //剔除之前完整的那条数据
                data = data.mid(i+1);
                i = -1;
            }
        }
    }
}

void MainPage::handleFrame(const QByteArray &data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    //解析得到控制信息类型 路口id
    int type = obj.value("type").toInt();
    int crossing_id = obj.value("crossing_id").toInt();
    //路口校验
    if(crossing_id != crossingId)
        return;
    if(type == 6001) {
        //获取工作模式
        mode = obj.value("mode").toInt();
        //上传最新模式到服务器
        obj.remove("type");
        obj.insert("type",4001);
        client->write(QJsonDocument(obj).toJson());
    }
}

//修改代码
void MainPage::lightChangeTimerTimeoutSlot()
{
    //添加模式相关代码【新增部分】
    // 默认0-auto 1-水平方向通行 2-垂直方向通行 3-禁行 4-夜间
    if(mode == 0) {
        hcolor = hcArr[index];
        vcolor = vcArr[index];
        index++;
        if(index >= hcArr.length())
            index = 0;
    }else if(mode == 1) {
        hcolor = 0;
        vcolor = 1;
    }else if(mode == 2) {
        hcolor = 1;
        vcolor = 0;
    }else if(mode == 3) {
        hcolor = 1;
        vcolor = 1;
    }else if(mode == 4) {
        //要实现交替闪烁
        hcolor = hcolor == 2 ? 3 : 2;
        vcolor = hcolor;
    }
    update();
    //如果网络不同，则不往下执行【发送】
    if(client->state() != QAbstractSocket::ConnectedState)
        return;
    //路口灯颜色改变后，需要上传到服务器【1个路口 4个灯】
    sendLightStatusToServer(leftId,hcolor);
    sendLightStatusToServer(rightId,hcolor);
    sendLightStatusToServer(upId,vcolor);
    sendLightStatusToServer(downId,vcolor);
}

QString MainPage::getServerIp()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 绿灯时长
    if(!set.allKeys().contains(SERVERIP)){
        set.setValue(SERVERIP,"127.0.0.1");
    }
    QString ip = set.value(SERVERIP).toString();
    set.endGroup();

    return ip;
}

int MainPage::getServerPort()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);

    //解析配置文件中 绿灯时长
    if(!set.allKeys().contains(SERVERPORT)){
        set.setValue(SERVERPORT,10086);
    }
    int port = set.value(SERVERPORT).toInt();
    set.endGroup();

    return port;
}

