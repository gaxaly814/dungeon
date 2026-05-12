#include "dungeonsoundmanager.h"
#include <QDebug>

DungeonSoundManager::DungeonSoundManager(QObject *parent)
    : QObject(parent)
{
}

DungeonSoundManager::~DungeonSoundManager()
{
    // 注意：不删除 m_player，因为它由外部管理
}

void DungeonSoundManager::addMusic(const QString &filePath)
{
    m_playlist.append(filePath);

    // 如果是第一次添加音乐，自动设置为当前播放索引
    if (m_currentIndex == -1 && !m_playlist.isEmpty()) {
        m_currentIndex = 0;
    }
}

void DungeonSoundManager::setPlayer(QMediaPlayer *player)
{
    m_player = player;

    if (m_player) {
        // 连接信号槽，监听播放状态变化
        connect(m_player, &QMediaPlayer::mediaStatusChanged,
                this, &DungeonSoundManager::onMediaStatusChanged);
    }
}

void DungeonSoundManager::playAtIndex(int index)
{
    // 检查播放器是否已设置
    if (!m_player) {
        qDebug() << "Error: Player not set! Call setPlayer() first.";
        return;
    }

    // 检查索引是否有效
    if (index < 0 || index >= m_playlist.size()) {
        qDebug() << "Error: Invalid index" << index << ", playlist size:" << m_playlist.size();
        return;
    }

    // 保存当前索引
    m_currentIndex = index;

    // 获取音乐文件路径
    QString filePath = m_playlist[m_currentIndex];
    QUrl musicUrl = QUrl(filePath);

    // 设置并播放
    m_player->setSource(musicUrl);
    m_player->play();

    qDebug() << "Playing sound at index" << index << ":" << filePath;
}

void DungeonSoundManager::play()
{
    if (!m_player) {
        qDebug() << "Error: Player not set!";
        return;
    }

    if (m_playlist.isEmpty()) {
        qDebug() << "Warning: Playlist is empty!";
        return;
    }

    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size()) {
        m_currentIndex = 0;
    }

    // 设置并播放当前音乐
    QUrl musicUrl = QUrl::fromLocalFile(m_playlist[m_currentIndex]);
    m_player->setSource(musicUrl);
    m_player->play();

    qDebug() << "Now playing:" << m_playlist[m_currentIndex];
}

void DungeonSoundManager::pause()
{
    if (m_player) {
        m_player->pause();
    }
}

void DungeonSoundManager::stop()
{
    if (m_player) {
        m_player->stop();
    }
}

void DungeonSoundManager::next()
{
    if (m_playlist.isEmpty()) return;

    // 切换到下一首
    m_currentIndex = (m_currentIndex + 1) % m_playlist.size();
    play();
}

void DungeonSoundManager::previous()
{
    if (m_playlist.isEmpty()) return;

    // 切换到上一首
    m_currentIndex = (m_currentIndex - 1 + m_playlist.size()) % m_playlist.size();
    play();
}

void DungeonSoundManager::setVolume(int volume)
{
    if (m_player && m_player->audioOutput()) {
        m_player->audioOutput()->setVolume(volume / 100.0);
    }
}

void DungeonSoundManager::clear()
{
    m_playlist.clear();
    m_currentIndex = -1;
    stop();
}

void DungeonSoundManager::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // 当一首歌播放完成时，自动切换到下一首
    if (status == QMediaPlayer::EndOfMedia && !m_playlist.isEmpty()) {
        next();
    }
}
