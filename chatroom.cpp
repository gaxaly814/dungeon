#include "chatroom.h"
#include "dungeon.h"

ChatRoom::ChatRoom(QWidget *parent):QWidget(parent)
{
    msgSize=0;
    headGet=false;
    pkStartGet=false;
    fac=new factory();
    initialChatRoomWidget();

}
//界面初始化
void ChatRoom::initialChatRoomWidget()
{
    contentListWidget=new QListWidget(this);  //显示聊天信息的列表
    sendLineEdit=new QLineEdit(this);//输入框
    sendBtn=new QPushButton("发送");//发送按钮
    QGridLayout*mainLayout=new QGridLayout(this);
    mainLayout->addWidget(contentListWidget,0,0,1,3);
    mainLayout->addWidget(sendLineEdit,1,0,1,2);
    this->setLayout(mainLayout);

    connect(sendBtn,&QPushButton::clicked,this,&ChatRoom::slotSend);
}

//发送聊天信息
void ChatRoom::slotSend()
{
    if(sendLineEdit->text()=="")
    {
        return;
    }
    QString msg=MESSAGE;
    msg+=playerName+'\t'+":"+sendLineEdit->text();
    msg+=ENDFLAG;
    tcpSocket->write(msg.toUtf8(),msg.toUtf8().length());//发送的数据转换为utf-8格式（网络字节序，中文也能识别
    sendLineEdit->clear();
}

//连接成功时的处理,TCP链接建立后，自动发送一条“进入聊天室”的系统信息
void ChatRoom::slotConnected()
{
    int length=0;
    QString msg = MESSAGE;
    msg += playerName +'\t'+ ": Enter ChatRoom";
    msg+=ENDFLAG;
    qDebug()<<msg;
    if((length = tcpSocket->write(msg.toUtf8(),msg.toUtf8(). length())) != msg.toUtf8().length())
    {
        return;
    }
}
void ChatRoom::mapRequest()
{
    QString msg = MAPRQ;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}
void ChatRoom::initialReadyRquest()
{
    QString msg = INITREQDYRQ;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}
void ChatRoom::pkRequest()
{
    QString msg = PKRQ;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
    if (!pkStartTimer->isActive())
        pkStartTimer->start(100);
}
void ChatRoom::udpkPosRequest(int x,int y,int dir)
{
    QString msg = UDPKPOSRQ;
    msg+=playerName+'\t'+QString::number(x)+'#'+QString::number(y)+'\t'+QString::number(dir);
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}
void ChatRoom::udpkStatusRequest(int hp, int atk, int def)
{
    QString msg = UDPKSTATUSRQ;
    msg+=playerName+'\t'+QString::number(hp)+'#'+QString::number(atk)+'\t'+QString::number(def);
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}
void ChatRoom::FightRequest()
{
    if (!FightTimer->isActive())
        FightTimer->start(50);
    QString msg = FIGHTRQ;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}
