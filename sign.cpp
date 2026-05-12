#include "sign.h"
#include<QDebug>
#include<QPushButton>
#include<QLabel>
#include<QLineEdit>
#include<QGridLayout>
#include<QMessageBox>

Sign::Sign(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("登录服务器"));
    setModal(true);
    setWindowModality(Qt::ApplicationModal);
    fac=new factory();
    tcpSocket = nullptr;
    isConnected = false;
    initialSignWidget();//初始化界面
    roomWidget = new rooms();//创建房间界面
    roomWidget->resize(840,680);
    roomWidget->hide();
}

//界面初始化函数
void Sign::initialSignWidget()
{
    //创造控件
    // 显式转换
    registerBtn = new QPushButton("注册", this);
    LogInBtn = new QPushButton("登录", this);
    nameLabel = new QLabel("用户名:", this);
    passwordLabel = new QLabel("密码:", this);
    nameLineEdit = new QLineEdit(this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);  // 密码模式（显示为***）


    connect(registerBtn, SIGNAL(clicked()), this, SLOT(slotRegister()));
    connect(LogInBtn, SIGNAL(clicked()), this, SLOT(slotLogIn()));

    //界面布局
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(nameLabel, 0, 0);
    mainLayout->addWidget(nameLineEdit, 0, 1, 1, 2);
    mainLayout->addWidget(passwordLabel, 1, 0);
    mainLayout->addWidget(passwordLineEdit, 1, 1, 1, 2);
    mainLayout->addWidget(registerBtn, 2, 1);
    mainLayout->addWidget(LogInBtn, 2, 2);
    statusLabel = new QLabel(tr("正在连接服务器..."), this);
    statusLabel->setStyleSheet("color:yellow;");
    mainLayout->addWidget(statusLabel, 3, 0, 1, 3);

    this->setLayout(mainLayout);


}

//注册功能
void Sign::slotRegister()
{
    if(!tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::information(this, tr("连接失败"), tr("尚未连接到服务器，请稍后重试。"));
        return;
    }
    username=nameLineEdit->text();
    //空用户名检查
    if(username=="")
    {
        QMessageBox::information(this,"无效用户名","用户名不能为空");
        return;
    }

    //非法字符检查
    if(username.contains('\t')||username.contains(':'))
    {
        QMessageBox::information(this,"无效用户名","用户名中包含非法字符");
        return;
    }

    //构造协议信息
    QString msg=REGISTER;//协议标识符
    QString password=passwordLineEdit->text();
    msg+=username+'\t'+password;
    msg+=ENDFLAG;//结束标志

    //发送到服务器
    int length=tcpSocket->write(msg.toUtf8(),msg.toUtf8().length());
    if(length!=msg.toUtf8().length())
    {
        QMessageBox::information(this, tr("发送失败"), tr("注册请求发送失败，请重试。"));
        return ;//发送失败
    }
    QMessageBox::information(this, tr("发送成功"), tr("注册请求已发送，等待服务器回应..."));


}

//登录功能
void Sign::slotLogIn()
{
    if(!tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::information(this, tr("连接失败"), tr("尚未连接到服务器，请稍后重试。"));
        return;
    }
    if(nameLineEdit->text()=="")
    {
        QMessageBox::information(this, "无效的用户名", "用户名不能为空！");
        return;
    }
    if(passwordLineEdit->text()=="")
    {
        QMessageBox::information(this, "无效的密码", "密码不能为空！");
        return;
    }

    QString msg=LOGIN;
    username = nameLineEdit->text();
    QString password = passwordLineEdit->text();
    msg += username + '\t' + password;
    msg+=ENDFLAG;

    int length = tcpSocket->write(msg.toUtf8(), msg.toUtf8().length());
    if(length != msg.toUtf8().length())
    {
        QMessageBox::information(this, tr("发送失败"), tr("登录请求发送失败，请重试。"));
        return;
    }
    QMessageBox::information(this, tr("发送成功"), tr("登录请求已发送，等待服务器回应..."));

}


void Sign::dataReceived()
{
    QByteArray datagram;
    datagram.resize(tcpSocket->bytesAvailable());
    tcpSocket->read(datagram.data(), datagram.size());
    QString msg = datagram.data();

    // 去掉结束标志 '-'
    if (msg.endsWith('-')) {
        msg.chop(1);
    }

    qDebug() << "sign msg:" << msg;

    if(msg == "LOG IN SUCCESS")
    {
        QMessageBox::information(this, "登录成功", "登录成功！");

        // 先断开 sign 的 readyRead 连接，避免重复处理
        disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));

        // 设置 socket 并连接 rooms 的信号槽
        roomWidget->setTcpSocket(tcpSocket);
        roomWidget->updatePlayerName(username);

        roomWidget->show();
        this->hide();
    }
    else if(msg == "password is wrong")
    {
        QMessageBox::information(this, "密码错误", "密码错误");
    }
    else if(msg == "REGISTER SUCCESS")
    {
        QMessageBox::information(this, "注册成功", "注册成功，现在可以登录了");
    }
    else if(msg == "You have to register first")
    {
        QMessageBox::information(this, "未注册", "请先注册");
    }
    else if(msg == "The name has been registered")
    {
        QMessageBox::information(this, "用户名已存在", "用户名已被注册");
    }
    else if(msg == "another client have Login this acount")
    {
        QMessageBox::information(this, "账号已登录", "该账号已在其他地方登录");
    }
    else
    {
        qDebug() << "未匹配的消息:" << msg;
    }
}

void Sign::socketConnected()
{
    isConnected = true;
    statusLabel->setText(tr("已连接到服务器"));
}

void Sign::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    isConnected = false;
    statusLabel->setText(tr("连接服务器失败"));
}

void Sign::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    this->setFocus();
    this->raise();
    this->activateWindow();

    // 如果已有socket，先删除
    if(tcpSocket)
    {
        tcpSocket->disconnectFromHost();
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
    }

    tcpSocket = fac->CreateQTcpSocket(SERVER_IP, SERVER_PORT);
    tcpSocket->setParent(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(tcpSocket, &QTcpSocket::connected, this, &Sign::socketConnected);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &Sign::socketError);
    isConnected = false;
    statusLabel->setText(tr("正在连接服务器..."));
}




















