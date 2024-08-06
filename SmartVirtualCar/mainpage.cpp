#include "mainpage.h"
#include "ui_mainpage.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

#define CONFIGFILENAME  "config.ini"
#define DEFGROUP        "set"
#define SERVERIP        "server_ip"
#define SERVERPORT      "server_port"
// 车辆编号
#define CARID           "car_id"
// 车辆密钥
#define CARKEY          "car_key"
// 车辆运行路径列表
#define CARRUNLIST "car_run_list"

MainPage::MainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPage)
{
    ui->setupUi(this);

    //解析配置文件，获取相关值
    serverIp = getServerIp();
    serverPort = getServerPort();
    carId = getCarId();
    carKey = getCarKey();
    carRunList = getCarRunList();

    qDebug() << serverIp << " " << serverPort;
    qDebug() << carId << " " << carKey;
    qDebug() <<"carRunList:"<<carRunList;

    //实例化车辆运行定时器
    carRunTimer = new QTimer(this);

    //实例化 通信套接字对象
    client = new QTcpSocket(this);

    //车辆运行
    connect(carRunTimer,&QTimer::timeout,
            this,&MainPage::carRunTimerTimeoutSlot);

    //注册信号和槽
    connect(client,&QTcpSocket::connected,
            this,&MainPage::clientConnectedSlot);

    connect(client,&QTcpSocket::readyRead,
            this,&MainPage::clientReadyReadSlot);

    //设置主窗体标题
    this->setWindowTitle(QString("模拟车辆: %1-未连接").arg(carId));

    //连接到服务器
    client->connectToHost(serverIp,serverPort);



    //实例化定时器对象
    alarmLightTimer = new QTimer(this);
    //绑定timeout信号和槽，实现双闪功能
    //timeout计时到设置时间后发送信号
    connect(alarmLightTimer,&QTimer::timeout,
            this,[=](){
        //如果双闪开启则500ms后关闭
        if(alarmLightState) {
            alarmLightState = false;
            //如果双闪关闭500ms后开启
        }else {
            alarmLightState = true;
        }
        update();
    });
    //双闪及开灯测试
    //    setLightState(true);
    //    setAlarmLightState(true);
}

MainPage::~MainPage()
{
    delete ui;
}

//核心功能：绘制车辆图片到主窗体上(注意放大缩小时等比例显示)，要有大灯和双闪效果
void MainPage::paintEvent(QPaintEvent *)
{
    QPixmap img(":/img/car.png");
    QPainter painter;
    //1.往img车上绘制大灯和双闪
    painter.begin(&img);
    //创建画笔
    QPen pen;
    //创建画刷
    QBrush brush;
    //设置画笔颜色
    pen.setColor(Qt::yellow);
    //设置画刷颜色
    brush.setColor(Qt::yellow);
    //设置画刷样式:纯色填充
    brush.setStyle(Qt::SolidPattern);
    //初始化画笔
    painter.setPen(pen);
    //初始化画刷
    painter.setBrush(brush);
    //绘制大灯
    if(lightState) { //固定尺寸
        painter.drawEllipse(17,112,35,36);
        painter.drawEllipse(216,112,35,36);
    }
    //绘制双闪灯
    if(alarmLightState) {
        painter.drawRect(17,155,35,15);
        painter.drawRect(216,155,35,15);
    }
    painter.end();
    //2.绘制图片到主窗体上
    painter.begin(this);
    //绘制图片时保证按比例压缩、伸展
    //获取合适比例新图片
    QPixmap img2 = img.scaled(this->width(),this->height(),
                              Qt::KeepAspectRatio);
    //QRect target(0,0,this->width(),this->height());
    //绘制中心区域
    painter.drawPixmap((this->width()-img2.width())/2,
                       (this->height()-img2.height())/2,
                       img2);
    painter.end();
}

//设置大灯的开关
void MainPage::setLightState(bool flag)
{
    lightState = flag;
    update();
}

//设置双闪的开关
void MainPage::setAlarmLightState(bool flag)
{
    alarmLightState = flag;
    //如果开启则开启定时器
    if(flag) {
        alarmLightTimer->start(500);
    }else {
        //如果关闭则暂停定时器,关闭双闪,重绘
        alarmLightTimer->stop();
        alarmLightState = false;
        update();
    }
}

//json指令解析及处理
void MainPage::handleFrame(const QByteArray &data)
{
    // {"type": 5002, "sw": true}
    //1.解析json指令
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    int type = obj.value("type").toInt();
    //获取小车id
    int id = obj.value("id").toInt();
    //获取小车key密钥
    QString key = obj.value("key").toString();

    //车辆id、key 校验【额外开放 3001指令 的处理】
    if((id != carId || key != carKey) && type != 3001)
        return;

    bool sw = obj.value("sw").toBool();

    //2.根据获取type值，然后做出相应处理
    if(type == 5001) {
        //车辆启动(必须解锁)
        if(lockState)
            return;
        //启动车辆
        carStart(sw);
        qDebug() << "车辆成功启动!";

        //上报启动状态: 2006
        sendCarStatusToServer(2006,sw);
    }else if(type == 5002) {
        //车灯控制
        setLightState(sw);

        //上报车灯状态: 2001
        sendCarStatusToServer(2001,sw);
    }else if(type == 5003) {
        //双闪控制
        setAlarmLightState(sw);

        //上报双闪状态: 2002
        sendCarStatusToServer(2002,sw);
    }else if(type == 5004) {
        //解锁|上锁 控制
        setLockState(sw);

        //上报锁定状态: 2007
        sendCarStatusToServer(2007,sw);
    }else if(type == 3001) {
        //信号灯状态
        int roadId = obj.value("road_id").toInt();
        int color = obj.value("color").toInt();
        roadColorHash.insert(roadId,color);
   }
}

