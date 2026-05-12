#include "rooms.h"

rooms::rooms(QWidget *parent) : QMainWindow(parent)
{
    fac = new factory();
    haveEmpty = false;
    haveinitial = false;
    newRoomCreated = false;
    LeaveDone = false;
    Ready = false;
    initialSize = 0;
    tcpSocket = nullptr;  // 初始化socket指针

    flushRoomListTimer = new QTimer(this);
    connect(flushRoomListTimer, SIGNAL(timeout()), this, SLOT(CheckLeaveStatus()));
    initialMainWidget();
}

//主界面布局
void rooms::initialMainWidget()
{
    //创建主容器
    QWidget* mainWidget=new QWidget(this);
    //创建中间+左侧容器
    QWidget* roomWidget=new QWidget(mainWidget);
    roomWidget->setMinimumSize(640,640);
    QHBoxLayout* h_roomWidgetLayout=new QHBoxLayout();
    h_roomWidgetLayout->setContentsMargins(0,0,0,0);
    h_roomWidgetLayout->setSpacing(0);
    roomWidget->setLayout(h_roomWidgetLayout);

    //创建房间信息区（中间，初始隐藏）
    roomInfo=new QWidget(mainWidget);
    roomInfo->setMinimumSize(640,640);
    roomInfo->hide();
    QVBoxLayout* vLayout=new QVBoxLayout();
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    roomInfo->setLayout(vLayout);

    //创建玩家头像区
    roomPlayers=new QWidget(roomInfo);
    roomPlayers->setMinimumSize(640,340);
    initialRoomPlayersWidget();

    //创建聊天室
    chatRoom=new ChatRoom(roomInfo);
    chatRoom->setMinimumSize(640,300);

    //组装房间信息区
    vLayout->addWidget(roomPlayers);
    vLayout->addWidget(chatRoom);
    h_roomWidgetLayout->addWidget(roomInfo);

    //创建房间列表区（左侧）
    roomsList=new QWidget(mainWidget);
    roomsList->setMinimumSize(640,640);
    ListLayout=new QVBoxLayout();
    ListLayout->setContentsMargins(0,0,0,0);
    ListLayout->setSpacing(0);
    ListLayout->setAlignment(Qt::AlignTop);
    roomsList->setLayout(ListLayout);
    h_roomWidgetLayout->addWidget(roomsList,0);

    //创建右侧信息栏
    QWidget* infoWidget=new QWidget(mainWidget);
    infoWidget->setStyleSheet("background-color:papayawhip");
    infoWidget->setMinimumSize(200,640);
    QString infoWidget_style="color:black;background-color:white";
    playerLbl=fac->CreateQLabel(infoWidget,0,320,120,40,"",infoWidget_style);
    QPushButton* playerBtn=fac->CreateQPushButton(infoWidget,0,380,120,40,"创建房间",infoWidget_style);
    QPushButton* deleteRoomBtn=fac->CreateQPushButton(infoWidget,0,440,120,40,"删除房间",infoWidget_style);
    initialCreateroomDialog();
    connect(playerBtn,SIGNAL(clicked()),this,SLOT(CreateRoom()));
    connect(deleteRoomBtn,SIGNAL(clicked()),this,SLOT(DeleteRoom()));

    //组装主布局
    QHBoxLayout* h_mainLayout=new QHBoxLayout();
    h_mainLayout->setContentsMargins(0,0,0,0);
    h_mainLayout->setSpacing(0);
    mainWidget->setLayout(h_mainLayout);
    h_mainLayout->addWidget(roomWidget);
    h_mainLayout->addWidget(infoWidget);

    this->setCentralWidget(mainWidget);
}

