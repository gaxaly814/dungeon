#ifndef DUNGEON_H
#define DUNGEON_H
#include <QLabel>
#include <QGridLayout>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QKeyEvent>
#include <QString>
#include <QVector>
#include <QStack>
#include <QQueue>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include "interface.h"
#include "factory.h"
#include "storewidget.h"
#include "sound.h"
#include "menu.h"
#include "help.h"
#include "sign.h"

class Dungeon;

extern QMainWindow *ptr_MianWindow;
extern interface* surface;
extern Dungeon* currentDungeon;
extern QGridLayout*gLayout_Map;
extern QWidget *MazeWidget[12][16];
extern QWidget *People;

struct character
{
    QString name;
    int level;
    int exp;
    int hp;
    int mp;
    int atk;
    int def;
    int scoreNum;
    int Pickaxe;
    int Downstairs;
};

struct Monster
{
    QString name;
    int hp;
    int atk;
    int def;
    int exp;
    int gold;
};

class Dungeon: public QWidget
{
    Q_OBJECT

public:
    explicit Dungeon(QWidget *parent = 0);
    ~Dungeon();

    // UI控件
    QWidget *infoWidget;                    // 状态显示界面
    QLineEdit *NameEdit;                    // 角色名输入框
    QPushButton *SetDone;                   // 确定按钮

    // 角色数据
    character role;
    Monster enemy[10];
    QString DungeonStytle[41][4];
    QString MonsterPic[20];                 // 怪物图片字符串
    QString CmptorPic[4][4];

    // 位置和楼层
    int currentFloor;                       // 当前楼层
    int x;                                  // 角色X坐标
    int y;                                  // 角色Y坐标
    int pre_x, pre_y, pre_floor;

    // 角色属性
    int MAX_HP;
    int MAX_MP;
    int deadTimes;
    bool isPre;
    bool isNext;
    bool PickaxeUse;
    bool isDead;

    // 迷雾系统
    QWidget *FogWidget[12][16];             // 战争迷雾窗口数组
    int FogArr[8][12][16];
    int haveVisited[8][12][16];

    // 公开方法
    void initialDugeon();
    void keyPressEvent(QKeyEvent *event);
    void pkRoundResult(bool localWin);

private slots:
    void GetitemWinshow();                  // 获得物品信息界面显示
    void FightWinshow();                    // 战斗界面显示
    void ShowdynamicEffect();               // 展示动态效果
    void OpenStore();                       // 打开商店
    void ReturnMainMenuPlay();              // 返回主菜单
    void RestartPlay();                     // 重新开始
    void ContinuePlay();                    // 继续游戏
    void ReturnRoomsPlay();                 // 返回游戏房间
    void HelpPlay();                        // 帮助界面
    void HelpReturnPlay();                  // 帮助界面返回
    void musicChangePlay();                 // 打开/关闭音乐
    void EnterDungeon();                    // 角色名设置完成,进入地牢
    void NetStart();
    void pkStart();
    void NetModeCountDownRun();
    void pkMapUpdate();
    void competitorGameOverCall();

private:
    // 初始化函数
    void initialMap();
    void initialpkMap();
    void initialCharacterPos();
    void initialFogWidget();
    void initialinfoWidget();               // 初始化状态界面
    void initialMapString();
    void initialMonsterPic();               // 初始化怪物图片（用于战斗界面）
    void initialCmptorPic();
    void initialrole();
    void initialenemy();
    void initialFogArr();
    void initialSetNameWin();
    void initialGetitemWin();               // 初始化获得物品信息界面
    void initialFightWin();                 // 初始化战斗界面

    // 显示函数
    void ShowDungeon();                     // 显示地牢
    void ShowFog();                         // 显示战争迷雾
    void hideFog();
    void hideDungeon();

    // 更新函数
    void updateStatusData();                // 更新状态数据

    // 判断函数
    bool isDynamic(int floor, int i, int j);
    bool isStatic(int num);

