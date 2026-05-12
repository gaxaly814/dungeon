#ifndef FACTORY_H
#define FACTORY_H
#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QSpinBox>
#include <QComboBox>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QTimer>
#include <QStyleFactory>
#include <QString>
#include <QHostAddress>
#include <QTcpSocket>
#include<QList>
class SimplePlaylist{
public:
    QList<QUrl> urls;//存储所有媒体文件的URL列表，播放列表容器
    int currentIndex;//当前播放的索引问题
    //构造函数
    SimplePlaylist():currentIndex(0){};
    //成员函数
    //添加媒体文件，通过文件路径添加
    void addMedia(const QString &filePath)
    {
        urls.append(QUrl::fromLocalFile(filePath));
    }
    //直接添加QUrl对象
    void addMedia(const QUrl& url){
        urls.append(url);
    }
    //获取当前媒体
    QUrl currentMedia()const{
        if(currentIndex>=0&&currentIndex<urls.size()){
            return urls[currentIndex];
        }
        return QUrl();
    }
    void next(){
        if(!urls.isEmpty())
        {
            currentIndex=(currentIndex+1)%urls.size();
        }
    }
    void previous(){
        if(!urls.isEmpty()){
            currentIndex=(currentIndex-1+urls.size())%urls.size();
        }
    }
    int size()const{
        return urls.size();
    }
    bool isEmpty()const{
        return urls.isEmpty();
    }
    void clear(){
        urls.clear();
        currentIndex=0;
    }
    //添加指定位置播放
    void playAt(int index){
        if(index>=0&&index<urls.size()){
            currentIndex=index;
        }
    }
};

class factory
{

public:
    QLabel* CreateQLabel(QWidget *pos, int x, int y, int w, int h,
                         QString Text="", QString StyleSheet=DefaultStyleSheet, QFont Font=DefaultFont, Qt::Alignment s=Qt::AlignCenter);
    QLabel* CreateQLabel(QMainWindow*pos,int x, int y, int w, int h,
                         QString Text="",QString StyleSheet=DefaultStyleSheet,QFont Font=DefaultFont,Qt::Alignment s=Qt::AlignCenter);
    QLabel* CreateQLabel(QWidget* pos,QString Text="",QString StyleSheet=DefaultStyleSheet,QFont Font=DefaultFont);
    QPushButton* CreateQPushButton(QWidget*pos,int x, int y, int w, int h,
                                   QString Text="",QString StyleSheet=DefaultStyleSheet,QFont Font=DefaultFont);
    QPushButton* CreateQPushButton(QMainWindow*pos,int x, int y, int w, int h,
                                   QString Text="",QString StyleSheet=DefaultStyleSheet,QFont Font=DefaultFont);
    QPushButton* CreateQPushButton(QWidget* pos,QString Text="",QString StyleSheet=DefaultStyleSheet);
    //进度条
    QProgressBar* CreateQProgressBar(QWidget*pos,int x, int y, int w, int h,bool TextVisible);
    //媒体播放器
    QMediaPlayer *CreateQMediaPlayer(QWidget*pos,QUrl url,int v);
    //播放列表
    SimplePlaylist* CreateQMediaPlaylist(QWidget* pos);
    //数字选择框
    QSpinBox* CreateQSpinBox(QWidget*pos,QString StyleSheet);
    //下拉选择框
    QComboBox* CreateQComboBox(QWidget* pos,int index);
    QLineEdit* CreateQLineEdit(QMainWindow*pos,int x, int y, int w,int h,QString StyleSheet,QFont Font);
    //创建网络连接
    QTcpSocket* CreateQTcpSocket(QString ServerIp, int ServerPort);
    QToolButton* CreateQToolButton(QString text,int w,int h,QString url,bool Enable);
private:
    static QString DefaultStyleSheet;//默认样式表
    static QFont DefaultFont;//默认字体
};

#endif // FACTORY_H
