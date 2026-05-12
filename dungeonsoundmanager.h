#ifndef DUNGEONSOUNDMANAGER_H
#define DUNGEONSOUNDMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class DungeonSoundManager : public QObject
{
    Q_OBJECT

public:
    explicit DungeonSoundManager(QObject *parent = nullptr);
    ~DungeonSoundManager();

    // 添加音乐到播放列表
    void addMusic(const QString &filePath);

    // 设置播放器（需要外部传入）
    void setPlayer(QMediaPlayer *player);

    // 播放音乐
    void play();

    //播放指定索引的音效
    void playAtIndex(int index);

    // 暂停
    void pause();

    // 停止
    void stop();

    // 下一首
    void next();

    // 上一首
    void previous();

    // 设置音量 (0-100)
    void setVolume(int volume);

    // 获取当前播放索引
    int currentIndex() const { return m_currentIndex; }

    // 获取播放列表大小
    int size() const { return m_playlist.size(); }

    // 清空播放列表
    void clear();

private slots:
    // 当媒体状态改变时自动调用（用于自动切换下一首）
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QMediaPlayer *m_player = nullptr;      // 播放器指针（不负责创建和销毁）
    QStringList m_playlist;                 // 音乐文件路径列表
    int m_currentIndex = -1;                // 当前播放的索引
    bool m_looping = true;                  // 是否循环播放
};

#endif // DUNGEONSOUNDMANAGER_H