//初始化房间内玩家显示区域
void rooms::initialRoomPlayersWidget()
{
    ownerTbt=new QToolButton(roomPlayers);
    ownerTbt->setGeometry(100,95,150,150);
    ownerTbt->setIcon(QPixmap(":/info/image/information/MarioLeft.png"));
    ownerTbt->setIconSize(QPixmap(":/info/image/information/MarioLeft.png").size());
    ownerTbt->setAutoRaise(true);
    ownerTbt->setEnabled(true);
    ownerTbt->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    playerTbt=new QToolButton(roomPlayers);
    playerTbt->setGeometry(390,95,150,150);
    playerTbt->setIcon(QPixmap(":/info/image/information/MarioRight.png"));
    playerTbt->setIconSize(QPixmap(":/info/image/information/MarioRight.png").size());
    playerTbt->setAutoRaise(true);
    playerTbt->setEnabled(true);
    playerTbt->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QFont font_VS("Microsoft YaHei",24,75);
    QString VS_style="color:orangered;background-color:transparent;";
    QLabel *VS=fac->CreateQLabel(roomPlayers,280,130,100,80,"VS",VS_style,font_VS);

    QString button_style="QPushButton{background-color:white;color:black;border-radius:5px;}"
                           "QPushButton:hover{background-color:palegreen; color: orangered;}"
                           "QPushButton:pressed{background-color:aquamarine;border-style:inset;}";

    QPushButton* returnRoomListBtn=fac->CreateQPushButton(roomPlayers,100,290,120,40,"房间列表",button_style);
    startGameBtn=fac->CreateQPushButton(roomPlayers,390,290,120,40,"",button_style);
    connect(returnRoomListBtn,SIGNAL(clicked()),this,SLOT(returnRoomList()));
    connect(startGameBtn,SIGNAL(clicked()),this,SLOT(GameStart()));
}

//初始化房间列表按钮
void rooms::initialRoomaListBtn()
{
    if(haveEmpty)
    {
        emptyBtn->hide();
        delete emptyBtn;
        haveEmpty=false;
    }
    if(haveinitial && initialSize!=0)
    {
        for(int i=0;i<initialSize;++i)
        {
            delete roomsListButtons[i];
        }
        delete [] roomsListButtons;
        haveinitial=false;
        initialSize=0;
    }
    if(newRoomCreated)
    {
        newRoomBtn->hide();
        delete newRoomBtn;
        newRoomCreated=false;
    }
    if(!roomsListQueue.isEmpty())
    {
        roomsListButtons=new QToolButton *[roomsListQueue.size()];
        initialSize=roomsListQueue.size();
        haveinitial=true;
    }
    int Num=0;
    while(!roomsListQueue.isEmpty())
    {
        QString name=roomsListQueue.front().first;
        QString owner=roomsListQueue.front().second;
        roomsListQueue.dequeue();
        QString text=name+" 房主: "+owner;
        roomsListButtons[Num]=fac->CreateQToolButton(text,640,40,":/info/image/information/房间.png",true);
        connect(roomsListButtons[Num],SIGNAL(clicked()),this,SLOT(EnterRoom()));
        ListLayout->addWidget(roomsListButtons[Num]);
        Num++;
    }
}

//创建房间对话框初始化
void rooms::initialCreateroomDialog()
{
    CreateroomName=new QDialog();
    QLabel* nameLabel = new QLabel("房间名:",CreateroomName);
    nameEdit=new QLineEdit(CreateroomName);
    QPushButton* EnterBtn = new QPushButton("确定",CreateroomName);
    QGridLayout* gLayout=new QGridLayout();
    gLayout->addWidget(nameLabel,0,0);
    gLayout->addWidget(nameEdit,0,1);
    gLayout->addWidget(EnterBtn,1,1);
    CreateroomName->setLayout(gLayout);
    connect(EnterBtn,SIGNAL(clicked()),this,SLOT(doCreateRoom()));
}

void rooms::CreateRoom()
{
    CreateroomName->show();
}

