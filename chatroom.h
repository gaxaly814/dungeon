#ifndef CHATROOM_H
#define CHATROOM_H


#include <QWidget>
#include <QListWidget>
#include <QGridLayout>
#include <QTimer>
#include "request.h"
#include "factory.h"

extern int ***map; //三维数组
extern int Total_Floor;//总楼层数
extern QTimer*NetStartTimer;//网络启动计时器
extern QTimer*pkStartTimer;//pk开始计时器
extern QTimer*pkMapUdTimer;//pk地图更新计时器
extern QTimer*FightTimer;//战斗计时器
extern QTimer*CmptorGameOverCallTimer;//游戏结束调用计时器

class ChatRoom:public QWidget
{
    Q_OBJECT
public:
    explicit ChatRoom(QWidget *parent = 0);
    QString playerName;
    QString owner_;
    QString competitoName;
    QListWidget *contentListWidget;    //显示聊天内容的列表控件

    void pkRequest(); //发起pk请求
    void udpkPosRequest(int x, int y, int dir); //更新并发送自己的位置、方向
    void udpkStatusRequest(int hp, int atk, int def);//更新并发送自己的战斗状态
    void FightRequest();//发起战斗请求
    void GameOverRequest(); //游戏结束请求
    void pkResultRequest(const QString &winnerName); //PK结果同步请求
    void resetPkStartFlag() { pkStartGet = false; }
    void handleServerMessage(const QString &msg);

    int cur_x;
    int cur_y;
    int dir;
    int hp;
    int atk;
    int def;
    void setTcpSocket(QTcpSocket* socket) {
        tcpSocket = socket;
        externalTcpSocket = true;
    }


private:
    factory *fac;
    QLineEdit *sendLineEdit;
    QPushButton *sendBtn;
    QTcpSocket *tcpSocket;//TCP套接字，用于网络通信
    bool externalTcpSocket = false;
    std::string recv;//接收数据的字符串缓冲区
    unsigned int msgSize;//消息总大小
    bool headGet;//是否以获取消息头
    bool pkStartGet;//是否以获取pk开始信息

private:
    void showEvent(QShowEvent*);//重写窗口显示
    void closeEvent(QCloseEvent*);
    void hideEvent(QHideEvent*);
    void initialChatRoomWidget();//初始化聊天室界面
    void mapRequest();//请求地图数据
    void initialReadyRquest();//初始化准备就绪请求

private slots:
    void slotConnected();//TCP链接成功时的处理
    void slotSend();//发送聊天信息
    void dataReceived();//接受网络数据（解析协议）
};

#endif // CHATROOM_H
