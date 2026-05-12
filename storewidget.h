#ifndef STOREWIDGET_H
#define STOREWIDGET_H

#include <QGraphicsView>
#include <QPushButton>
#include <QTimer>
#include <QKeyEvent>
#include <QShowEvent>
#include "factory.h"
class StoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StoreWidget(QWidget *parent = 0);
    int chooseOption; //选择了第几项
    int BuyTimes;
    QTimer *haveBuyTimer;//购买计时器
    bool setChooseEnable;//是否可以购买
    bool StoreWinisShow;//商店窗口是否显示
    int Currstore_price;//当前价格
    int Nextstore_price;//下一次价格
    void initialStoreDatas();//初始化商店数据
private:
    int OptionNum; //有多少个选项
    int border_color_it;//边框颜色索引
    factory *fac;
    //边框颜色数组（实现彩虹闪烁效果）
    QString OptionBoxColor[6]= {"FFFFFF", "CCCCCC", "999999", "666666", "999999", "CCCCCC"};
    QTimer *OptionBoxTimer;//选项框闪烁定时器
    QGraphicsView *OptionBox;
    QLabel *text1;
    QLabel *text2;
    QPushButton *hpButton;
    QPushButton *mpButton;
    QPushButton *atkButton;
    QPushButton *defButton;
    QPushButton *exitButton;
private:
    void initialStoreWin();//初始化商店界面
    void keyPressEvent(QKeyEvent *event);//当前类中的键盘事件与mainwindow中的键盘事件互不影响
    void showEvent(QShowEvent *);//重写窗口显示事件,为了使当前窗口获得焦点
    void processSelectedOption();
public slots:
    void OptionBoxborderChanged();//选项框边框颜色变化（闪烁效果）
    void onOptionButtonClicked();
};

#endif
