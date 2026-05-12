#include "interface.h"
#include <QPalette>
#include <QPixmap>
#include <QMovie>
#include <QFont>
#include <QDebug>
#include<QLabel>
#include<QTimer>
int msgNum=0;
QString msg[]={"加载中.","加载中..","加载中...","加载中...."};

interface::interface(QWidget*parent): QWidget(parent)
{
    fac=new factory();
    msgLabel=fac->CreateQLabel(this,380,440,100,75,"","color:white",QFont("Microsoft YaHei",13));
    msgLabel->hide();

    probar=fac->CreateQProgressBar(this,180,500,480,15,true);
    probar->hide();

    GifLabel=new QLabel(this);
    GifLabel->setScaledContents(true);//图片自动缩放填充整个标签

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(loading()));

    QuitTimer=new QTimer(this);
    connect(QuitTimer,SIGNAL(timeout()),this,SLOT(doQuit()));

    sign=new Sign();
    sign->resize(400,200);//不设置位置，默认居中显示
    sign->hide();

    QFont font("Microsoft YaHei", 20, 85);  // 字体：微软雅黑，20号，85(粗体)
    QString button_style =
        "QPushButton{background-color:transparent;color:papayawhip;border-radius:5px;}"
        "QPushButton:hover{background-color:palegreen;color:orange;}"
        "QPushButton:pressed{background-color:aquamarine;border-style:inset;}";

    //按钮创建与连接
    //联网对战按钮(x:340,y:190,宽：160，高：60）
    button_NetPlay=fac->CreateQPushButton(this,340,190,160,60,"联网对战",button_style,font);
    connect(button_NetPlay,SIGNAL(clicked()),this,SLOT(NetPlayStart()));
    //单机模式按钮
    button_LocalDungeon=fac->CreateQPushButton(this,340,270,160,60,"单机模式",button_style,font);
    connect(button_LocalDungeon,SIGNAL(clicked()),this,SLOT(LocalDungeonStart()));
    //离开游戏按钮
    button_Quit= fac->CreateQPushButton(this,340,350,160,60,"离开游戏",button_style,font);
    connect(button_Quit,SIGNAL(clicked()),this,SLOT(Quit()));
    //关于
    button_About= fac->CreateQPushButton(this,340,430,160,60,"关于",button_style,font);
    connect(button_About,SIGNAL(clicked()),this,SLOT(AboutShow()));
    //状态变量
    isok=false;//是否加载完成
    dungeonisok=false;//是否进入地牢模式
    surfaceShow=true;//是否显示主菜单
}

//窗口大小变化
void interface::resizeEvent(QResizeEvent*)
{
    GifLabel->clear();//清空原有内容
    //未加载完成，显示启动动画
    if(!isok)
    {
        QMovie*movie=new QMovie(":/interface/image/interface/StartPage.gif");
        GifLabel->setMovie(movie);
        movie->start();
    }
    //加载完成，显示设置名称界面背景
    else
    {
        GifLabel->setStyleSheet("border-image: url(:/interface/image/interface/SetNamePage.PNG);");
    }
    GifLabel->resize(this->size());
}

//显示主菜单
void interface::showMainMenu()
{
    button_NetPlay->show();
    button_LocalDungeon->show();
    button_Quit->show();
    button_About->show();
    surfaceShow=true;
}

//联网对战
void interface::NetPlayStart()
{
    sign->show();  // 显示登录/注册窗口
}

//单机模式
void interface::LocalDungeonStart()
{
    //隐藏所有菜单按钮
    button_NetPlay->hide();
    button_LocalDungeon->hide();
    button_Quit->hide();
    button_About->hide();

    //显示加载界面元素
    msgLabel->show();//提示消息标签
    probar->show();//进度条
    timer->start(100);//自动定时器（每100ms更新一次）；
    //标记进入地牢模式
    dungeonisok=true;
}

//加载动画逻辑
void interface::loading()//由timer的timeout信号触发
{
    //更新提示文字（循环显示4条消息）
    msgLabel->setText(msg[(msgNum++)%4]);
    //进度条增加20%
    probar->setValue(probar->value()+20);
    //加载完成（进度条>=99）
    if(probar->value()>=99)
    {
        timer->stop();          // 停止定时器
        msgLabel->hide();       // 隐藏提示
        probar->hide();         // 隐藏进度条
        isok = true;            // 标记加载完成
        probar->setValue(0);    // 重置进度条
        resizeEvent(nullptr);   // 触发重绘（切换到设置名称界面）
    }
}

//退出游戏（带确认对话框）
void interface::Quit()
{
    //创建确认对话框
    QMessageBox message(QMessageBox::Information, "退出游戏", "是否退出？",
                        QMessageBox::Yes | QMessageBox::No, this);
    message.setIconPixmap(QPixmap(":/info/image/information/退出.png"));
    message.setWindowIcon(QIcon(":/info/image/information/关于.ico"));

    //自定义按钮文字
    QPushButton *yesButton = message.addButton("确定", QMessageBox::YesRole);
    QPushButton *noButton = message.addButton("返回", QMessageBox::NoRole);

    //用户确认退出
    if(message.exec()==QMessageBox::Yes)
    {
        //如果聊天窗口可见，发送离线请求
        if(sign->roomWidget->isVisible())
        {
            sign->roomWidget->initialPlayerRequest();
            sign->roomWidget->doOfflineRequest();
        }
        QuitTimer->start(100);//自动退出计时器
    }
    else
        return;
}

void interface::doQuit()
{
    qApp->quit();//退出应用程序
    QuitTimer->stop();
}

//关于对话框
void interface::AboutShow()
{
    QMessageBox message(QMessageBox::Information, "关于",
                        "Written By Miao,in NanKai,2026.05",
                        QMessageBox::Yes, this);
    message.setIconPixmap(QPixmap(":/info/image/information/关于.ico"));
    message.button(QMessageBox::Yes)->hide();  // 隐藏"确定"按钮
    message.exec();  // 显示对话框（用户无法关闭，只能点X）
}





