void ChatRoom::GameOverRequest()
{
    QString msg = GAMEOVERRQ;
    msg+=playerName;
    msg+=ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

void ChatRoom::pkResultRequest(const QString &winnerName)
{
    QString msg = PKRESRQ;
    msg += winnerName;
    msg += ENDFLAG;
    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
        return;
}

//数据接收，实现了一个简单的TCP粘包处理协议
void ChatRoom::dataReceived()
{
    QByteArray datagram;
    datagram.resize(tcpSocket->bytesAvailable());
    tcpSocket->read(datagram.data(), datagram.size());
    recv.append(datagram.constData(), datagram.size());

    QString recvStr = QString::fromUtf8(recv.data(), static_cast<int>(recv.size()));
    QStringList messages = recvStr.split(ENDFLAG, Qt::KeepEmptyParts);
    int completeCount = recvStr.endsWith(ENDFLAG) ? messages.size() : messages.size() - 1;

    for (int i = 0; i < completeCount; ++i) {
        QString msg = messages[i];
        if (msg.isEmpty())
            continue;
        handleServerMessage(msg);
    }

    if (!recvStr.endsWith(ENDFLAG) && !messages.isEmpty()) {
        QString leftover = messages.last();
        recv.assign(leftover.toUtf8().constData(), leftover.toUtf8().size());
    } else {
        recv.clear();
    }
}

void ChatRoom::handleServerMessage(const QString &msg)
{
    qDebug() << "ChatRoom received:" << msg;

    if (msg == "游戏开始") {
        qDebug() << "ChatRoom 收到游戏开始消息！";
        if (!NetStartTimer->isActive())
            NetStartTimer->start(100);
    }
    else if (msg.startsWith("PRQ")) {
        qDebug() << "ChatRoom 收到 PK 请求:" << msg;
        if (!pkStartGet)
            pkStartTimer->start(100);
        pkStartGet = true;
    }
    else if (msg.startsWith("FRQ")) {
        qDebug() << "ChatRoom 收到 战斗请求:" << msg;
        qDebug() << "此处由对方发起PK，本地等待结果，不再本地同步战斗。";
    }
    else if (msg.startsWith("KRQ")) {
        QString Name = msg.mid(3);
        qDebug() << "ChatRoom 收到 PK 结果:" << Name;
        if (surface && surface->sign && surface->sign->roomWidget && currentDungeon) {
            bool localWin = (Name == playerName);
            currentDungeon->pkRoundResult(localWin);
        }
    }
    else if (msg.startsWith("GRQ")) {
        QString Name = msg.mid(3);
        qDebug() << "ChatRoom 收到 GameOver 请求:" << Name;
        if (Name != playerName)
            CmptorGameOverCallTimer->start(1);
    }
    else if (msg.startsWith("URQ")) {
        QString data = msg.mid(3);
        QString Name = data.section('\t', 0, 0);
        QString pos_str = data.section('\t', 1, 1);
        int dir = data.section('\t', 2, 2).toInt();
        if (Name != playerName) {
            competitoName = Name;
            cur_x = pos_str.section('#', 0, 0).toInt();
            cur_y = pos_str.section('#', 1, 1).toInt();
            this->dir = dir;
            qDebug() << "收到对手位置更新:" << competitoName << cur_x << cur_y << "dir=" << dir;
            pkMapUdTimer->start(1);
        }
    }
    else if (msg.startsWith("USQ")) {
        QString data = msg.mid(3);
        QString Name = data.section('\t', 0, 0);
        QString status_str = data.section('\t', 1, 1);
        if (Name != playerName) {
            competitoName = Name;
            hp = status_str.section(',', 0, 0).toInt();
            atk = status_str.section(',', 1, 1).toInt();
            def = status_str.section(',', 2, 2).toInt();
        }
    }
    else if (msg.startsWith("pkPosUpdate")) {
        QString data = msg.mid(11);
        QString Name = data.section('#', 0, 0);
        QString pos_str = data.section('#', 1, 1);
        if (Name != playerName) {
            competitoName = Name;
            cur_x = pos_str.section(',', 0, 0).toInt();
            cur_y = pos_str.section(',', 1, 1).toInt();
            dir = pos_str.section(',', 2, 2).toInt();
            pkMapUdTimer->start(1);
        }
    }
    else if (msg.startsWith("pkStaUpdate")) {
        QString data = msg.mid(11);
        QString Name = data.section('#', 0, 0);
        QString status_str = data.section('#', 1, 1);
        if (Name != playerName) {
            competitoName = Name;
            hp = status_str.section(',', 0, 0).toInt();
            atk = status_str.section(',', 1, 1).toInt();
            def = status_str.section(',', 2, 2).toInt();
        }
    }
    else if (msg.startsWith("MES")) {
        QString content = msg.mid(3);
        contentListWidget->addItem(content);
        contentListWidget->scrollToBottom();
    }
}

void ChatRoom::showEvent(QShowEvent *)
{
    // 如果已经有 socket（从 rooms 传入），就不要重新创建
    if(tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "ChatRoom: 使用已有 socket 连接";
        if (!externalTcpSocket) {
            connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()), Qt::UniqueConnection);
            connect(tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()), Qt::UniqueConnection);
        }

        // 如果不是房主，发送准备就绪请求
        if(playerName != "@" && owner_ != "" && playerName != owner_)
        {
            initialReadyRquest();
        }
        return;
    }

    // 如果没有 socket，才创建新连接（兼容旧逻辑）
    tcpSocket = fac->CreateQTcpSocket(SERVER_IP, SERVER_PORT);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()));

    if(playerName != "@" && owner_ != "" && playerName != owner_)
    {
        initialReadyRquest();
    }
}

void ChatRoom::closeEvent(QCloseEvent*)
{

}

void ChatRoom::hideEvent(QHideEvent*)
{

}































