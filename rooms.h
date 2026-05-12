#ifndef ROOMS_H
#define ROOMS_H

#include <QMainWindow>
#include <QToolButton>
#include <QDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QQueue>
#include "factory.h"
#include "chatroom.h"

class rooms: public QMainWindow
{
    Q_OBJECT
public:
    explicit rooms(QWidget *parents=0);
    QString playerName;
    QString owner_;
    ChatRoom * chatRoom;
    QPushButton * startGameBtn;

    // 设置TCP socket（从Sign传递过来）
    void setTcpSocket(QTcpSocket* socket);
    //网络请求函数
    void initialPlayerRequest(); //初始化玩家信息请求
    void doOfflineRequest();//发送离线请求

    void updatePlayerName(QString name);
    void setPlayerName(QString name) { playerName = name; if(playerLbl) playerLbl->setText(name); }


private:
    //界面初始化
    void initialMainWidget();
    void initialRoomPlayersWidget();//初始化房间内玩家列表界面
    void initialRoomaListBtn();//房间列表按钮
    void initialCreateroomDialog();// 初始化创建房间对话框
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent*);
    void flushRoomList();//向服务器请求刷新房间列表
    void doEnterRoom();
    void startRq();//房主点击“开始”时发送开始请求
    void beReady();
    void processServerMessage(QString msg);

private:
    factory* fac;
    QTcpSocket* tcpSocket;
    QLabel* playerLbl;//显示当前玩家名称
    QToolButton* emptyBtn;//房间为空时的提示按钮
    bool haveEmpty;
    bool haveinitial;
    bool newRoomCreated;
    bool LeaveDone;
    bool Ready;
    int initialSize;
    QToolButton* ownerTbt;//房主头像按钮
    QToolButton* playerTbt;//玩家
    QWidget* roomInfo;
    QWidget* roomPlayers;
    QWidget* roomsList;
    QString roomName;
    QTimer* flushRoomListTimer;
    QToolButton **roomsListButtons;
    QToolButton* newRoomBtn;
    QQueue<std::pair<QString,QString>> roomsListQueue;
    QVBoxLayout* ListLayout;
    QLineEdit* nameEdit;
    QDialog* CreateroomName;
private slots:
    void slotConnected();
    void dataReceived();
    void CreateRoom();
    void DeleteRoom();
    void doCreateRoom();
    void EnterRoom();
    void LeaveRoom();
    void returnRoomList();
    void GameStart();
    void CheckLeaveStatus();
};

#endif // ROOMS_H
