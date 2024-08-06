#include "mainpage.h"
#include "ui_mainpage.h"
#include <QTcpSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

//以往代码
#define CONFIGFILENAME "config.ini"
#define DEFGROUPNAME "set"
#define SERVERIP "server_ip"
#define SERVERPORT "server_port"

MainPage::MainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPage)
{
    ui->setupUi(this);

    //1.获取配置文件中信息
    ip = getServerIP();
    port = getServerPort();

    //2.实例化通信套接字对象,并绑定信号和槽
    client = new QTcpSocket(this);
    connect(client, &QTcpSocket::connected,
            this, &MainPage::clientConnectedSlot);
    connect(client, &QTcpSocket::readyRead,
            this, &MainPage::clientReadyReadSlot);

    setWindowTitle(QString("客户端-未连接"));

    //3.连接到服务器
    client->connectToHost(ip, port);
}

MainPage::~MainPage()
{
    delete ui;
}

void MainPage::setCarStart(int id, QString key, bool sw)
{
    QJsonObject obj;
    obj.insert("type",5001);
    obj.insert("id",id);
    obj.insert("key",key);
    obj.insert("sw",sw);

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    client->write(data);
}

void MainPage::setCarLock(int id, QString key, bool sw)
{
    QJsonObject obj;
    obj.insert("type",5004);
    obj.insert("id",id);
    obj.insert("key",key);
    obj.insert("sw",sw);

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    client->write(data);
}

void MainPage::setCarLight(int id, QString key, bool sw)
{
    QJsonObject obj;
    obj.insert("type",5002);
    obj.insert("id",id);
    obj.insert("key",key);
    obj.insert("sw",sw);

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    client->write(data);
}

void MainPage::setCarAlarmLight(int id, QString key, bool sw)
{
    QJsonObject obj;
    obj.insert("type",5003);
    obj.insert("id",id);
    obj.insert("key",key);
    obj.insert("sw",sw);

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    client->write(data);
}

//连接成功
void MainPage::clientConnectedSlot()
{
    setWindowTitle(QString("客户端-已连接"));

    QJsonObject obj;
    obj.insert("type", 1000);
    obj.insert("client_type", 3);
    client->write(QJsonDocument(obj).toJson());
}

//接收信息【固定代码】
void MainPage::clientReadyReadSlot()
{
    QByteArray data = client->readAll();
    int count = 0;
    for(int i = 0; i < data.length(); i++) {
        if(data.at(i) == '{')
            count++;
        else if(data.at(i) == '}') {
            count--;
            if(count == 0) {
                QByteArray temp = data.mid(0, i + 1);

                //具体协议解析
                //handleFrame(temp);

                data = data.mid(i+1);
                i = -1;
            }
        }
    }
}

void MainPage::handleFrame(const QByteArray &data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    int type = obj.value("type").toInt();

    qDebug() << "in handleFrame, type: " << type;

}

void MainPage::on_btnAllInfo_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageMap);
}

void MainPage::on_btnCarInfo_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageMap);
}

void MainPage::on_btnCrossingInfo_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageMap);
}

void MainPage::on_btnRoadInfo_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageMap);
}

void MainPage::on_btnCarSet_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageCar);
}

void MainPage::on_btnCrossingSet_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageCrossing);
}

void MainPage::on_btnSystemSet_clicked()
{
    ui->swMain->setCurrentWidget(ui->pageConfig);
}

//解析配置文件
QString MainPage::getServerIP()
{
    QSettings set(CONFIGFILENAME, QSettings::IniFormat);
    set.beginGroup(DEFGROUPNAME);
    if(!set.allKeys().contains(SERVERIP))
        set.setValue(SERVERIP, "127.0.0.1");
    QString ip = set.value(SERVERIP).toString();
    set.endGroup();
    return ip;
}

//解析配置文件
int MainPage::getServerPort()
{
    QSettings set(CONFIGFILENAME, QSettings::IniFormat);
    set.beginGroup(DEFGROUPNAME);
    if(!set.allKeys().contains(SERVERPORT))
        set.setValue(SERVERPORT, 10086);
    int port = set.value(SERVERPORT).toInt();
    set.endGroup();
    return port;
}


void MainPage::on_btnCarLock_clicked()
{
    setCarLock(0,ui->lineEditCar0Key->text(),true);
}

void MainPage::on_btnCarUnlock_clicked()
{
    setCarLock(0,ui->lineEditCar0Key->text(),false);
}

void MainPage::on_btnCarStart_clicked()
{
    setCarStart(0,ui->lineEditCar0Key->text(),true);
}

void MainPage::on_btnStop_clicked()
{
    setCarStart(0,ui->lineEditCar0Key->text(),false);
}

void MainPage::on_btnCarLightON_clicked()
{
    setCarLight(0,ui->lineEditCar0Key->text(),true);
}

void MainPage::on_btnCarLightOFF_clicked()
{
    setCarLight(0,ui->lineEditCar0Key->text(),false);
}

void MainPage::on_btnCarAralmLightON_clicked()
{
    setCarAlarmLight(0,ui->lineEditCar0Key->text(),true);
}

void MainPage::on_btnCarAralmLightOFF_clicked()
{
    setCarAlarmLight(0,ui->lineEditCar0Key->text(),false);
}
