#ifndef INTERFACE_H
#define INTERFACE_H

#include <QPainter>
#include <QMovie>
#include <QResizeEvent>
#include <QMessageBox>
#include <QApplication>
#include<QLabel>
#include<QTimer>
#include "factory.h"
#include "sign.h"
class interface : public QWidget
{
    Q_OBJECT
public:
    explicit interface(QWidget *parent=0);
    void resizeEvent(QResizeEvent*);//当窗口大小改变时自动调用，实现自适应布局

public:
    Sign*sign;
    QTimer *timer;
    QPushButton *button_NetPlay;//网络游戏按钮
    QPushButton * button_LocalDungeon;// 本地地牢按钮
    QPushButton *button_Quit;//退出
    QPushButton *button_About;//关于
    bool isok;
    bool dungeonisok;
    bool surfaceShow;
    void showMainMenu();//显示主菜单

private:
    factory*fac;
    QLabel*msgLabel;
    QLabel*GifLabel;
    QProgressBar* probar;
    QPainter* painter;
    QTimer* QuitTimer;

public slots:
    void NetPlayStart();//启动网络游戏
    void LocalDungeonStart();//启动本地游戏
    void loading();//加载过程
    void Quit();
    void doQuit();
    void AboutShow();
};

#endif // INTERFACE_H
