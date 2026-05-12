#include "sound.h"
#include <QDebug>
#include <QUrl>

Sound::Sound(QWidget *parent) :
    QWidget(parent)
{
    fac = new factory();

    // 创建播放器
    //界面背景音乐（菜单主界面）
    InterfaceBGM = fac->CreateQMediaPlayer(this, QUrl("qrc:/music/music/interface_main_theme.mp3"), 50);
    //地牢背景音乐（游戏进行时
    DungeonBGM = fac->CreateQMediaPlayer(this, QUrl("qrc:/music/music/main_theme.mp3"), 50);
    //战斗音效
    FightSound = fac->CreateQMediaPlayer(this, QUrl("qrc:/music/music/brick_smash.mp3"), 80);

    // 创建地牢音效播放器，播放地牢中的各种短音效
    DungeonSound = fac->CreateQMediaPlayer(this, QUrl(""), 0);

    // 创建播放列表管理器
    m_soundManager = new DungeonSoundManager(this);
    m_soundManager->setPlayer(DungeonSound);  //将管理器与播放器关联

    // 添加地牢音效到播放列表
    // 根据CurrentIndex索引播放不同的音效
    // Index 0-2：普通音效（播放1秒）
    // Index 3：短促音效（播放400ms）
    // Index 4：普通音效（播放1秒）
    // Index 5：长音效（播放3秒）

    m_soundManager->addMusic("qrc:/music/brick_smash.mp3");    // Index 0
    m_soundManager->addMusic("qrc:/music/coin.mp3");           // Index 1
    m_soundManager->addMusic("qrc:/music/one_up.mp3");         // Index 2
    m_soundManager->addMusic("qrc:/music/stomp.mp3");          // Index 3 - 短促
    m_soundManager->addMusic("qrc:/music/powerup.mp3");        // Index 4
    m_soundManager->addMusic("qrc:/music/death.mp3");          // Index 5 - 长音效

    // 创建定时器
    InterfaceBGMTimer = new QTimer(this);//界面背景音乐状态检查定时器
    DungeonBGMTimer = new QTimer(this);//地牢
    FightSoundTimer = new QTimer(this);//战斗音效
    DungeonSoundTimer = new QTimer(this);//地牢音效停止定时器

    // 连接信号槽
    //定时器每个一段时间触发，检查界面背景音乐是否停止，若停止则重新播放
    connect(InterfaceBGMTimer, SIGNAL(timeout()), this, SLOT(CheckInterfaceBGMState()));
    connect(DungeonBGMTimer, SIGNAL(timeout()), this, SLOT(CheckDungeonBGMState()));
    connect(FightSoundTimer, SIGNAL(timeout()), this, SLOT(CheckFightSoundState()));
    //定时器到达指定时间后触发，停止地牢音效（用于控制短音效的播放时长）
    connect(DungeonSoundTimer, SIGNAL(timeout()), this, SLOT(SoundStop()));

    // 启动定时器
    //每10ms检查一次播放状态
    InterfaceBGMTimer->start(10);
    DungeonBGMTimer->start(10);
    FightSoundTimer->start(10);

    musicChange = true;

    // 播放界面背景音乐
    if (InterfaceBGM) {
        InterfaceBGM->play();
    }
}

void Sound::CheckInterfaceBGMState()
{
    if (InterfaceBGM && InterfaceBGM->playbackState() == QMediaPlayer::StoppedState) {
        InterfaceBGM->play();
    }
}

void Sound::CheckDungeonBGMState()
{
    if (DungeonBGM && DungeonBGM->playbackState() == QMediaPlayer::StoppedState) {
        DungeonBGM->play();
    }
}

void Sound::CheckFightSoundState()
{
    if (FightSound && FightSound->playbackState() == QMediaPlayer::StoppedState) {
        FightSound->play();
    }
}

void Sound::SoundStop()
{
    if (DungeonSound) {
        DungeonSound->stop();
    }
    if (DungeonSoundTimer) {
        DungeonSoundTimer->stop();
    }
}

void Sound::SoundPlay(int CurrentIndex)
{
    //安全检查：确保管理器、播放器和音频输出都存在
    if (!m_soundManager || !DungeonSound) return;

    // 使用管理器播放指定索引的音效
    m_soundManager->playAtIndex(CurrentIndex);

    // 根据不同的音效设置不同的停止时间，防止重复播放
    //短促音效：400ms后停止（捡起道具、跳跃、快速攻击等）
    if (CurrentIndex == 3) {
        DungeonSoundTimer->start(400);
    }
    //长音效：3s,开门、过关动画、Boss出现
    else if (CurrentIndex == 5) {
        DungeonSoundTimer->start(3000);
    }
    //普通：1s
    else {
        DungeonSoundTimer->start(1000);
    }
}

void Sound::setMusicMuted(bool muted)
{
    //设置地牢背景音乐的静音状态
    if (DungeonBGM && DungeonBGM->audioOutput()) {
        DungeonBGM->audioOutput()->setMuted(muted);
    }
}

void Sound::setMasterVolume(int volume)
{
    float vol = volume / 100.0f;
    if (InterfaceBGM && InterfaceBGM->audioOutput()) {
        InterfaceBGM->audioOutput()->setVolume(vol);
    }
    if (DungeonBGM && DungeonBGM->audioOutput()) {
        DungeonBGM->audioOutput()->setVolume(vol);
    }
    if (FightSound && FightSound->audioOutput()) {
        FightSound->audioOutput()->setVolume(vol);
    }
}