void MainPage::setLockState(bool flag)
{
    lockState = flag;

    setWindowTitle(QString("模拟车辆：%1-已连接-%2")
                   .arg(carId)
                   .arg(lockState ? "锁定" : "解锁"));
}

void MainPage::sendCarStatusToServer(int type, bool sw)
{
    QJsonObject obj;
    obj.insert("type",type);
    obj.insert("id",carId);
    obj.insert("sw",sw);

    QByteArray data = QJsonDocument(obj).toJson();

    //2.借助套接字发送给服务器
    client->write(data);
}

//客户端连接成功后发送身份确认信息
void MainPage::clientConnectedSlot()
{
    this->setWindowTitle(QString("模拟车辆: %1-已连接")
                         .arg(carId));
    //1.按照协议封装 得到 Json指令(QByteArray)
    QJsonObject obj;
    obj.insert("type",1000);
    obj.insert("client_type",0);

    QJsonDocument doc = QJsonDocument(obj);
    QByteArray data = doc.toJson();

    //2.发送指令给服务器
    client->write(data);
}

//收取服务器发送的json指令并处理
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
                //处理一条完整json指令
                handleFrame(temp);
                //剔除之前完整的那条数据
                data = data.mid(i+1);
                i = -1;
            }
        }
    }
}

QString MainPage::getServerIp()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);
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
    //解析配置文件中 指定属性（车辆编号，默认0）
    if(!set.allKeys().contains(SERVERPORT)){
        set.setValue(SERVERPORT,10086);
    }
    int port = set.value(SERVERPORT).toInt();
    set.endGroup();
    return port;
}

int MainPage::getCarId()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);
    //解析配置文件中 指定属性（车辆编号，默认0）
    if(!set.allKeys().contains(CARID)){
        set.setValue(CARID,0);
    }
    int id = set.value(CARID).toInt();
    set.endGroup();
    return id;
}

QString MainPage::getCarKey()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);
    if(!set.allKeys().contains(CARKEY)){
        set.setValue(CARKEY,"123456");
    }
    QString carKey = set.value(CARKEY).toString();
    set.endGroup();
    return carKey;
}

//获取车辆运行路径列表
QStringList MainPage::getCarRunList()
{
    QSettings set(CONFIGFILENAME,QSettings::IniFormat);
    set.beginGroup(DEFGROUP);
    //解析配置文件中 指定属性（车辆运行路径）
    if(!set.allKeys().contains(CARRUNLIST))
    {
        QStringList list;
        list << "0" << "6" << "3" << "9";
        set.setValue(CARRUNLIST, list);
    }
    QStringList list = set.value(CARRUNLIST).toStringList();
    set.endGroup();
    return list;
}

//车辆启动，启动定时器，更新位置
void MainPage::carStart(bool sw)
{
    if(sw) {
        //车辆定时器启动已1000ms计时
        carRunTimer->start(1000);
    }else {
        //车辆暂停
        carRunTimer->stop();
    }
}
//【核心功能】
//车辆启动后运行定时器
void MainPage::carRunTimerTimeoutSlot()
{
    //如果道路列表为空,什么也不做
    if(carRunList.size() <= 0)
        return;
    //获取具体哪条路上
    int roadId = carRunList.at(roadIndex).toInt();
    //按照协议发送 小车当前位置信息[路id，位置]
    QJsonObject obj;
    //2008协议
    obj.insert("type",2008);
    //车辆编号
    obj.insert("id",carId);
    //道路编号
    obj.insert("road_id",roadId);
    //道路位置
    obj.insert("road_pos",roadPos);
    client->write(QJsonDocument(obj).toJson());

    //红路灯处理
    if(roadPos==100)
    {
        //获取该路上的红路灯
        int color = roadColorHash.value(roadId,0);
        //如果路上的红路灯是红灯或者是黄灯则停止
        if(color == 1||color == 2)
            return;
    }


    //调整获取最新车辆位置
    roadPos += 10;
    //如果车辆道路位置大于100
    //车辆到下一个道路
    if(roadPos > 100) {
        roadPos = 0;
        roadIndex++;
        //如果车辆跑完道路列表中的所有道路
        //车辆到第一条道路
        if(roadIndex >= carRunList.length()) {
            roadIndex = 0;
        }
    }
}

void testHash()
{
    QHash<int,int> roadColor;
    roadColor.insert(1,1);
    qDebug() << "roadColor: " << roadColor;
    //如果key已存在，则覆盖
    roadColor.insert(1,2);
    qDebug() << "roadColor: " << roadColor;
    //如果key不存在，则新增
    roadColor.insert(2,1);
    qDebug() << "roadColor: " << roadColor;
    roadColor.insert(2,3);
    qDebug() << "roadColor: " << roadColor;
}
