#include "storewidget.h"
# include<QDebug>

StoreWidget::StoreWidget(QWidget *parent):QWidget(parent)
{
    //初始化商店界面
    initialStoreWin();
}

void StoreWidget::initialStoreWin()
{
    QWidget *StoreWin=new QWidget(this);
    QFont font("Microsoft YaHei" ,12, 30);
    this->setFont(font);
    StoreWin->setStyleSheet("color:white;background-color:black");

    fac=new factory();
    //创建商店图标
    QLabel *Store_pic=fac->CreateQLabel(StoreWin,10,10,30,30,"","border-image: url(:/info/image/information/Store.png);");
    text1=fac->CreateQLabel(StoreWin,50,10,192,30,"你若给我 40 个金币");
    text2=fac->CreateQLabel(StoreWin,50,50,192,30,"我就可以帮你");
    //选项
    hpButton=new QPushButton("提升 100 点生命", StoreWin);
    hpButton->setGeometry(32,112,192,32);
    hpButton->setStyleSheet("color:white;background-color:transparent;border:none;text-align:left;padding-left:8px;");
    hpButton->setCursor(Qt::PointingHandCursor);

    mpButton=new QPushButton("提升 20 点魔法", StoreWin);
    mpButton->setGeometry(32,160,192,32);
    mpButton->setStyleSheet("color:white;background-color:transparent;border:none;text-align:left;padding-left:8px;");
    mpButton->setCursor(Qt::PointingHandCursor);

    atkButton=new QPushButton("提升 2 点攻击力", StoreWin);
    atkButton->setGeometry(32,208,192,32);
    atkButton->setStyleSheet("color:white;background-color:transparent;border:none;text-align:left;padding-left:8px;");
    atkButton->setCursor(Qt::PointingHandCursor);

    defButton=new QPushButton("提升 4 点防御力", StoreWin);
    defButton->setGeometry(32,256,192,32);
    defButton->setStyleSheet("color:white;background-color:transparent;border:none;text-align:left;padding-left:8px;");
    defButton->setCursor(Qt::PointingHandCursor);

    exitButton=new QPushButton("离开商店", StoreWin);
    exitButton->setGeometry(32,304,192,32);
    exitButton->setStyleSheet("color:white;background-color:transparent;border:none;text-align:left;padding-left:8px;");
    exitButton->setCursor(Qt::PointingHandCursor);

    //创建选项选择框
    OptionBox=new QGraphicsView(StoreWin);//最后构造的控件会在最上面
    OptionBox->setStyleSheet("border:3px solid #FFFFFF;");
    OptionBox->setGeometry(32,112,192,32);
    OptionBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    initialStoreDatas();
    OptionBoxTimer = new QTimer;
    haveBuyTimer=new QTimer(this);

    connect(OptionBoxTimer, SIGNAL(timeout()), this, SLOT(OptionBoxborderChanged()));
    connect(hpButton, &QPushButton::clicked, this, &StoreWidget::onOptionButtonClicked);
    connect(mpButton, &QPushButton::clicked, this, &StoreWidget::onOptionButtonClicked);
    connect(atkButton, &QPushButton::clicked, this, &StoreWidget::onOptionButtonClicked);
    connect(defButton, &QPushButton::clicked, this, &StoreWidget::onOptionButtonClicked);
    connect(exitButton, &QPushButton::clicked, this, &StoreWidget::onOptionButtonClicked);
    OptionBoxTimer->start(100);
}

//初始化
void StoreWidget::initialStoreDatas()
{
    border_color_it=0;
    OptionNum = 5;//选项数
    chooseOption = 0;//选择的选项
    BuyTimes=1;//购买次数
    Currstore_price=40;
    StoreWinisShow=false;
    setChooseEnable=false;
}

void StoreWidget::OptionBoxborderChanged()
{
    //边框颜色索引循环
    if (border_color_it <= 4)
        border_color_it++;
    else
        border_color_it = 0;
    //实现选项框边缘颜色的变化（闪烁效果）

    //样式表 setStyleSheet 的组合用法  QString重载了 '+' 号运算符
    OptionBox->setStyleSheet(QString("border:3px solid #") +
                             QString(OptionBoxColor[border_color_it]) + QString(";background-color:transparent;"));
    //OptionBox的背景也为黑色，会遮挡住下面的控件，所以要设置背景为透明
}

void StoreWidget::onOptionButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button)
        return;

    if (button == hpButton)
        chooseOption = 0;
    else if (button == mpButton)
        chooseOption = 1;
    else if (button == atkButton)
        chooseOption = 2;
    else if (button == defButton)
        chooseOption = 3;
    else if (button == exitButton)
        chooseOption = 4;

    OptionBox->setGeometry(32, 112 + 48 * chooseOption, 192, 32);
    processSelectedOption();
}

void StoreWidget::processSelectedOption()
{
    if (chooseOption != OptionNum - 1)//不是选择的离开商店选项
    {
        //计算价格：[(购买次数)^2-(购买次数）+2]*20
        Currstore_price = ((BuyTimes) * (BuyTimes) - (BuyTimes) + 2) * 20;
        //显示下次购买价格
        Nextstore_price = ((BuyTimes + 1) * (BuyTimes + 1) - (BuyTimes + 1) + 2) * 20;

        haveBuyTimer->start(100);//开启购买
        if(!setChooseEnable)//金币不够，则不能购买
        {
            return;
        }

        //更新第一行提示文字：显示下次购买
        text1->setText(QString::fromStdWString(L"你若给我 ") +
                       QString::number(Nextstore_price) + QString::fromStdWString(L" 个金币"));
        text2->setText(QString::fromStdWString(L"我就可以帮你"));
        hpButton->setText(QString::fromStdWString(L"提升 ") +
                          QString::number(100 * (BuyTimes + 1)) + QString::fromStdWString(L" 点生命"));
        mpButton->setText(QString::fromStdWString(L"提升 ") +
                          QString::number(20 * (BuyTimes + 1)) + QString::fromStdWString(L" 点魔法"));

        BuyTimes++;

        return;
    }
    else
    {
        //离开商店
        this->hide();
        StoreWinisShow=false;
        chooseOption=0;//选项框回到起始位置
        OptionBox->setGeometry(32,112,192,32);
        return;
    }
}

//处理玩家在商店中的键盘输入
void StoreWidget::keyPressEvent(QKeyEvent *event)
{
    //上移
    if(event->key() == Qt::Key_W||event->key()==Qt::Key_Up)
    {
        if (chooseOption > 0)
        {
            chooseOption -= 1;
            OptionBox->setGeometry(32, 112 + 48 * chooseOption, 192, 32);
        }
        else
            chooseOption = 0;
    }
    //下移
    else if (event->key() == Qt::Key_S||event->key()==Qt::Key_Down)
    {
        if (chooseOption <= OptionNum - 2)
        {
            chooseOption += 1;
            OptionBox->setGeometry(32, 112 + 48 * chooseOption, 192, 32);
        }
        else
            chooseOption = OptionNum - 1;
    }
    //确定
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space)
    {
        processSelectedOption();
    }
}

//设置焦点到商店窗口，使键盘输入有效
void StoreWidget::showEvent(QShowEvent *)
{
    this->setFocus();
}






