void rooms::doCreateRoom()
{
    if(nameEdit->text()=="")
    {
        QMessageBox::information(this, "创建失败", "房间名不能为空");
        CreateroomName->raise();
        return;
    }
    if(nameEdit->text().contains('\t')||nameEdit->text().contains(':'))
    {
        QMessageBox::information(this, "无效的用户名", "用户名中包含非法字符！");
        return;
    }
    QString msg = CREATEROOM;
    msg+=nameEdit->text()+'\t'+playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::DeleteRoom()
{
    QString msg = DELETEROOM;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::EnterRoom()
{
    QToolButton* curBtn = qobject_cast<QToolButton*>(sender());
    QString text = curBtn->text();
    QString str = " 房主: ";
    int index = text.indexOf(str, 0);
    roomName = text.mid(0, index);
    owner_ = text.mid(index + 5);

    qDebug() << "========== EnterRoom ==========";
    qDebug() << "按钮文字:" << text;
    qDebug() << "房间名:" << roomName;
    qDebug() << "房主名:" << owner_;

    chatRoom->owner_ = owner_;

    QString msg = ENTERROOM;
    msg += roomName + '\t' + playerName;
    msg += ENDFLAG;
    qDebug() << "发送进入房间消息:" << msg;

    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::slotConnected()
{
    QString msg = ENTERROOM;
    msg+=roomName+'\t'+playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::doEnterRoom()
{
    qDebug() << "doEnterRoom - roomName:" << roomName;
    qDebug() << "doEnterRoom - owner_:" << owner_;
    qDebug() << "doEnterRoom - playerName:" << playerName;

    // 将 socket 传递给 chatRoom
    if(chatRoom) {
        chatRoom->setTcpSocket(tcpSocket);
        chatRoom->playerName = playerName;
        chatRoom->owner_ = owner_;
    }

    if(owner_ == playerName)
    {
        startGameBtn->setText("开始");
        ownerTbt->setText(owner_);
        playerTbt->setText("");
    }
    else
    {
        startGameBtn->setText("准备");
        playerTbt->setText(playerName);
        ownerTbt->setText("");
    }

    roomInfo->show();
    roomsList->hide();
}

void rooms::LeaveRoom()
{
    QString msg = LEAVEROOM;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::CheckLeaveStatus()
{
    if(LeaveDone)
    {
        flushRoomList();
        flushRoomListTimer->stop();
        LeaveDone=false;
    }
}

void rooms::returnRoomList()
{
    LeaveRoom();
    flushRoomListTimer->start(10);
    roomInfo->hide();
    chatRoom->contentListWidget->clear();
    roomsList->show();
}

void rooms::startRq()
{
    QString msg = STARTRQ;
    msg+=playerName+'\t'+roomName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::beReady()
{
    Ready=!Ready;
    QString ready_str;
    if(Ready)
    {
        startGameBtn->setText("取消准备");
        ready_str="ready";
    }
    else
    {
        startGameBtn->setText("准备");
        ready_str="unready";
    }
    QString msg = READYRQ;
    msg+=playerName+'\t'+roomName+'\t'+ready_str;
    msg+=ENDFLAG;
    qDebug()<<"ready msg:"<<msg;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::GameStart()
{
    if(owner_ == playerName)
        startRq();
    else
        beReady();
}

void rooms::initialPlayerRequest()
{
    if(playerName != owner_)
    {
        QString msg = INITPLAYERRQ;
        msg += playerName;
        msg += ENDFLAG;
        int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
        if(length != msg.toUtf8().length())
            return;
    }
}

void rooms::doOfflineRequest()
{
    QString msg = OFFLINERQ;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

// ========== 设置TCP socket（从Sign传入）==========
void rooms::setTcpSocket(QTcpSocket* socket)
{
    qDebug() << "rooms::setTcpSocket called";
    tcpSocket = socket;

    // 直接连接信号槽（不要 disconnect，因为 sign 已经断开了）
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()));

    // 延迟一点刷新房间列表，确保连接稳定
    QTimer::singleShot(100, this, [this]() {
        flushRoomList();
    });
}
// ========== 处理服务器消息（拆分多条消息）==========
void rooms::dataReceived()
{
    if (!tcpSocket) return;

    QByteArray datagram;
    datagram.resize(tcpSocket->bytesAvailable());
    tcpSocket->read(datagram.data(), datagram.size());
    QString fullMsg = datagram.data();

    qDebug() << "rooms raw msg:" << fullMsg;

    // 按 '-' 分割多条消息
    QStringList messages = fullMsg.split('-', Qt::SkipEmptyParts);

    for (QString msg : messages) {
        qDebug() << "rooms processing:" << msg;
        processServerMessage(msg);
    }
}

// ========== 处理单条服务器消息 ==========
void rooms::processServerMessage(QString msg)
{
    if(msg == "rooms empty!")
    {
        emptyBtn = fac->CreateQToolButton("(空)",640,40,":/info/image/information/房间.png",false);
        ListLayout->addWidget(emptyBtn);
        haveEmpty = true;
    }
    else if(msg == "duplicated name!")
    {
        QMessageBox::information(this, "创建失败", "房间名已被占用");
        nameEdit->clear();
        CreateroomName->raise();
    }
    else if(msg == "you already have a room!")
    {
        QMessageBox::information(this, "创建失败", "你已经创建了一个房间了!");
        nameEdit->clear();
        CreateroomName->hide();
    }
    else if(msg == "No Room!")
    {
        QMessageBox::information(this, "删除房间失败", "你还没有创建房间");
    }
    else if(msg == "Enter Refused!")
    {
        QMessageBox::information(this, "进入房间失败", "房主不能进入他人房间,可以删除自己的房间后进入");
    }
    else if(msg == "room is full")
    {
        QMessageBox::information(this, "房间已满员", "房间已满员");
    }
    else if(msg == "CraeteRoom done!")
    {
        if(haveEmpty)
        {
            haveEmpty = false;
            emptyBtn->hide();
            delete emptyBtn;
        }
        QString text = nameEdit->text() + " 房主: " + playerName;
        newRoomBtn = fac->CreateQToolButton(text,640,40,":/info/image/information/房间.png",true);
        connect(newRoomBtn, SIGNAL(clicked()), this, SLOT(EnterRoom()));
        ListLayout->addWidget(newRoomBtn);
        nameEdit->clear();
        CreateroomName->hide();
        newRoomCreated = true;
    }
    else if(msg == "DeleteRoom done!")
    {
        qDebug() << "房间删除成功";
    }
    else if(msg.mid(0,9) == "EnterRoom")
    {
        QString text = msg.mid(9);

        // 按 '\t' 分割
        QStringList parts = text.split('\t');

        qDebug() << "EnterRoom 解析 - 原始:" << text;
        qDebug() << "分割后数量:" << parts.size();

        if(parts.size() >= 3) {
            // 新格式：房间名\t房主名\t玩家名
            QString roomNameFromServer = parts[0];
            QString owner = parts[1];
            QString player = parts[2];

            qDebug() << "房间名:" << roomNameFromServer << "房主:" << owner << "玩家:" << player;

            // 保存房间名（重要！）
            roomName = roomNameFromServer;
            owner_ = owner;

            doEnterRoom();
            ownerTbt->setText(owner);
            playerTbt->setText(player);
        }
        else if(msg == "游戏开始")
        {
            qDebug() << "游戏开始！";

        }
        else if(parts.size() >= 2) {
            // 旧格式：房主名\t玩家名（兼容）
            QString owner = parts[0];
            QString player = parts[1];

            qDebug() << "旧格式 - 房主:" << owner << "玩家:" << player;

            owner_ = owner;

            doEnterRoom();
            ownerTbt->setText(owner);
            playerTbt->setText(player);
        }
    }
    else if(msg.mid(0,9) == "LeaveRoom")
    {
        QString player = msg.mid(9);
        if(playerTbt->text() == player)
            playerTbt->setText("");
        else if(ownerTbt->text() == player)
            ownerTbt->setText("");
        LeaveDone = true;
    }
    else if(msg == "FRL")
    {
        flushRoomList();
    }
    else if(msg == "游戏开始")
    {
        qDebug() << "游戏开始！";
    }
    else if(msg.contains('#') && msg.contains(':'))
    {
        // 解析房间列表数据，格式: "1#房间名1:房主名1#房间名2:房主名2#"
        roomsListQueue.clear();

        // 找到第一个 # 的位置，跳过房间数量
        int firstHash = msg.indexOf('#');
        QString content = msg.mid(firstHash + 1);

        // 移除末尾可能多余的 #
        if (content.endsWith('#')) {
            content.chop(1);
        }

        qDebug() << "房间列表内容:" << content;

        // 按 "#" 分割多个房间
        QStringList roomsPart = content.split('#', Qt::SkipEmptyParts);

        for (QString roomPart : roomsPart) {
            QString name = roomPart.section(':', 0, 0);
            QString owner = roomPart.section(':', 1, 1);

            qDebug() << "解析到房间 - 名:" << name << "房主:" << owner;

            if(!name.isEmpty() && !owner.isEmpty()) {
                roomsListQueue.push_back(std::pair<QString, QString>(name, owner));
            }
        }

        qDebug() << "最终房间数量:" << roomsListQueue.size();
        initialRoomaListBtn();
    }

    if(chatRoom)
    {
        chatRoom->handleServerMessage(msg);
    }
}

void rooms::flushRoomList()
{
    if (!tcpSocket) return;
    QString msg = FLUSHROOMLIST;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void rooms::showEvent(QShowEvent *)
{
    // 只更新界面显示，不创建连接（连接由setTcpSocket设置）
    if (playerLbl) {
        playerLbl->setText(playerName);
    }
    if (chatRoom) {
        chatRoom->playerName = playerName;
    }
}

void rooms::closeEvent(QCloseEvent*)
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        initialPlayerRequest();
        doOfflineRequest();
    }
}

void rooms::updatePlayerName(QString name)
{
    playerName = name;
    if(playerLbl) {
        playerLbl->setText(name);
    }
    if(chatRoom) {
        chatRoom->playerName = name;
    }
}




























