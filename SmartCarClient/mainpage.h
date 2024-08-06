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

    //车辆控制相关
    void setCarStart(int id, QString key, bool sw);
    void setCarLock(int id, QString key, bool sw);
    void setCarLight(int id, QString key, bool sw);
    void setCarAlarmLight(int id, QString key, bool sw);

protected:
    //配置文件解析
    QString getServerIP();
    int getServerPort();

    //接收消息处理方法
    void handleFrame(const QByteArray &data);

protected slots:
    //网络传输相关槽函数
    void clientConnectedSlot();
    void clientReadyReadSlot();

private slots:
    void on_btnAllInfo_clicked();

    void on_btnCarInfo_clicked();

    void on_btnCrossingInfo_clicked();

    void on_btnRoadInfo_clicked();

    void on_btnCarSet_clicked();

    void on_btnCrossingSet_clicked();

    void on_btnSystemSet_clicked();

    void on_btnCarLock_clicked();

    void on_btnCarUnlock_clicked();

    void on_btnCarStart_clicked();

    void on_btnStop_clicked();

    void on_btnCarAralmLightOFF_clicked();

    void on_btnCarLightON_clicked();

    void on_btnCarLightOFF_clicked();

    void on_btnCarAralmLightON_clicked();

private:
    Ui::MainPage *ui;

    QTcpSocket *client;
    QString ip;
    int port;
};

#endif // MAINPAGE_H