    // 移动和操作
    void Move();                            // 人物移动
    void Operation(int num);                // 移动操作
    void changeHP(int num);
    void changeMP(int num);
    void QuestionBox(int num);              // 问号箱
    void items(int _itemNum, int _moveDirection);   // 物品
    void Monsters(int _MonsterNum, int _moveDirection);
    void checkPrefloor();                   // 上一层
    void checkNextfloor();                  // 下一层
    void setCharacterPos(int floor, int target);
    void breakWall(int _moveDirection);
    void DownstairsRun();
    void deadCall();
    void GameOverCall();
    void stopAllwork();

    // 战斗相关
    int pk(Monster &m);
    int calc_damage(int MonsterNum);
    void pkMapFightWinShow();
    void ReturnDungeon(bool localWin);
    void pkOverCall(int MyPoint, int CmptorPoint);

    // 地图生成
    void RandomGeneraterMap(int arr[12][16], int floor);
    void GenerateElement(int arr[12][16], int floor);
    void GenerateMonster(int arr[12][16], int floor);
    void GenerateItem(int arr[12][16], int floor);
    void GenerateEquip();
    bool isValid(int arr[12][16], int i, int j, int floor);
    int sum(int arr[], int n);
    void doDfs(int arr[12][16], QStack<std::pair<int,int>>& s, bool visited[12][16]);
    void setExit(int arr[12][16], std::pair<int, int> &start, std::pair<int, int> &end);
    void dfs(int arr[12][16], bool visited[12][16], QStack<std::pair<int, int> > &s, int i, int j);
    void bfs(int arr[12][16], QStack<std::pair<int,int>> &s, std::pair<int, int> start, std::pair<int, int> end);

    // 迷雾
    void addFogArea(int floor, int i, int j, int num);
    void updateFogArea(int floor, int i, int j);

private:
    // 获得物品信息栏
    QLabel *GetitemsShow;
    QTimer *GetitemTimer;
    int GetitemNum;
    int itemNum;

    // 战斗界面
    QWidget *FightWidget;
    bool isFighting;
    QLabel *Monster_pic;        // 怪物信息标签
    QLabel *Monster_name;
    QLabel *Monster_hpText;
    QLabel *Monster_hp;
    QLabel *Monster_atkText;
    QLabel *Monster_atk;
    QLabel *Monster_defText;
    QLabel *Monster_def;

    QLabel *Character_pic;      // 人物信息标签
    QLabel *Character_name;
    QLabel *Character_hpText;
    QLabel *Character_hp;
    QLabel *Character_atkText;
    QLabel *Character_atk;
    QLabel *Character_defText;
    QLabel *Character_def;

    // 网络对战
    QLabel *NetMode_time;
    QLabel *NetMode_secondes;
    QLabel* label_MyName;
    QLabel* label_VS;
    QLabel* label_CmptorName;
    QLabel* label_MyPonit;
    QLabel* label_split;
    QLabel* label_CmptorPoint;
    QTimer *NetModeCountDownTimer;
    QWidget* competitor;
    int CountDown;
    int MyPonit;
    int CmptorPoint;
    bool NetModeStart;
    bool MyWin;
    bool dogFall;

    // 战斗状态
    int MonsterNum;             // 怪物编号
    int fight_period_it;        // 战斗次数
    int fight_end_it;           // 战斗结束标识

    // 动态效果
    QTimer *dynamicEffectTimer; // 动态效果计时器
    int display_it;

    // 音效
    Sound* music;

    // 移动
    int moveDirection;          // 移动方向
    int moveNum;

    // UI控件
    QLabel *Floor;              // 地牢层数
    QLabel *level;              // 人物状态信息
    QLabel *exp;
    QLabel *hp;
    QLabel *mp;
    QLabel *atk;
    QLabel *def;
    QLabel *score;              // 金币
    QLabel *PickaxeNum;         // 镐头数量
    QLabel *DownstairsNum;      // 下楼器数量
    QLabel *CharacterName;
    QLabel *FloorNum;

    // 其他
    StoreWidget *Store;
    Menu* menu;
    Help* help;
    factory* fac;
};

#endif // DUNGEON_H
