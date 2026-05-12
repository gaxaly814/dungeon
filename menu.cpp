//游戏菜单界面
#include "menu.h"

Menu::Menu(QWidget *parent) :
    QWidget(parent)
{
    fac=new factory();
    MenuWinShow=false;
    //样式表
    //普通：透明  番木瓜色  圆角5px
    //悬停：淡绿  橙色  圆角
    //按下：海蓝  橙色  内陷
    QString button_style="QPushButton{background-color:transparent;color:papayawhip;border-radius:5px;}"
                           "QPushButton:hover{background-color:palegreen; color: orangered;}"
                           "QPushButton:pressed{background-color:aquamarine;border-style:inset;}";
    //界面
    QString MenuWidget_style="color:orangered;background-color:transparent";
    QFont fontLabel("Microsoft YaHei" ,12, 75);
    QFont fontPauseEvent("Microsoft YaHei",14,100);
    QWidget* MenuWidget=new QWidget(this);
    MenuWidget->setStyleSheet(MenuWidget_style);

    pauseEvent=fac->CreateQLabel(MenuWidget,60,50,400,100,"PAUSE",MenuWidget_style,fontPauseEvent);
    ReturnMainMenu=fac->CreateQPushButton(MenuWidget,200,160,120,30,"返回主菜单",button_style,fontLabel);
    Restart=fac->CreateQPushButton(MenuWidget,200,200,120,30,"重新开始",button_style,fontLabel);
    Continue=fac->CreateQPushButton(MenuWidget,200,240,120,30,"继续游戏",button_style,fontLabel);
    ReturnRooms=fac->CreateQPushButton(MenuWidget,200,200,120,30,"返回房间",button_style,fontLabel);
    ReturnRooms->hide();//初始隐藏
    Help=fac->CreateQPushButton(MenuWidget,200,280,120,30,"帮助",button_style,fontLabel);

    music=fac->CreateQPushButton(MenuWidget,300,40,30,30,"",
                                   "border-image: url(:/info/image/information/music-on.png);");

    // 按钮点击事件的连接
    connect(Continue, SIGNAL(clicked()), this, SLOT(hide()));           // 继续游戏
    connect(Restart, SIGNAL(clicked()), this, SLOT(onRestart()));       // 重新开始
    connect(ReturnMainMenu, SIGNAL(clicked()), this, SLOT(onReturnToMain())); // 返回主菜单
    connect(Help, SIGNAL(clicked()), this, SLOT(showHelp()));           // 显示帮助
    connect(music, SIGNAL(clicked()), this, SLOT(toggleMusic()));       // 切换音乐



}
void Menu::showEvent(QShowEvent *)
{
    MenuWinShow=true;
}
void Menu::hideEvent(QHideEvent*)
{
    MenuWinShow=false;
}
