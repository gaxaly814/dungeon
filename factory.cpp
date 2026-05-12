#include "factory.h"
QString factory::DefaultStyleSheet="color:white;backgroung-color:black";
QFont factory::DefaultFont=QFont("Microsoft YaHei",12,10);
QLabel* factory::CreateQLabel(QWidget *pos, int x, int y, int w, int h,
                               QString Text, QString StyleSheet, QFont Font, Qt::Alignment s)
{
    QLabel*p=new QLabel(pos);
    p->setGeometry(x,y,w,h);//设置标签的位置和大小
    p->setText(Text);
    p->setStyleSheet(StyleSheet);//设置样式表，可以改变标签的外观（颜色、字体、边框等）
    p->setFont(Font);
    p->setAlignment(s);//设置文本对齐方式
    return p;
}
QLabel* factory::CreateQLabel(QMainWindow*pos,int x, int y, int w, int h,
                              QString Text,QString StyleSheet,QFont Font,Qt::Alignment s)
{
    QLabel*p=new QLabel(pos);
    p->setGeometry(x,y,w,h);//设置标签的位置和大小
    p->setText(Text);
    p->setStyleSheet(StyleSheet);//设置样式表，可以改变标签的外观（颜色、字体、边框等）
    p->setFont(Font);
    p->setAlignment(s);//设置文本对齐方式
    return p;
}

QLabel* factory::CreateQLabel(QWidget *pos, QString Text, QString StyleSheet, QFont Font)
{
    QLabel* p=new QLabel(pos);
    p->setText(Text);
    p->setStyleSheet(StyleSheet);
    p->setFont(Font);
    return p;
}

QPushButton* factory::CreateQPushButton(QWidget*pos,int x, int y, int w, int h,
                                        QString Text,QString StyleSheet,QFont Font)
{
    QPushButton* p=new QPushButton(pos);
    p->setGeometry(x,y,w,h);
    p->setText(Text);
    p->setStyleSheet(StyleSheet);
    p->setFont(Font);
    return p;
}
QPushButton* factory::CreateQPushButton(QMainWindow*pos,int x, int y, int w, int h,
                                        QString Text,QString StyleSheet,QFont Font)
{
    QPushButton* p=new QPushButton(pos);
    p->setGeometry(x,y,w,h);
    p->setText(Text);
    p->setStyleSheet(StyleSheet);
    p->setFont(Font);
    return p;
}

QPushButton* factory::CreateQPushButton(QWidget* pos,QString Text,QString StyleSheet)
{
    QPushButton* p=new QPushButton(pos);
    p->setText(Text);
    p->setStyleSheet(StyleSheet);
    return p;
}

QProgressBar*factory::CreateQProgressBar(QWidget*pos,int x,int y,int w,int h,bool TextVisible)
{
    QProgressBar*p=new QProgressBar(pos);
    p->setGeometry(x,y,w,h);
    p->setStyle(QStyleFactory::create("fusion"));//将界面风格设置为“Fusion"样式
    p->setTextVisible(TextVisible);//设置进度条上的文本是否显示
    return p;
}

QMediaPlayer* factory::CreateQMediaPlayer(QWidget* pos, QUrl url, int v)
{
    // 1. 创建播放器
    QMediaPlayer* p = new QMediaPlayer(pos);

    // 2. 设置媒体源
    p->setSource(url);

    // 3. 创建音频输出对象
    QAudioOutput* audioOutput = new QAudioOutput(p);

    // 4. 将音频输出设置给播放器
    p->setAudioOutput(audioOutput);

    // 5. 设置音量（0-100 转换为 0.0-1.0）
    float volume = qBound(0.0f, v / 100.0f, 1.0f);
    audioOutput->setVolume(volume);

    return p;
}

SimplePlaylist*factory::CreateQMediaPlaylist(QWidget*pos)
{
    Q_UNUSED(pos);
    SimplePlaylist* playlist = new SimplePlaylist();

    // 添加游戏音效到播放列表
    playlist->addMedia("qrc:/music/music/coin.mp3");      // 索引0: 硬币音效
    playlist->addMedia("qrc:/music/music/powerup.mp3");   // 索引1: 升级音效
    playlist->addMedia("qrc:/music/music/flagpole.mp3");  // 索引2: 旗子音效
    playlist->addMedia("qrc:/music/music/stomp.mp3");     // 索引3: 钥匙音效
    playlist->addMedia("qrc:/music/music/one_up.mp3");    // 索引4: 蘑菇音效
    playlist->addMedia("qrc:/music/music/death.mp3");     // 索引5: 死亡音效
    return playlist;

}

QSpinBox* factory::CreateQSpinBox(QWidget*pos,QString StyleSheet)
{
    QSpinBox*p=new QSpinBox(pos);
    p->setStyleSheet(StyleSheet);
    return p;
}


QComboBox* factory::CreateQComboBox(QWidget* pos,int index)
{
    QComboBox* p=new QComboBox(pos);
    p->addItem(QObject::tr("魔法少女"));//项目编号从0开始
    p->addItem(QObject::tr("火影忍者"));
    p->addItem(QObject::tr("超级玛丽"));
    p->setCurrentIndex(index);
    return p;
}
QLineEdit* factory::CreateQLineEdit(QMainWindow*pos,int x, int y, int w,int h,QString StyleSheet,QFont Font)
{
    QLineEdit* p=new QLineEdit(pos);
    p->setGeometry(x,y,w,h);
    p->setStyleSheet(StyleSheet);
    p->setFont(Font);
    return p;
}
//连接到制定IP和端口的服务器
QTcpSocket* factory::CreateQTcpSocket(QString ServerIp, int ServerPort)
{
    QTcpSocket* tcpSocket = new QTcpSocket();

    // 直接在栈上创建，避免内存泄漏
    QHostAddress serverIP;
    serverIP.setAddress(ServerIp);

    // 连接到服务器（异步）
    tcpSocket->connectToHost(serverIP, ServerPort);

    // 连接信号，处理连接结果
    QObject::connect(tcpSocket, &QTcpSocket::connected, [tcpSocket]() {
        qDebug() << "连接成功！";
    });

    QObject::connect(tcpSocket, &QTcpSocket::errorOccurred, [tcpSocket]() {
        qDebug() << "连接失败：" << tcpSocket->errorString();
    });

    return tcpSocket;
}
QToolButton* factory::CreateQToolButton(QString text,int w,int h,QString url,bool Enable)
{
    QToolButton* p=new QToolButton();
    p->setText(text);
    p->setMinimumSize(w,h);
    p->setIcon(QPixmap(url));
    p->setIconSize(QPixmap(url).size());
    p->setAutoRaise(true);//设置按钮在鼠标悬停时自动凸起显示
    p->setEnabled(Enable);//设置鼠标是否可用（可点击）
    p->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);//设置工具按钮上图标和文字的排列方式
    return p;
}
