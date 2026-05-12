#ifndef SOUND_H
#define SOUND_H

#include <QWidget>
#include <QMediaPlayer>
#include <QTimer>
#include "factory.h"
#include "dungeonsoundmanager.h"

class Sound : public QWidget
{
    Q_OBJECT

public:
    explicit Sound(QWidget *parent = 0);

    void SoundPlay(int CurrentIndex);
    void setMusicMuted(bool muted);
    void setMasterVolume(int volume);

private slots:
    void CheckInterfaceBGMState();
    void CheckDungeonBGMState();
    void CheckFightSoundState();
    void SoundStop();

public:
    factory* fac;

    // 播放器
    QMediaPlayer *InterfaceBGM;
    QMediaPlayer *DungeonBGM;
    QMediaPlayer *FightSound;
    QMediaPlayer *DungeonSound;

    // 播放列表管理器
    DungeonSoundManager *m_soundManager;

    // 定时器
    QTimer *InterfaceBGMTimer;
    QTimer *DungeonBGMTimer;
    QTimer *FightSoundTimer;
    QTimer *DungeonSoundTimer;

    bool musicChange;
};

#endif // SOUND_H

