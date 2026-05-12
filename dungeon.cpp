#include "dungeon.h"
#include<QDebug>
#include<QTime>

//方向控制
std::vector<std::vector<int>> directions{{0,-1},{0,1},{-1,0},{1,0}};
int***map;//三维数组地图
int Total_Floor;
int currentFloor;
//网络对战相关定时器
QTimer *NetStartTimer;//网络开始定时器
QTimer *pkStartTimer;
QTimer *pkMapUdTimer;
QTimer *FightTimer;
QTimer *CmptorGameOverCallTimer;
Dungeon* currentDungeon = nullptr;

Dungeon::Dungeon(QWidget*parent):QWidget(parent)
{
    currentDungeon = this;
    Total_Floor=7;
    //开辟堆上三维数组
    map=new int **[Total_Floor+1];
    for(int i=0;i<Total_Floor+1;i++)
    {
        map[i]=new int *[12];
        for(int j=0;j<12;j++)
        {
            map[i][j]=new int[16];
            for(int k=0;k<16;k++)
            {
                map[i][j][k]=0;//初始化为空地
            }
        }
    }
    //初始化随机数种子
    QTime time=QTime::currentTime();//获取当前时间
    srand(time.msec()+time.second()*1000);

    //初始化游戏各模块
    initialFogArr();
    initialMapString();
    initialMonsterPic();
    initialCmptorPic();
    initialrole();
    initialenemy();

    //初始化游戏状态变量
    //获得物品信息栏初始化
    GetitemNum=0;
    itemNum=0;
    MonsterNum=0;
    //战斗状态信息栏初始化
    fight_period_it = 0;
    fight_end_it = 0;
    moveDirection=-1;
    moveNum=0;
    pre_x=1;
    pre_y=1;
    pre_floor=1;
    MyPonit=0;
    CmptorPoint=0;
    CountDown=80;
    NetModeStart=false;
    MyWin=false;
    dogFall=false;
    isFighting=false;
    display_it=0;
    PickaxeUse=false;
    isDead=false;
    fac=new factory();
    music=new Sound();
    NetStartTimer=new QTimer;
    pkStartTimer=new QTimer;
    pkMapUdTimer=new QTimer;
    CmptorGameOverCallTimer=new QTimer;
    connect(NetStartTimer,SIGNAL(timeout()),this,SLOT(NetStart()));
    connect(pkStartTimer,SIGNAL(timeout()),this,SLOT(pkStart()));
    connect(pkMapUdTimer,SIGNAL(timeout()),this,SLOT(pkMapUpdate()));
    connect(CmptorGameOverCallTimer,SIGNAL(timeout()),this,SLOT(competitorGameOverCall()));
}
Dungeon::~Dungeon(){
    if (currentDungeon == this)
        currentDungeon = nullptr;
}

//人物数据初始化
void Dungeon::initialrole()
{
    role.name="Mario";
    role.level=1;
    role.exp=0;
    role.hp=100;
    role.mp=100;
    role.atk=5;
    role.def=3;
    role.scoreNum=0;
    role.Pickaxe=0;
    role.Downstairs=0;
}

//怪物数据初始化
void Dungeon::initialenemy()
{
    enemy[0].name="小星星";
    enemy[0].hp=60;
    enemy[0].atk=4;
    enemy[0].def=1;
    enemy[0].exp=4;
    enemy[0].gold=5;

    enemy[1].name="毒蘑菇";
    enemy[1].hp=100;
    enemy[1].atk=9;
    enemy[1].def=5;
    enemy[1].exp=8;
    enemy[1].gold=10;

    enemy[2].name="绿鸭子";
    enemy[2].hp=200;
    enemy[2].atk=16;
    enemy[2].def=12;
    enemy[2].exp=16;
    enemy[2].gold=20;

    enemy[3].name="红鸭子";
    enemy[3].hp=300;
    enemy[3].atk=24;
    enemy[3].def=14;
    enemy[3].exp=32;
    enemy[3].gold=30;

    enemy[4].name="食人花-绿";
    enemy[4].hp=600;
    enemy[4].atk=32;
    enemy[4].def=20;
    enemy[4].exp=42;
    enemy[4].gold=40;

    enemy[5].name="食人花-红";
    enemy[5].hp=900;
    enemy[5].atk=46;
    enemy[5].def=22;
    enemy[5].exp=52;
    enemy[5].gold=46;

    enemy[6].name="飞行鸭";
    enemy[6].hp=1200;
    enemy[6].atk=82;
    enemy[6].def=42;
    enemy[6].exp=64;
    enemy[6].gold=60;

}

//迷雾系统初始化
void Dungeon::initialFogArr()
{
    memset(FogArr,0,sizeof(FogArr));
    //设置迷雾区域
    for(int i=0;i<2;++i)
    {
        for(int j=6;j<9;++j)
        {
            FogArr[3][i][j]=15;//第三层迷雾强度设为15
        }
    }
    //初始化已访问数组（记录玩家到过的位置）
    memset(haveVisited,0,sizeof(haveVisited));
}

//动态贴图方式（根据地块类型动态设置地图格子的背景图片）
void Dungeon::initialMapString()
{
    for(int i=0;i<41;i++)
    {
        for(int j=0;j<4;j++)
        {
            //判断是否为静态地板（不会变化的地形）
            if(isStatic(i))
            {
                //静态地块：图片直接放在Dungeon目录下
                DungeonStytle[i][j]=QString("border-image: url(:/Dungeon/image/Dungeon/")+QString::number(i)+QString(".png);");
                break;
            }
            else
            {
                //动态地块：放在以数字命名的子目录下
                DungeonStytle[i][j]=QString("border-image: url(:/Dungeon/image/Dungeon/")+
                                      QString::number(i)+"/"+QString::number(i)+QString::number(j)+QString(".png);");
            }
        }
    }
}

//初始化怪物图片
void Dungeon::initialMonsterPic()
{
    MonsterPic[0]="border-image: url(:/info/image/information/start.png);";
    MonsterPic[1]="border-image: url(:/info/image/information/badmushroom.png);";
    MonsterPic[2]="border-image: url(:/info/image/information/greenDuck.png);";
    MonsterPic[3]="border-image: url(:/info/image/information/flyDuck.png);";
    MonsterPic[4]="border-image: url(:/info/image/information/绿色食人花.png);";
    MonsterPic[5]="border-image: url(:/info/image/information/红色食人花.png);";
    MonsterPic[6]="border-image: url(:/info/image/information/飞行鸭.png);";
}

//初始化电脑对手图片
void Dungeon::initialCmptorPic()
{
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            if(i==0||i==1)
            {
                CmptorPic[i][j]=QString("border-image: url(:/Cmptor/image/Cmptor/")+
                                  QString::number(i)+QString(".png);");
                break;
            }
            else
            {
                CmptorPic[i][j]=QString("border-image: url(:/Cmptor/image/Cmptor/")+
                                  QString::number(i)+"/"+QString::number(i)+QString::number(j)+QString(".png);");
            }
        }
    }
}

//pk战斗计算
int Dungeon::pk(struct Monster&m)
{
    // 计算双方的实际伤害值
    int playerDamage = role.atk - m.def;   // 玩家对怪物造成的伤害
    int monsterDamage = m.atk - role.def;  // 怪物对玩家造成的伤害

    // ========== 情况1：玩家无法破怪物的防 ==========
    if (playerDamage <= 0)
    {
        if (monsterDamage > 0)
            return -1;  // 怪物能破防，玩家打不过
        else
            return -2;  // 双方都无法破防，需要提升攻击力
    }

    // ========== 情况2：怪物无法破玩家的防 ==========
    if (monsterDamage <= 0)
    {
        return 1;  // 玩家无伤胜利
    }

    // ========== 情况3：双方都能破防，模拟回合制战斗 ==========
    // 复制战斗数据，避免修改原始值
    int playerHp = role.hp;
    int monsterHp = m.hp;

    // 模拟回合制战斗（玩家先手攻击）
    while (true)
    {
        // 玩家攻击怪物
        monsterHp -= playerDamage;
        if (monsterHp <= 0)
        {
            return 1;  // 怪物死亡，玩家胜利
        }

        // 怪物攻击玩家
        playerHp -= monsterDamage;
        if (playerHp <= 0)
        {
            return -1;  // 玩家死亡，打不过
        }
    }
}

//伤害计算
int Dungeon::calc_damage(int MonsterNum)
{
    if (role.atk <= enemy[MonsterNum].def)
    {
        if (enemy[MonsterNum].atk > role.def)
            return -1;
        else
            return -2;
    }
    else
    {
        if (enemy[MonsterNum].atk > role.def)
        {
            //计算杀死怪物需要的回合数
            int atk_times = (enemy[MonsterNum].hp - 1) / (role.atk - enemy[MonsterNum].def);//怪物攻击的次数
            return atk_times * (enemy[MonsterNum].atk - role.def);//造成的伤害
        }
        else
        {
            return 0;//怪物破不了我们的防，造成的伤害为0
        }
    }

}

//求和
int Dungeon::sum(int arr[],int n)
{
    if(arr==nullptr)
        return 0;
    int res=0;
    for(int i=0;i<n;++i)
        res+=arr[i];
    return res;
}
//深度优先搜索
void Dungeon::dfs(int arr[12][16],bool visited[12][16],QStack<std::pair<int,int>>&s,int i,int j)
{
    if(i<0||i>=12||j<0||j>=16||arr[i][j]==0||visited[i][j])
        return ;
    visited[i][j]=true;
    s.push(std::pair<int,int>(i,j));
    for(auto dir:directions)
        dfs(arr,visited,s,i+dir[0],j+dir[1]);

}

//广度优先搜索
void Dungeon::bfs(int arr[12][16],QStack<std::pair<int,int>>&s,std::pair<int,int>start,std::pair<int,int>end)
{
    // 访问标记数组
    bool visited[12][16] = {false};

    // 记录每个格子是从哪个格子来的（用于回溯路径）
    std::pair<int,int> parent[12][16];

    // BFS队列
    QQueue<std::pair<int,int>> q;

    // 起点入队
    q.enqueue(start);
    visited[start.first][start.second] = true;
    parent[start.first][start.second] = std::make_pair(-1, -1);  // 起点无父节点

    while (!q.empty())
    {
        std::pair<int,int> current = q.dequeue();
        int ci = current.first;
        int cj = current.second;

        // 到达终点，回溯路径
        if (ci == end.first && cj == end.second)
        {
            // 清空栈（如果有旧数据）
            s.clear();

            // 从终点回溯到起点，将路径压入栈
            std::pair<int,int> node = end;
            while (node.first != -1 && node.second != -1)
            {
                s.push(node);
                node = parent[node.first][node.second];
            }
            return;  // 找到路径，直接返回
        }

        // 向四个方向探索
        for (auto dir : directions)
        {
            int ni = ci + dir[0];
            int nj = cj + dir[1];

            // 边界检查
            if (ni < 0 || ni >= 12 || nj < 0 || nj >= 16)
                continue;

            // 障碍物检查 或 已访问检查
            if (arr[ni][nj] == 0 || visited[ni][nj])
                continue;

            // 新格子入队
            visited[ni][nj] = true;
            parent[ni][nj] = current;
            q.enqueue(std::make_pair(ni, nj));
        }
    }

    // 没有找到路径，栈保持为空（或清空）
    s.clear();

}

//随机深度优先搜索
void Dungeon::doDfs(int arr[12][16],QStack<std::pair<int,int>>&s,bool visited[12][16])
{
    //随机生成起始点坐标
    int dfsi=rand()%12;
    int dfsj=rand()%16;
    //如果当前格子不可走，则继续寻找
    while(arr[dfsi%12][dfsj%16]!=1)
    {
        dfsj++;
        if(dfsj%16==0)
            dfsi++;
    }
    //从找到的合法起点开始DFS
    dfs(arr,visited,s,dfsi%12,dfsj%16);
}

//设置起点和终点
void Dungeon::setExit(int arr[12][16],std::pair<int,int>&start,std::pair<int,int>&end)
{
    QStack<std::pair<int,int>> s;
    bool visited[12][16]={false};
    doDfs(arr,s,visited);
    //选择路径上的第0个点作为起点
    int startPos=0;
    int starti=s[startPos].first,startj=s[startPos].second;
    int endi=s.top().first,endj=s.top().second;
    //如果起点和终点距离太近，则重新生成
    while(std::abs(starti-endi)<=3||std::abs(startj-endj)<=3)
    {
        s.clear();
        //重置visited数组
        for(int i=0;i<12;++i)
        {
            for(int j=0;j<16;++j)
            {
                if(visited[i][j])
                    visited[i][j]=false;
            }
        }
        doDfs(arr,s,visited);
        starti=s[startPos].first;
        startj=s[startPos].second;
        endi=s.top().first;
        endj=s.top().second;
    }
    //设置起点坐标
    start.first=starti;
    start.second=startj;
    //终点坐标
    end.first=endi;
    end.second=endj;
    arr[starti][startj]=6;
    arr[endi][endj]=7;
}

//生成怪物
//地图值14-22：怪物
void Dungeon::GenerateMonster(int arr[12][16],int floor)
{
    int totalMonsterNum=40+floor*2;
    int MonsterArr[9]{0};
    int MonsterInitial[9]{8,8,5,3,1,0,0,0,0};
    for(int i=0;i<9;++i)
    {
        if(i==0)
        {
            //类型0怪物：随楼层增加而减少
            MonsterArr[i]=MonsterInitial[i]-(floor-1)*2;
            if(MonsterArr[i]<=2)
                MonsterArr[i]=2;//至少两只
        }
        else if(i==1)
        {
            MonsterArr[i]=MonsterInitial[i]-(floor-1)*2;
            if(MonsterArr[i]<=2)
                MonsterArr[i]=0;
        }
        else if(i==2)
            MonsterArr[i]=5;
        else if(i==3)
            MonsterArr[i]=3;
        else
        {
            //高级怪随楼层增加而增加
            MonsterArr[i]=MonsterInitial[i]+(floor-1)*(0.5*(i-3));
        }
    }
    //如果怪物总数不足，补充怪物
    while(sum(MonsterArr,9)<totalMonsterNum)
    {
        for(int i=floor-1;i<9;++i)
            MonsterArr[i]+=1;
    }
    //平衡怪物数量
    while(MonsterArr[3]>3)
    {
        MonsterArr[2]++;
        MonsterArr[3]--;
    }
    //地图上放置
    int GenerateMonsterNum=0;
    int MonsterArrIndex=0;
    // 首先在起点周围放置一些弱怪物
    for(int i=5;i<=7;++i)
    {
        for(int j=6;j<=9;++j)
        {
            if(arr[i][j]==1 && rand()%3 == 0) // 1/3 概率放置弱怪物
            {
                arr[i][j] = 14; // 小星星
                GenerateMonsterNum++;
            }
        }
    }
    while(GenerateMonsterNum<totalMonsterNum)
    {
        for(int i=0;i<12;++i)
        {
            for(int j=0;j<16;++j)
            {
                //只有可走的路才可以放怪物，且不是起点周围
                if(arr[i][j]==1 && !(i >= 5 && i <= 7 && j >= 6 && j <= 9)) // 避免在起点周围放置怪物
                {

                    if(MonsterArr[MonsterArrIndex++%9]!=0)
                    {
                        arr[i][j]= (MonsterArrIndex-1)%9+14;
                        MonsterArr[(MonsterArrIndex-1)%9]--;
                    }
                    //随机跳过一些格子，避免怪物太密集
                    i+=rand()%6;
                    j+=rand()%8;
                    GenerateMonsterNum++;
                    if(GenerateMonsterNum>=totalMonsterNum)
                        return;
                }
            }
        }
    }
}

//生成物品
//值为23-27 buff物品
//31-32 工具物品
void Dungeon::GenerateItem(int arr[12][16],int floor)
{
    //生成buff物品
    int totalBuffItemNum=5-floor+1;
    int BuffItemArr[5]{0};
    int BuffItemInitial[5]{2,1,1,0,0};
    for(int i=0;i<5;++i)
    {
        if(i==2)
            BuffItemArr[i]=BuffItemInitial[i]+(floor-1)*0.5;
        else
        {
            BuffItemArr[i]=BuffItemInitial[i]+(floor-1)*1;
        }
    }
    while(sum(BuffItemArr,5)<totalBuffItemNum)
    {
        for(int i=0;i<3+floor-1;++i)
            BuffItemArr[i]+=1;
    }
    //地图上放置
    int GenerateBuffItemNum=0;
    int BuffItemArrIndex=0;
    bool GenerateBuffItemDone=false;
    while(GenerateBuffItemNum<totalBuffItemNum)
    {
        for(int i=0;i<12;++i)
        {
            for(int j=0;j<16;++j)
            {
                if(arr[i][j]==1)
                {
                    if(BuffItemArr[BuffItemArrIndex++%5]!=0)
                    {
                        arr[i][j]= (BuffItemArrIndex-1)%5+23;
                        BuffItemArr[(BuffItemArrIndex-1)%5]--;
                    }
                    i+=rand()%6;
                    j+=rand()%8;
                    GenerateBuffItemNum++;
                    if(GenerateBuffItemNum>=totalBuffItemNum)
                    {
                        GenerateBuffItemDone=true;
                        break;
                    }
                }
            }
            if(GenerateBuffItemDone)
                break;
        }
        if(GenerateBuffItemDone)
            break;
    }

    //生成工具物品
    int totalToolItemNum=4-floor+1;
    int ToolItemArr[2]{0};
    int ToolItemInitial[2]{2,1};
    for(int i=0;i<2;++i)
    {
        if(i==0)
            ToolItemArr[i]=ToolItemInitial[i]+(floor-1)*1;
        else
            ToolItemArr[i]=ToolItemInitial[i]+(floor-1)*0.5;
    }
    while(sum(ToolItemArr,1)<totalToolItemNum)
    {
        for(int i=0;i<1;++i)
            ToolItemArr[i]+=1;
    }
    int GenerateToolItemNum=0;
    int ToolItemArrIndex=0;
    while(GenerateToolItemNum<totalToolItemNum)
    {
        for(int i=0;i<12;++i)
        {
            for(int j=0;j<16;++j)
            {
                if(arr[i][j]==1)
                {
                    if(ToolItemArr[ToolItemArrIndex++%2]!=0)
                    {
                        arr[i][j]= (ToolItemArrIndex-1)%2+31;
                        ToolItemArr[(ToolItemArrIndex-1)%2]--;
                    }
                    i+=rand()%6;
                    j+=rand()%8;
                    GenerateToolItemNum++;
                    if(GenerateToolItemNum>=totalToolItemNum)
                        return;
                }
            }
        }
    }

}

//生成装备
void Dungeon::GenerateEquip()
{
    int ironSwordFloor=2;//铁剑：2层
    int flowerSwordFloor=4;//花剑：4层
    int goldenSwordFloor=7;//金剑：7层
    //放置
    //检查目标位置是否在起点（6）或终点（7）
    if(map[ironSwordFloor-1][10][14]==6||map[ironSwordFloor-1][10][14]==7)
    {
        //占用（10,13）
        map[ironSwordFloor-1][10][13]=28;
    }
    else
        //未被占用（10,14）
        map[ironSwordFloor-1][10][14]=28;
    if(map[flowerSwordFloor-1][10][14]==6||map[flowerSwordFloor-1][10][14]==7)
    {
        map[flowerSwordFloor-1][10][13]=29;
    }
    else
        map[flowerSwordFloor-1][10][14]=29;
    if(map[goldenSwordFloor-1][10][14]==6||map[goldenSwordFloor-1][10][14]==7)
    {
        map[goldenSwordFloor-1][10][13]=30;
    }
    else
        map[goldenSwordFloor-1][10][14]=30;
}

//生成地图元素
void Dungeon::GenerateElement(int arr[12][16],int floor)
{
    GenerateMonster(arr,floor);
    GenerateItem(arr,floor);
}
//检查位置是否有效
bool Dungeon::isValid(int arr[12][16], int i, int j, int floor)
{
    int val = arr[i][j];

    // 可通行值的范围
    return (val >= 1 && val <= 1) ||   // 路
           (val >= 6 && val <= 7) ||   // 起点、终点
           (val >= 14 && val <= 32);   // 怪物、物品、装备
}
//随机生成地图
void Dungeon::RandomGeneraterMap(int arr[12][16], int floor)
{
    std::pair<int, int> start;
    std::pair<int, int> end;

    // 1. 设置起点和终点
    setExit(arr, start, end);

    // 2. 生成怪物和物品
    GenerateElement(arr, floor);

    // 3. 检查是否有路径
    QStack<std::pair<int, int>> pathStack;
    bfs(arr, pathStack, start, end);

    // 4. 如果没有路径，清理一条路径出来
    if (pathStack.isEmpty())
    {
        // 使用DFS找一条路径（忽略怪物和物品）
        bool visited[12][16] = {false};
        QStack<std::pair<int, int>> dfsStack;
        doDfs(arr, dfsStack, visited);

        // 将DFS路径上的格子清空为可走的路
        while (!dfsStack.isEmpty())
        {
            std::pair<int, int> pos = dfsStack.top();
            dfsStack.pop();
            if (arr[pos.first][pos.second] != 6 && arr[pos.first][pos.second] != 7)
            {
                arr[pos.first][pos.second] = 1;  // 清空为路
            }
        }

        // 重新BFS获取路径
        bfs(arr, pathStack, start, end);
    }

    // 5. 确保路径上的格子不会被怪物或物品阻塞
    while (!pathStack.isEmpty())
    {
        std::pair<int, int> pos = pathStack.top();
        pathStack.pop();

        // 跳过起点和终点
        if (pos == start || pos == end)
            continue;

        // 如果路径上是怪物或物品，替换为路
        int val = arr[pos.first][pos.second];
        if (val >= 14 && val <= 30)  // 怪物或物品
        {
            arr[pos.first][pos.second] = 1;
        }
    }
}

//初始化所有地图
void Dungeon::initialMap()
{
    currentFloor=1;
    MAX_HP=100;
    MAX_MP=100;
    deadTimes=0;
    isPre=false;
    isNext=false;
    initialrole();
    while(1)
    {

        if(currentFloor==1)
        {
            int tempmap[12][16]={
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,
                0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,1,
                0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,
                0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,
                0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,
                0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,
                0,1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,
                0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0,
                0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0

            };
            //在模版基础上随机生成怪物和物品
            RandomGeneraterMap(tempmap,currentFloor);
            //将生成的地图复制到全局地图数组
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        if(currentFloor==2)
        {
            int tempmap[12][16]={
                1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,
                1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,
                1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,
                1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
                1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0,
                1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,
                1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,
                1,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,
                1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,
                0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0
            };
            RandomGeneraterMap(tempmap,currentFloor);
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        if(currentFloor==3)
        {
            int tempmap[12][16]={
                1,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,
                1,1,1,1,0,1,1,1,1,0,1,0,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,0,1,1,1,1,0,1,0,0,0,0,0,
                0,0,1,0,0,0,1,1,0,0,1,0,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                0,0,1,0,0,1,1,1,1,0,1,0,0,0,0,0,
                1,1,1,1,0,0,1,1,0,0,1,0,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,0,1,1,1,1,0,1,0,0,0,0,0,
                0,0,0,0,0,0,1,1,0,0,1,0,1,1,1,1,
                1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
            };
            RandomGeneraterMap(tempmap,currentFloor);
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        if(currentFloor==4)
        {
            int tempmap[12][16]={
                1,1,1,1,0,1,8,9,10,1,0,1,1,1,1,1,
                1,1,1,1,0,1,11,12,13,1,0,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,
                0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,
                1,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,
                1,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,
                1,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,
            };
            RandomGeneraterMap(tempmap,currentFloor);
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        if(currentFloor==5)
        {
            int tempmap[12][16]={
                1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,
                1,0,1,1,1,0,1,0,1,1,1,0,1,1,0,1,
                1,1,1,1,1,0,1,0,1,1,1,0,1,1,0,1,
                0,0,0,0,1,0,1,0,1,1,1,0,1,1,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,1,
                0,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,
                1,1,1,1,1,0,1,0,1,1,1,0,0,0,0,0,
                0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,1,
                1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,
            };
            RandomGeneraterMap(tempmap,currentFloor);
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        if(currentFloor==6)
        {
            int tempmap[12][16]={
                1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,
                1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,1,
                1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,
                1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1,
                1,1,1,1,1,0,1,0,1,1,1,0,1,1,0,1,
                1,1,1,1,1,0,1,0,1,1,1,0,1,1,0,1,
                1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,
                1,0,0,0,0,0,1,0,0,0,0,1,1,1,0,1,
                1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,
            };
            RandomGeneraterMap(tempmap,currentFloor);
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        if(currentFloor==7)
        {
            int tempmap[12][16]={
                1,0,1,1,0,1,0,1,1,1,0,1,0,1,0,1,
                1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,
                1,0,1,1,0,1,1,1,0,1,0,1,0,1,0,1,
                1,0,1,1,1,1,0,1,0,1,1,1,1,1,1,1,
                1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,
                1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,
                1,0,1,1,1,1,0,1,0,1,0,1,0,1,0,1,
                1,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1,
                1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,
                1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,
            };
            RandomGeneraterMap(tempmap,currentFloor);
            for(int i=0;i<12;i++)
            {
                for(int j=0;j<16;j++)
                {
                    map[currentFloor-1][i][j]=tempmap[i][j];
                }
            }
        }
        currentFloor++;
        if(currentFloor>Total_Floor)
        {
            break;
        }
    }
    currentFloor=1;
    //设置角色起始位置
    setCharacterPos(currentFloor-1,6);
    //生成特殊装备
    GenerateEquip();
}

//pk地图
void Dungeon::initialpkMap()
{
    int tempmap[12][16]={
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,1,14,14,14,0,14,14,14,0,1,1,1,1,0,
        0,1,1,14,0,0,0,14,0,14,0,0,0,1,1,0,
        0,1,0,14,14,14,14,14,0,14,14,14,0,1,1,0,
        0,1,0,0,0,0,0,0,0,0,0,14,0,1,1,0,
        0,1,14,14,14,14,0,14,14,14,0,14,0,1,1,0,
        0,1,0,0,0,0,0,14,0,14,0,14,1,1,1,0,
        0,1,1,14,0,14,14,14,0,14,0,14,1,1,1,0,
        0,1,1,14,0,14,0,0,0,14,0,0,0,0,1,0,
        0,1,1,14,0,14,14,14,0,14,14,1,1,1,1,0,
        0,1,1,14,14,14,14,14,0,14,14,1,1,1,1,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    for(int i=0;i<12;i++)
    {
        for(int j=0;j<16;j++)
        {
            map[7][i][j]=tempmap[i][j];
        }
    }
}

//设置角色位置，将人物放在目标各自旁边
void Dungeon::setCharacterPos(int floor,int target)
{
    bool findflag=false;
    int i,j;
    //在地图中查找目标格子
    for(i=0;i<12;i++)
    {
        for(j=0;j<16;j++)
        {
            if(map[floor][i][j]==target)
            {
                findflag=true;
                break;
            }
        }
        if(findflag)
            break;
    }
    if(!findflag)
        return;
    //检查四个方向下->上->右->左
    if(i<11&&((map[floor][i+1][j]==1||isDynamic(floor,i+1,j))
                   ||(map[floor][i+1][j]>=23&&map[floor][i+1][j]<=27)))
    {
        //如果目标格子是物品/怪物，先清除为路
        if(map[floor][i+1][j]!=1)
            map[floor][i+1][j]=1;
        x=i+1;
        y=j;
    }
    else if(i>0&&((map[floor][i-1][j]==1||isDynamic(floor,i-1,j))
                       ||(map[floor][i-1][j]>=23&&map[floor][i-1][j]<=27)))
    {
        if(map[floor][i-1][j]!=1)
            map[floor][i-1][j]=1;
        x=i-1;
        y=j;
    }
    else if(j<15&&((map[floor][i][j+1]==1||isDynamic(floor,i,j+1))
                        ||(map[floor][i][j+1]>=23&&map[floor][i][j+1]<=27)))
    {
        if(map[floor][i][j+1]!=1)
            map[floor][i][j+1]=1;
        x=i;
        y=j+1;
    }
    else if(j>0&&((map[floor][i][j-1]==1||isDynamic(floor,i,j-1))
                       ||(map[floor][i][j-1]>=23&&map[floor][i][j-1]<=27)))
    {
        if(map[floor][i][j-1]!=1)
            map[floor][i][j-1]=1;
        x=i;
        y=j-1;
    }
    //都不可走，放在（0,0）
    else
    {
        x=y=0;
    }
    //在起点处更新迷雾位置
    if(target==6)
        updateFogArea(floor,x,y);
}

//根据上下楼，在新楼层找到正确位置放置角色
void Dungeon::initialCharacterPos()
{
    if(isPre)
    {
        setCharacterPos(currentFloor-2,7);//7终点
        isPre=false;
    }
    if(isNext)
    {
        setCharacterPos(currentFloor,6);
        isNext=false;
    }
}

//初始化地牢
void Dungeon::initialDugeon()
{
    initialFogWidget();//初始化迷雾窗口
    initialMap();//初始化地图
    //初始化商店窗口
    Store=new StoreWidget(ptr_MianWindow);//作为主窗口的子窗口
    Store->setGeometry(292,192,256,352);
    Store->hide();
    //创建菜单窗口
    menu=new Menu(ptr_MianWindow);
    menu->setGeometry(160,160,520,320);
    menu->hide();
    //创建网络对战UI
    QFont fontLabel("Microsoft YaHei" ,12, 75);
    QFont fontNum("Microsoft YaHei" ,10, 75);
    NetMode_time=fac->CreateQLabel(ptr_MianWindow,640,20,200,40,"遭遇战倒计时:","color:white",fontLabel);
    NetMode_secondes=fac->CreateQLabel(ptr_MianWindow,740,60,80,40,QString::number(CountDown),"color:red",fontNum);
    NetMode_time->hide();
    NetMode_secondes->hide();
    //创建帮助窗口
    help=new Help(ptr_MianWindow);
    help->setGeometry(160,160,520,320);
    help->hide();
    //初始化其他组件
    initialinfoWidget();//信息栏
    initialFightWin();//战斗窗口
    initialSetNameWin();//设置名字
    initialGetitemWin();//获取物品

    GetitemTimer=new QTimer(this);
    FightTimer=new QTimer(this);
    dynamicEffectTimer=new QTimer(this);

    connect(GetitemTimer,SIGNAL(timeout()),this,SLOT(GetitemWinshow()));
    connect(FightTimer,SIGNAL(timeout()),this,SLOT(FightWinshow()));
    connect(Store->haveBuyTimer,SIGNAL(timeout()),this,SLOT(OpenStore()));
    connect(dynamicEffectTimer,SIGNAL(timeout()),this,SLOT(ShowdynamicEffect()));
}

//初始化迷雾窗口
void Dungeon::initialFogWidget()
{
    //计算格子的宽度和高度
    int BASIC_WIDTH=832/16;
    int BASIC_HEIGHT=672/12;

    for(int i=0;i<12;++i)
    {
        for(int j=0;j<16;++j)
        {
            FogWidget[i][j]=new QWidget(ptr_MianWindow);
            FogWidget[i][j]->setGeometry(j*BASIC_WIDTH,i*BASIC_HEIGHT,BASIC_WIDTH,BASIC_HEIGHT);
            FogWidget[i][j]->setAutoFillBackground(true);
            FogWidget[i][j]->hide();//等地图加载后再显示
        }
    }

}

//初始化信息栏，创建显示角色的所有UI控件
void Dungeon::initialinfoWidget()
{
    infoWidget=new QWidget(ptr_MianWindow);
    QFont fontLabel("Microsoft YaHei" ,12, 75);
    QFont fontNum("Microsoft YaHei" ,10, 65);
    QFont fontName("Microsoft YaHei" ,10, 55);
    QString labelbg_style="border-image: url(:/interface/image/interface/labelbg.png);";
    QString CharacterPic_style="border-image: url(:/info/image/information/Character.png);";
    QString Pickaxe_style="border-image: url(:/info/image/information/Pickaxe.png);";
    QString Downstairs_style="border-image: url(:/info/image/information/Downstairs.png);";
    QString redkey_style="border-image: url(:/info/image/information/redkey.png);";
    //第一行：角色头像和名称区域
    QLabel *CharacterPic=fac->CreateQLabel(infoWidget,20,20,40,40,"",CharacterPic_style);
    QLabel *label_Name=fac->CreateQLabel(infoWidget,20,70,100,30,"人们叫他",labelbg_style,fontLabel,Qt::AlignLeft);
    CharacterName=fac->CreateQLabel(infoWidget,130,70,170,30,"",labelbg_style,fontName,Qt::AlignLeft);
    //第二行：等级
    QLabel *label_level=fac->CreateQLabel(infoWidget,140,20,40,40,"级",labelbg_style,fontLabel);
    //第三行：生命值
    QLabel *label_hp=fac->CreateQLabel(infoWidget,20,110,65,30,"生命",labelbg_style,fontLabel,Qt::AlignLeft);
    //第四行：魔法值
    QLabel *label_mp=fac->CreateQLabel(infoWidget,20,150,65,30,"魔法",labelbg_style,fontLabel,Qt::AlignLeft);
    //第五行：攻击力
    QLabel *label_atk=fac->CreateQLabel(infoWidget,20,230,65,30,"攻击",labelbg_style,fontLabel,Qt::AlignLeft);
    //第六行：防御力
    QLabel *label_def=fac->CreateQLabel(infoWidget,20,270,65,30,"防御",labelbg_style,fontLabel,Qt::AlignLeft);
    //第七行：金币
    QLabel *label_score=fac->CreateQLabel(infoWidget,20,310,65,30,"金币",labelbg_style,fontLabel,Qt::AlignLeft);
    //经验值
    QLabel *label_exp=fac->CreateQLabel(infoWidget,20,190,65,30,"经验",labelbg_style,fontLabel,Qt::AlignLeft);
    //道具图标（破墙镐头）
    QLabel *label_Pickaxe=fac->CreateQLabel(infoWidget,20,370,40,40,"",Pickaxe_style);
    //下楼图标
    QLabel *label_Downstairs=fac->CreateQLabel(infoWidget,20,420,40,40,"",Downstairs_style);
    //当前楼层显示
    QLabel *label_floor1=fac->CreateQLabel(infoWidget,20,520,40,40,"第",labelbg_style,fontLabel);
    QLabel *label_floor2=fac->CreateQLabel(infoWidget,140,520,40,40,"层",labelbg_style,fontLabel);

    //数值显示
    level=fac->CreateQLabel(infoWidget,80,20,40,40,QString::number(role.level),labelbg_style,fontNum);
    hp=fac->CreateQLabel(infoWidget,105,110,100,30,QString::number(role.hp),labelbg_style,fontNum);
    mp=fac->CreateQLabel(infoWidget,105,150,100,30,QString::number(role.mp),labelbg_style,fontNum);
    atk=fac->CreateQLabel(infoWidget,105,230,100,30,QString::number(role.atk),labelbg_style,fontNum);
    def=fac->CreateQLabel(infoWidget,105,270,100,30,QString::number(role.def),labelbg_style,fontNum);
    score=fac->CreateQLabel(infoWidget,105,310,100,30,QString::number(role.scoreNum),labelbg_style,fontNum);
    exp=fac->CreateQLabel(infoWidget,105,190,100,30,QString::number(role.exp),labelbg_style,fontNum);
    PickaxeNum=fac->CreateQLabel(infoWidget,105,370,100,30,QString::number(role.Pickaxe),labelbg_style,fontNum);
    DownstairsNum=fac->CreateQLabel(infoWidget,105,420,100,30,QString::number(role.Downstairs),labelbg_style,fontNum);
    Floor=fac->CreateQLabel(infoWidget,80,520,40,40,QString::number(currentFloor),labelbg_style,fontNum);

    //pk对战信息，默认隐藏，只在pk模式下显示
    label_MyName=fac->CreateQLabel(infoWidget,20,580,100,30,"",labelbg_style,fontLabel,Qt::AlignLeft);
    label_VS=fac->CreateQLabel(infoWidget,120,580,40,30,"VS",labelbg_style,fontLabel,Qt::AlignLeft);
    label_CmptorName=fac->CreateQLabel(infoWidget,180,580,100,30,"",labelbg_style,fontLabel,Qt::AlignLeft);
    label_MyPonit=fac->CreateQLabel(infoWidget,50,620,40,30,"0",labelbg_style,fontNum,Qt::AlignLeft);
    label_split=fac->CreateQLabel(infoWidget,130,620,40,30,":",labelbg_style,fontLabel,Qt::AlignLeft);//分隔符
    label_CmptorPoint=fac->CreateQLabel(infoWidget,210,620,40,30,"0",labelbg_style,fontNum,Qt::AlignLeft);
    label_MyName->hide();
    label_VS->hide();
    label_CmptorName->hide();
    label_MyPonit->hide();
    label_split->hide();
    label_CmptorPoint->hide();

    connect(menu->ReturnMainMenu,SIGNAL(clicked()),this,SLOT(ReturnMainMenuPlay()));
    connect(menu->Restart,SIGNAL(clicked()),this,SLOT(RestartPlay()));
    connect(menu->Continue,SIGNAL(clicked()),this,SLOT(ContinuePlay()));
    connect(menu->ReturnRooms,SIGNAL(clicked()),this,SLOT(ReturnRoomsPlay()));
    connect(menu->Help,SIGNAL(clicked()),this,SLOT(HelpPlay()));
    connect(help->Return,SIGNAL(clicked()),this,SLOT(HelpReturnPlay()));
    connect(menu->music,SIGNAL(clicked()),this,SLOT(musicChangePlay()));
    //信息栏样式
    infoWidget->setMinimumSize(300,672);
    infoWidget->setStyleSheet("color:white;border-image: url(:/interface/image/interface/infobg.png);");
    infoWidget->hide();
}

//返回主菜单
void Dungeon::ReturnMainMenuPlay()
{
    infoWidget->hide();//隐藏状态界面
    hideDungeon();//隐藏地牢界面
    hideFog();
    //重置菜单界面显示
    menu->pauseEvent->setText("PAUSE");
    menu->Restart->show();
    menu->Continue->show();
    menu->hide();
    NetMode_time->hide();
    NetMode_secondes->hide();
    //重置对战分数
    MyPonit=0;
    CmptorPoint=0;
    //重置游戏状态
    surface->isok=false;
    NetModeStart=false;
    //切换到界面背景音乐
    music->InterfaceBGM->play();
    music->InterfaceBGMTimer->start(10);
    NameEdit->clear();
    surface->show();//开始界面显示后，主窗口大小还是不变
    ptr_MianWindow->resize(832,672);//需要重新设置主窗口大小
    surface->resizeEvent(nullptr);
    surface->showMainMenu();
    //充值游戏数据
    initialFogArr();
    initialMap();
    Store->initialStoreDatas();
    updateStatusData();
}

//重新开始
void Dungeon::RestartPlay()
{
    menu->hide();
    isDead=false;
    moveNum=0;
    moveDirection=-1;
    initialFogArr();
    initialMap();
    Store->initialStoreDatas();
    ShowDungeon();
    ShowFog();
    updateStatusData();
}

//继续游戏
void Dungeon::ContinuePlay()
{
    menu->hide();
}

//返回房间
void Dungeon::ReturnRoomsPlay()
{
    ReturnMainMenuPlay();
    menu->hide();
    if(surface->sign->roomWidget->owner_!=surface->sign->roomWidget->playerName)
        surface->sign->roomWidget->startGameBtn->setText("准备");
    surface->sign->roomWidget->show();
}

void Dungeon::HelpPlay()
{
    help->show();
}
void Dungeon::HelpReturnPlay()
{
    help->hide();
}

void Dungeon::musicChangePlay()
{
    music->musicChange=!music->musicChange;
    if(music->musicChange==false)
    {
        music->m_soundManager->setVolume(0);
        music->m_soundManager->setVolume(0);
        music->m_soundManager->setVolume(0);
        music->m_soundManager->setVolume(0);
        menu->music->setStyleSheet("border-image: url(:/info/image/information/music-off.png);");
    }
    else
    {
        music->m_soundManager->setVolume(50);
        music->m_soundManager->setVolume(50);
        music->m_soundManager->setVolume(80);
        music->m_soundManager->setVolume(80);
        menu->music->setStyleSheet("border-image: url(:/info/image/information/music-on.png);");
    }
}

void Dungeon::ShowFog()
{
    for(int i=0;i<12;i++)
    {
        for(int j=0;j<16;j++)
        {
            if(FogArr[currentFloor-1][i][j] == 15)
            {
                FogWidget[i][j]->hide();
                continue;
            }
            QPalette palette;
            //从迷雾图片中裁剪对应迷雾强度的区域
            palette.setBrush(QPalette::Window,
                             QBrush(QImage(":/Dungeon/image/Dungeon/fog.png").
                                    copy((FogArr[currentFloor-1][i][j]/4)*54,(FogArr[currentFloor-1][i][j]%4)*58,54,58)
                                    ));
            FogWidget[i][j]->setPalette(palette);
            FogWidget[i][j]->show();
            FogWidget[i][j]->raise();
        }
    }

    if(NetMode_time && NetMode_time->isVisible())
        NetMode_time->raise();
    if(NetMode_secondes && NetMode_secondes->isVisible())
        NetMode_secondes->raise();
    if(menu && menu->isVisible())
        menu->raise();
    if(help && help->isVisible())
        help->raise();
    if(infoWidget && infoWidget->isVisible())
        infoWidget->raise();
}

void Dungeon::hideFog()
{
    for(int i=0;i<12;i++)
    {
        for(int j=0;j<16;j++)
        {
            FogWidget[i][j]->hide();
        }
    }
}

//显示地牢地图（所有元素）
void Dungeon::ShowDungeon()
{
    int BASIC_WIDTH=832/16;
    int BASIC_HEIGHT=672/12;
    if(surface->surfaceShow)
    {
        surface->hide();
        //gLayout_Map->removeWidget(surface);
        surface->surfaceShow=false;
    }

    //显示角色
    People->setMinimumSize(BASIC_WIDTH,BASIC_HEIGHT);
    gLayout_Map->addWidget(People,x,y);
    People->setStyleSheet(DungeonStytle[3][0]);
    People->show();
    //修正楼层范围
    if(currentFloor==0)
        currentFloor=1;
    else if(currentFloor>8)
        currentFloor=7;
    for(int i=0;i<12;i++)
    {
        for(int j=0;j<16;j++)
        {
            MazeWidget[i][j]->setMinimumSize(BASIC_WIDTH,BASIC_HEIGHT);
            MazeWidget[i][j]->show();
            MazeWidget[i][j]->setStyleSheet(DungeonStytle[map[currentFloor-1][i][j]][0]);
        }
    }
    //启动动态效果定时器，用于处理动画效果
    dynamicEffectTimer->start(500);

    music->DungeonBGM->play();
    music->DungeonBGMTimer->start(10);
}

void Dungeon::hideDungeon()
{
    for(int i=0;i<12;i++)
    {
        for(int j=0;j<16;j++)
        {
            MazeWidget[i][j]->hide();
        }
    }
    People->hide();
    surface->dungeonisok=false;
    dynamicEffectTimer->stop();
    music->DungeonBGM->stop();
    music->DungeonBGMTimer->stop();
}

void Dungeon::initialSetNameWin()
{
    QString button_style="QPushButton{background-color:black;color:white;border-radius:10px;}"
                           "QPushButton:hover{background-color:black;color:springgreen;}"
                           "QPushButton:pressed{background-color:black;color:springgreen;}";
    QFont font("Microsoft YaHei" ,12, 35);

    NameEdit=fac->CreateQLineEdit(ptr_MianWindow,320,260,200,40,"color:black;background-color:white;border:1px solid black;",font);
    SetDone=fac->CreateQPushButton(ptr_MianWindow,360,320,120,30,"确定",button_style,font);
    connect(SetDone,SIGNAL(clicked()),this,SLOT(EnterDungeon()));
    NameEdit->hide();
    SetDone->hide();
}

void Dungeon::EnterDungeon()
{
    if(NameEdit->text()=="")
    {
        return;
    }
    else
    {
        NameEdit->hide();
        SetDone->hide();
        Character_name->setText(NameEdit->text());
        CharacterName->setText(NameEdit->text());
        infoWidget->show();//显示信息栏
        menu->ReturnRooms->hide();
        setCharacterPos(currentFloor-1, 6); // 初始进入地牢时放置角色到起点附近
        ShowDungeon();
        updateFogArea(currentFloor-1, x, y);
        ShowFog();
        music->InterfaceBGM->stop();
        music->InterfaceBGMTimer->stop();
    }
}
//网络对战模式
void Dungeon::NetStart()
{
    NetModeStart=true;
    currentFloor=1;
    MyPonit=0;
    CmptorPoint=0;
    //更新分数显示
    label_MyPonit->setText(QString::number(MyPonit));
    label_CmptorPoint->setText(QString::number(CmptorPoint));
    menu->pauseEvent->setText("PAUSE");
    menu->Restart->hide();//不支持重新开始
    initialFogArr();
    setCharacterPos(currentFloor-1,6);
    surface->button_NetPlay->hide();
    surface->button_LocalDungeon->hide();
    surface->button_Quit->hide();
    surface->button_About->hide();
    surface->sign->roomWidget->hide();
    surface->dungeonisok=true;
    //显示网络对战倒计时
    NetMode_time->show();
    NetMode_secondes->show();
    //创建倒计时对战定时器
    NetModeCountDownTimer=new QTimer;
    connect(NetModeCountDownTimer,SIGNAL(timeout()),this,SLOT(NetModeCountDownRun()));
    NetModeCountDownTimer->start(1000);//每秒触发一次
    Character_name->setText(surface->sign->roomWidget->playerName);
    CharacterName->setText(surface->sign->roomWidget->playerName);
    infoWidget->show();
    ShowDungeon();
    ShowFog();
    music->InterfaceBGM->stop();
    music->InterfaceBGMTimer->stop();
    NetStartTimer->stop();
    initialpkMap();
    competitor=new QWidget;
    competitor->hide();
}
//每秒更新计时数字
void Dungeon::NetModeCountDownRun()
{
    NetMode_secondes->setText(QString::number(--CountDown));
    if(CountDown<=0)
    {
        CountDown=80;//重置，发起请求
        surface->sign->roomWidget->chatRoom->pkRequest();
        NetModeCountDownTimer->stop();
    }
}

void Dungeon::pkStart()
{
    stopAllwork();//停止所有可能的动作
    hideDungeon();//隐藏地牢界面
    hideFog();
    surface->dungeonisok=true;
    NetMode_time->hide();
    NetMode_secondes->hide();
    //保存当前位置信息
    pre_floor=currentFloor;
    pre_x=x;
    pre_y=y;
    currentFloor=8;//pk专用地图
    if(surface->sign->roomWidget->playerName==surface->sign->roomWidget->owner_)
    {
        //当前玩家是房主：位于左下角，对手：右上角
        x=1;
        y=1;
        surface->sign->roomWidget->chatRoom->cur_x=10;
        surface->sign->roomWidget->chatRoom->cur_y=14;
        //添加对手控件
        gLayout_Map->addWidget(competitor,10,14);
        competitor->setStyleSheet(CmptorPic[1][0]);
        competitor->show();
    }
    else
    {
        x=10;
        y=14;
        surface->sign->roomWidget->chatRoom->cur_x=1;
        surface->sign->roomWidget->chatRoom->cur_y=1;
        gLayout_Map->addWidget(competitor,1,1);
        competitor->setStyleSheet(CmptorPic[1][0]);
        competitor->show();
    }
    //更新对手状态
    surface->sign->roomWidget->chatRoom->udpkStatusRequest(role.hp,role.atk,role.def);
    initialpkMap();
    ShowDungeon();
    pkStartTimer->stop();
}

//pk结束后恢复到普通游戏状态
void Dungeon::ReturnDungeon(bool localWin)
{
    MyWin = localWin;
    competitor->hide();
    //恢复之前位置
    currentFloor=pre_floor;
    x=pre_x;
    y=pre_y;
    //显示地牢界面
    infoWidget->show();
    ShowDungeon();
    ShowFog();
    //显示pk对战信息
    label_MyName->setText(surface->sign->roomWidget->chatRoom->playerName);
    label_MyName->show();
    label_VS->show();
    label_CmptorName->setText(surface->sign->roomWidget->chatRoom->competitoName);
    label_CmptorName->show();
    //更新分数
    if(!dogFall)//不是平局
    {
        if(MyWin)
            MyPonit++;
        else
            CmptorPoint++;
    }
    MyWin=false;
    dogFall=false;
    //更新分数显示
    label_MyPonit->setText(QString::number(MyPonit));
    label_MyPonit->show();
    label_split->show();
    label_CmptorPoint->setText(QString::number(CmptorPoint));
    label_CmptorPoint->show();
    //判断游戏是否结束，有一方先达到两分，结束
    if(MyPonit==2||CmptorPoint==2)
    {
        pkOverCall(MyPonit,CmptorPoint);
        return;
    }
    //准备下一轮ok
    NetMode_time->show();
    NetMode_secondes->show();
    NetModeCountDownTimer->start(1000);
    if (surface->sign->roomWidget->chatRoom)
        surface->sign->roomWidget->chatRoom->resetPkStartFlag();
}
//pk结束时调用，显示游戏结束界面，宣布胜利者
void Dungeon::pkRoundResult(bool localWin)
{
    ReturnDungeon(localWin);
}

void Dungeon::pkOverCall(int MyPoint,int CmptorPoint)
{
    if(CmptorPoint == 2 && NetModeStart && surface->sign->roomWidget->chatRoom)
    {
        // PK 结束时，失败方通知对手，胜利方通过对方的结束请求显示胜利
        surface->sign->roomWidget->chatRoom->GameOverRequest();
    }

    if(menu)
    {
        menu->show();
        menu->raise();
    }

    if(MyPoint == 2)
    {
        menu->pauseEvent->setText("GAME OVER！You Win!");
    }
    else if(CmptorPoint == 2)
    {
        QString WinerName = surface->sign->roomWidget->chatRoom->competitoName;
        if(WinerName.isEmpty())
            WinerName = "Opponent";
        menu->pauseEvent->setText("GAME OVER！You Lost! Winner: " + WinerName);
    }

    //显示返回房间按钮
    menu->ReturnRooms->show();
    menu->Restart->hide();
    menu->Continue->hide();
    NetModeCountDownTimer->stop();
}

//pk地图更新（对手移动）
void Dungeon::pkMapUpdate()
{
    //获取对手当前坐标和移动方向
    int cur_x=surface->sign->roomWidget->chatRoom->cur_x;
    int cur_y=surface->sign->roomWidget->chatRoom->cur_y;
    int dir=surface->sign->roomWidget->chatRoom->dir;
    switch(dir)
    {
    case 0://up
    {
        if(cur_x>0)
        {
            cur_x-=1;
            surface->sign->roomWidget->chatRoom->cur_x=cur_x;
            //检查目标格子是否可走
            if(map[currentFloor-1][cur_x][cur_y]!=0)
            {
                //如果是怪物，播放战斗音效并击败怪物
                if(map[currentFloor-1][cur_x][cur_y]==14)
                    music->SoundPlay(0);
                //清空格子，变成路
                map[currentFloor-1][cur_x][cur_y]=1;
                MazeWidget[cur_x][cur_y]->setStyleSheet(DungeonStytle[1][0]);
                //移动对手到新位置
                gLayout_Map->addWidget(competitor,cur_x,cur_y);
                competitor->setStyleSheet(CmptorPic[0][0]);
            }
        }
        else
            break;
    }
    break;
    case 1://down
    {
        if(cur_x<11)
        {
            cur_x+=1;
            surface->sign->roomWidget->chatRoom->cur_x=cur_x;
            if(map[currentFloor-1][cur_x][cur_y]!=0)
            {
                if(map[currentFloor-1][cur_x][cur_y]==14)
                    music->SoundPlay(0);
                map[currentFloor-1][cur_x][cur_y]=1;
                MazeWidget[cur_x][cur_y]->setStyleSheet(DungeonStytle[1][0]);
                gLayout_Map->addWidget(competitor,cur_x,cur_y);
                competitor->setStyleSheet(CmptorPic[1][0]);
            }
        }
        else
            break;
    }
    break;
    case 2://left
    {
        if(cur_y>0)
        {
            cur_y-=1;
            surface->sign->roomWidget->chatRoom->cur_y=cur_y;
            if(map[currentFloor-1][cur_x][cur_y]!=0)
            {
                if(map[currentFloor-1][cur_x][cur_y]==14)
                    music->SoundPlay(0);
                map[currentFloor-1][cur_x][cur_y]=1;
                MazeWidget[cur_x][cur_y]->setStyleSheet(DungeonStytle[1][0]);
                gLayout_Map->addWidget(competitor,cur_x,cur_y);
                competitor->setStyleSheet(CmptorPic[2][moveNum++%4]);//使用动画帧
            }
        }
        else
            break;
    }
    break;
    case 3://right
    {
        if(cur_y<15)
        {
            cur_y+=1;
            surface->sign->roomWidget->chatRoom->cur_y=cur_y;
            if(map[currentFloor-1][cur_x][cur_y]!=0)
            {
                if(map[currentFloor-1][cur_x][cur_y]==14)
                    music->SoundPlay(0);
                map[currentFloor-1][cur_x][cur_y]=1;
                MazeWidget[cur_x][cur_y]->setStyleSheet(DungeonStytle[1][0]);
                gLayout_Map->addWidget(competitor,cur_x,cur_y);
                competitor->setStyleSheet(CmptorPic[3][moveNum++%4]);
            }
        }
        else
            break;
    }
    break;
    }

    pkMapUdTimer->stop();
}

//初始化获取物品窗口
void Dungeon::initialGetitemWin()
{
    //显示获得物品信息
    QFont font("Microsoft YaHei" ,12, 75);
    GetitemsShow=fac->CreateQLabel(ptr_MianWindow,120,340,600,50,"","color:orangered;background-color:transparent;",font);
    GetitemsShow->hide();
}

//初始化战斗窗口
void Dungeon::initialFightWin()
{
    QFont font("Microsoft YaHei" ,12, 75);
    QFont fontName("Microsoft YaHei" ,8, 55);
    QFont font_VS("Microsoft YaHei" ,24, 75);
    QString FightWidget_style="color:orangered;background-color:black;";
    QString Character_pic_style="border-image: url(:/info/image/information/Character.png);";
    FightWidget=new QWidget(ptr_MianWindow);
    FightWidget->setStyleSheet(FightWidget_style);
    FightWidget->setGeometry(160,160,520,320);
    //左侧：怪物
    Monster_pic=fac->CreateQLabel(FightWidget,25,60,40,40,"",FightWidget_style,fontName);
    Monster_name=fac->CreateQLabel(FightWidget,10,160,100,40,"",FightWidget_style,fontName);
    Monster_hpText=fac->CreateQLabel(FightWidget,105,20,100,40,"生命值",FightWidget_style,font);
    Monster_hp=fac->CreateQLabel(FightWidget,105,60,100,40,"",FightWidget_style,font);
    Monster_atkText=fac->CreateQLabel(FightWidget,105,120,100,40,"攻击力",FightWidget_style,font);
    Monster_atk=fac->CreateQLabel(FightWidget,105,160,100,40,"",FightWidget_style,font);
    Monster_defText=fac->CreateQLabel(FightWidget,105,220,100,40,"防御力",FightWidget_style,font);
    Monster_def=fac->CreateQLabel(FightWidget,105,260,100,40,"",FightWidget_style,font);


    Character_pic=fac->CreateQLabel(FightWidget,445,60,40,40,"",Character_pic_style,fontName);
    Character_name=fac->CreateQLabel(FightWidget,400,160,120,40,role.name,FightWidget_style,fontName);
    Character_hpText=fac->CreateQLabel(FightWidget,335,20,100,40,"生命值",FightWidget_style,font);
    Character_hp=fac->CreateQLabel(FightWidget,335,60,100,40,"",FightWidget_style,font);
    Character_atkText=fac->CreateQLabel(FightWidget,335,120,100,40,"攻击力",FightWidget_style,font);
    Character_atk=fac->CreateQLabel(FightWidget,335,160,100,40,"",FightWidget_style,font);
    Character_defText=fac->CreateQLabel(FightWidget,335,220,100,40,"防御力",FightWidget_style,font);
    Character_def=fac->CreateQLabel(FightWidget,335,260,100,40,"",FightWidget_style,font);

    QLabel *VS=fac->CreateQLabel(FightWidget,225,100,100,80,"VS",FightWidget_style,font_VS);

    FightWidget->hide();
}


//更新状态显示数据
void Dungeon::updateStatusData()
{
    //检查升级：经验值>=100升级
    if(role.exp>=100*role.level)
    {
        MAX_HP+=role.level*50;
        role.hp=MAX_HP;
        MAX_MP+=role.level*5;
        role.mp=MAX_MP;
        role.atk+=3;
        role.def+=2;
        role.level+=1;
        role.exp=0;
        //升级特效
        itemNum=-3;
        GetitemTimer->start(100);
        music->SoundPlay(1);
    }
    //更新UI显示
    score->setText(QString::number(role.scoreNum));
    level->setText(QString::number(role.level));
    exp->setText(QString::number(role.exp));
    hp->setText(QString::number(role.hp));
    mp->setText(QString::number(role.mp));
    atk->setText(QString::number(role.atk));
    def->setText(QString::number(role.def));
    PickaxeNum->setText(QString::number(role.Pickaxe));
    DownstairsNum->setText(QString::number(role.Downstairs));
    Floor->setText(QString::number(currentFloor));
}

//根据不同itemNum显示不同提示信息
void Dungeon::GetitemWinshow()
{
    //第一次显示
    if(GetitemNum==0)
    {
        switch(itemNum)
        {
        case -1://死亡复活
        {
            GetitemsShow->setText(QString::fromWCharArray(L"你因死亡失去 ") + QString::number(20*deadTimes) + QString::fromWCharArray(L" 生命上限 ")
                                                                                                                                        + QString::number(5*deadTimes)+ QString::fromWCharArray(L" 魔法上限 ")
                                                                                                                                        + QString::number(40*deadTimes)+ QString::fromWCharArray(L" 金币 "));
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case -2://获得经验和金币
        {
            GetitemsShow->setText(QString::fromWCharArray(L"获得经验值 ") + QString::number(enemy[MonsterNum].exp) + QString::fromWCharArray(L" 金币 ") + QString::number(enemy[MonsterNum].gold));
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case -3://升级
        {
            GetitemsShow->setText("Level Up!!!");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 0://平局
        {
            GetitemsShow->setText("你们谁也干不掉谁！");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 1://生命值不足
        {
            GetitemsShow->setText("你的生命值太低，需要提高才能击败目标！");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 2://胜利
        {
            GetitemsShow->setText("You Win！");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 3://失败
        {
            GetitemsShow->setText("You Lost！");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 23://小回血蘑菇
        {
            GetitemsShow->setText("获得小回血蘑菇!生命值加 100");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 24://回蓝蘑菇
        {
            GetitemsShow->setText("获得回蓝蘑菇!魔法值加 20");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        case 25://大回血蘑菇
        {
            GetitemsShow->setText("获得大回血蘑菇！增加50点生命值上限！");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
            //        case 11:
            //        {
            //            GetitemsShow->setText("突然从箱子里跳出一只绿鸭子并冲向你，你不得不与之战斗！！！");
            //            GetitemsShow->show();
            //        }
            //            break;
            //        case 12:
            //        {
            //            GetitemsShow->setText("突然从箱子里跳出一只红鸭子并飞向你，你不得不与之战斗！！！");
            //            GetitemsShow->show();
            //        }
        case 16:
        {
            GetitemsShow->setText("不能使用破墙镐，魔法值低于20！");
            GetitemsShow->show();
        }
        break;
        case 17:
        {

            GetitemsShow->setText("不能使用下楼器，魔法值低于40！");
            GetitemsShow->show();

        }
        break;
        case 18:
        {
            GetitemsShow->setText("打不开门, 缺少紫钥匙！");
            GetitemsShow->show();
        }
        break;
        case 26:
        {
            GetitemsShow->setText("获得绿龟壳，防御力加 3");
            GetitemsShow->show();
        }
        break;
        case 27:
        {
            GetitemsShow->setText("获得红龟壳，攻击力加 2");
            GetitemsShow->show();
        }
        break;
        case 28:
        {
            GetitemsShow->setText("获得铁剑，攻击力加 8");
            GetitemsShow->show();
        }
        break;
        case 29:
        {
            GetitemsShow->setText("获得花剑，攻击力加 16");
            GetitemsShow->show();
        }
        break;
        case 30:
        {
            GetitemsShow->setText("获得金剑，攻击力加 24");
            GetitemsShow->show();
        }
        break;
        case 31:
        {
            GetitemsShow->setText("获得一把破墙镐");
            GetitemsShow->show();
        }
        break;
        case 32:
        {
            GetitemsShow->setText("发现一个下楼器");
            GetitemsShow->show();
        }
        break;
        case 12:
        {
            GetitemsShow->setText("你没有足够的金币");
            GetitemsShow->show();
            GetitemsShow->raise();
        }
        break;
        }
        GetitemNum++;
    }
    else if(GetitemNum<=11)//控制显示时间
    {
        GetitemNum++;
    }
    else
    {
        GetitemNum=0;
        GetitemTimer->stop();
        GetitemsShow->hide();
    }
}

//角色死亡处理，进行处罚措施
void Dungeon::deadCall()
{
    deadTimes++;
    MAX_HP-=(20*deadTimes);
    MAX_HP=MAX_HP<0?0:MAX_HP;
    if(MAX_HP==0)
        GameOverCall();
    MAX_MP-=(5*deadTimes);
    MAX_MP=MAX_MP<0?0:MAX_MP;
    role.hp=MAX_HP/deadTimes;
    role.mp=MAX_MP;
    role.exp=0;
    role.scoreNum-=(40*deadTimes);
    if(role.scoreNum<0)
        role.scoreNum=0;
    itemNum=-1;
    GetitemTimer->start(100);
    isDead=true;
    music->SoundPlay(5);
    People->setStyleSheet("border-image: url(:/Dungeon/image/Dungeon/dead.png);");
}

//游戏结束（玩家失败）
void Dungeon::GameOverCall()
{
    if(NetModeStart)
        surface->sign->roomWidget->chatRoom->GameOverRequest();
    menu->show();
    menu->raise();
    menu->pauseEvent->setText("GAME OVER！You Lost!");
    menu->Continue->hide();
    menu->Restart->show();
    menu->ReturnRooms->hide();
    if(NetModeStart)
    {
        menu->Restart->hide();
        menu->ReturnRooms->show();
        NetModeCountDownTimer->stop();
    }
}

//游戏结束，对手失败
void Dungeon::competitorGameOverCall()
{
    if(menu)
        menu->raise();
    menu->show();
    menu->pauseEvent->setText("对方多次阵亡...You Win!");
    menu->ReturnRooms->show();
    menu->Restart->hide();
    menu->Continue->hide();
    NetModeCountDownTimer->stop();
    CmptorGameOverCallTimer->stop();
}

//单人模式下战斗界面显示
void Dungeon::FightWinshow()
{
    //pk模式
    if(currentFloor==8)
    {
        if(MonsterNum!=0)
        {
            MonsterNum=0;
            FightTimer->stop();
            return;
        }
        pkMapFightWinShow();

    }
    //普通模式
    else
    {
        //初始化战斗界面
        if (fight_period_it == 0)
        {
            isFighting=true;
            //设置怪物属性显示
            Monster_hp->setText(QString::number(enemy[MonsterNum].hp));
            Monster_atk->setText(QString::number(enemy[MonsterNum].atk));
            Monster_def->setText(QString::number(enemy[MonsterNum].def));
            //玩家属性显示
            Character_hp->setText(QString::number(role.hp));
            Character_atk->setText(QString::number(role.atk));
            Character_def->setText(QString::number(role.def));
            FightWidget->show();
            FightWidget->raise();
            //设置怪物图片和名称
            Monster_pic->setStyleSheet(MonsterPic[MonsterNum]);
            Monster_name->setText(enemy[MonsterNum].name);
            fight_period_it = 1;
            music->FightSound->play();
            music->FightSoundTimer->start(100);
        }//fight_period_it 代表对战次数，奇数次计算人物对怪物造成的伤害，偶数次计算怪物对人造成的伤害
        //玩家攻击（奇数）
        else if (fight_period_it % 2 == 1 && fight_end_it == 0)
        {
            int damage=role.atk - enemy[MonsterNum].def;
            if (enemy[MonsterNum].hp - (damage>0?damage:0) * (fight_period_it / 2 + 1) <= 0)
            {
                Monster_hp->setText(QString::number(0));//fight_period_it / 2 + 1 代表人物对怪物攻击的累计次数
                fight_end_it = 1;
            }
            else
            {
                Monster_hp->setText(QString::number(enemy[MonsterNum].hp - (damage>0?damage:0) * (fight_period_it / 2 + 1)));
                fight_period_it++;
            }

        }
        //怪物攻击
        else if (fight_period_it % 2 == 0 && fight_end_it == 0)
        {
            int damage=enemy[MonsterNum].atk - role.def;
            if(role.hp - (damage>0?damage:0) * (fight_period_it / 2)<=0)
            {
                Character_hp->setText(QString::number(0));
                fight_end_it = 1;
            }
            else
            {
                if(enemy[MonsterNum].atk > role.def)
                    Character_hp->setText(QString::number(role.hp - (damage>0?damage:0) * (fight_period_it / 2)));
                fight_period_it++;
            }
        }
        //战斗结束计算
        else
        {
            if (fight_end_it <= 2)//可以控制界面存在时长，等待动画
            {
                fight_end_it++;
            }
            else if(fight_end_it == 3)
            {
                //结算战斗结果
                int damage = calc_damage(MonsterNum);
                if(damage==-1||damage<0||role.hp-damage<=0)
                    deadCall();
                else
                {
                    role.hp -= damage;
                    role.scoreNum += enemy[MonsterNum].gold;
                    role.exp += enemy[MonsterNum].exp;
                    itemNum=-2;
                    GetitemTimer->start(100);
                }
                //隐藏战斗界面
                FightWidget->hide();
                music->FightSound->stop();
                music->FightSoundTimer->stop();
                updateStatusData();//更新战斗结果
                fight_end_it++;
            }
            else if (fight_end_it <= 5)//可以控制界面存在时长
            {
                fight_end_it++;
            }
            else //fight_end_it =6 的时候,战斗状态信息栏才隐藏，完全结束
            {
                FightTimer->stop();
                if(!isDead)
                    Move();//移动角色到怪物位置
                isFighting=false;
                fight_end_it = 0;
                fight_period_it = 0;
                MonsterNum = 0;
                isDead=false;
            }
        }
    }
}
//停止所有进行中的工作
void Dungeon::stopAllwork()
{
    fight_end_it = 0;
    fight_period_it = 0;
    FightTimer->stop();
    FightWidget->hide();
    music->FightSound->stop();
    music->FightSoundTimer->stop();
    isFighting=false;
}

//pk模式下战斗界面的显示
void Dungeon::pkMapFightWinShow()
{
    int cmptor_hp=surface->sign->roomWidget->chatRoom->hp;
    int cmptor_atk=surface->sign->roomWidget->chatRoom->atk;
    int cmptor_def=surface->sign->roomWidget->chatRoom->def;
    //初始化战斗界面
    if (fight_period_it == 0)
    {
        isFighting=true;
        //设置对手属性显示（显示在怪物位置）
        Monster_hp->setText(QString::number(cmptor_hp));
        Monster_atk->setText(QString::number(cmptor_atk));
        Monster_def->setText(QString::number(cmptor_def));
        //玩家
        Character_hp->setText(QString::number(role.hp));
        Character_atk->setText(QString::number(role.atk));
        Character_def->setText(QString::number(role.def));
        FightWidget->show();
        FightWidget->raise();
        QString Character_pic_style="border-image: url(:/info/image/information/Character.png);";
        Monster_pic->setStyleSheet(Character_pic_style);
        Monster_name->setText(surface->sign->roomWidget->chatRoom->competitoName);
        fight_period_it = 1;
        music->FightSound->play();
        music->FightSoundTimer->start(100);
    }//fight_period_it 代表对战次数，奇数次计算人物对怪物造成的伤害，偶数次计算怪物对人造成的伤害
    //玩家攻击
    else if (fight_period_it % 2 == 1 && fight_end_it == 0)
    {
        int damage=role.atk - cmptor_def;
        if (cmptor_hp - (damage>0?damage:0) * (fight_period_it / 2 + 1) <= 0)
        {
            Monster_hp->setText(QString::number(0));//fight_period_it / 2 + 1 代表人物对怪物攻击的累计次数
            fight_end_it = 1;
        }
        else
        {
            Monster_hp->setText(QString::number(cmptor_hp - (damage>0?damage:0) * (fight_period_it / 2 + 1)));
            fight_period_it++;
        }

    }
    else if (fight_period_it % 2 == 0 && fight_end_it == 0)
    {
        int damage=cmptor_atk - role.def;
        if(role.hp - (damage>0?damage:0) * (fight_period_it / 2)<=0)
        {
            Character_hp->setText(QString::number(0));
            fight_end_it = 1;
        }
        else
        {
            if(cmptor_atk > role.def)
                Character_hp->setText(QString::number(role.hp - (damage>0?damage:0) * (fight_period_it / 2)));
            fight_period_it++;
        }
    }
    else
    {
        if (fight_end_it <= 2)//可以控制界面存在时长
        {
            fight_end_it++;
        }
        else if(fight_end_it == 3)
        {
            if(cmptor_hp==role.hp&&cmptor_atk==role.atk&&cmptor_def==role.def)
            {
                dogFall=true;
                itemNum=0;
                GetitemTimer->start(100);
            }
            else
            {
                //结算战斗结果
                int damage;
                if (role.atk <= cmptor_def)
                {
                    if (cmptor_atk > role.def)
                        damage=-1;
                    else
                        damage=-2;
                }
                else
                {
                    if (cmptor_atk > role.def)
                    {
                        int atk_times = (cmptor_hp - 1) / (role.atk - cmptor_def);//怪物攻击的次数
                        damage=atk_times * (cmptor_atk - role.def);//造成的伤害
                    }
                    else
                    {
                        damage=0;//怪物破不了我们的防，造成的伤害为0
                    }
                }
                //玩家失败
                if(damage==-1||damage<0||role.hp-damage<=0)
                {
                    MyWin=false;
                    itemNum=3;
                    GetitemTimer->start(100);
                }
                //平局
                else if(damage==-2)
                {
                    dogFall=true;
                    itemNum=0;
                    GetitemTimer->start(100);
                }
                //玩家胜利
                else
                {
                    MyWin=true;
                    itemNum=2;
                    GetitemTimer->start(100);
                }
            }
            //隐藏战斗界面
            FightWidget->hide();
            music->FightSound->stop();
            music->FightSoundTimer->stop();
            fight_end_it++;
        }
        else if (fight_end_it <= 5)//可以控制界面存在时长
        {
            fight_end_it++;
        }
        else //fight_end_it =6 的时候,战斗状态信息栏才隐藏
        {
            FightTimer->stop();
            isFighting=false;
            fight_end_it = 0;
            fight_period_it = 0;
            MonsterNum = 0;
            ReturnDungeon(MyWin);
        }
    }
}
//静态贴图
bool Dungeon::isStatic(int num)
{
    return (num>=0&&num<=3)||(num>=6&&num<=13)||(num>=23&&num<=32);
}
//动态贴图
bool Dungeon::isDynamic(int floor,int i,int j)
{
    return map[floor][i][j]>=14&&map[floor][i][j]<=22;
}

//角色移动
void Dungeon::Move()
{
    switch(moveDirection)
    {
    case 0://up
    {
        x-=1;
        map[currentFloor-1][x][y]=1;//清空格子
        MazeWidget[x][y]->setStyleSheet(DungeonStytle[1][0]);//设置为路
        gLayout_Map->addWidget(People,x,y);//移动角色控件
        People->setStyleSheet(DungeonStytle[2][0]);//设置向上样式
    }
    break;
    case 1://down
    {
        x+=1;
        map[currentFloor-1][x][y]=1;
        MazeWidget[x][y]->setStyleSheet(DungeonStytle[1][0]);
        gLayout_Map->addWidget(People,x,y);
        People->setStyleSheet(DungeonStytle[3][0]);
    }
    break;
    case 2://left
    {
        y-=1;
        map[currentFloor-1][x][y]=1;
        MazeWidget[x][y]->setStyleSheet(DungeonStytle[1][0]);
        gLayout_Map->addWidget(People,x,y);
        People->setStyleSheet(DungeonStytle[4][moveNum%4]);
    }
    break;
    case 3://right
    {
        y+=1;
        map[currentFloor-1][x][y]=1;
        MazeWidget[x][y]->setStyleSheet(DungeonStytle[1][0]);
        gLayout_Map->addWidget(People,x,y);
        People->setStyleSheet(DungeonStytle[5][moveNum%4]);
    }
    break;
    }
    moveNum++;
    //楼层小于8时，更新迷雾区域
    if(currentFloor<8)
        updateFogArea(currentFloor-1,x,y);
}

//显示动态效果
void Dungeon::ShowdynamicEffect()
{
    if (display_it <= 2)
        display_it += 1;
    else
        display_it = 0;
    for(int i=0;i<12;i++)
    {
        for(int j=0;j<16;j++)
        {
            if(isDynamic(currentFloor-1,i,j))
            {
                MazeWidget[i][j]->setStyleSheet(DungeonStytle[map[currentFloor-1][i][j]][display_it]);
            }
            else
            {
                MazeWidget[i][j]->setStyleSheet(DungeonStytle[map[currentFloor-1][i][j]][0]);
            }
        }
    }

}

//处理商店购买
void Dungeon::OpenStore()
{
    if(role.scoreNum-Store->Currstore_price<0)//金币不够，不能购买
    {
        itemNum=12;//提示玩家，金币不足
        GetitemTimer->start(100);
        Store->haveBuyTimer->stop();
    }
    else
    {
        role.scoreNum-=Store->Currstore_price;
        //购买生命值
        if(Store->chooseOption==0)
        {
            MAX_HP+=100 * (Store->BuyTimes-1);
            role.hp+=100 * (Store->BuyTimes-1);
            hp->setText(QString::number(role.hp));
            score->setText(QString::number(role.scoreNum));
            if(role.scoreNum-Store->Nextstore_price<0)//判断下一次是否可以购买
            {
                Store->setChooseEnable=false;
            }
            else
            {
                Store->setChooseEnable=true;
            }
            Store->haveBuyTimer->stop();//购买完成
        }
        //魔法值
        else if(Store->chooseOption==1)
        {
            role.mp+=20 * (Store->BuyTimes-1);
            mp->setText(QString::number(role.mp));
            score->setText(QString::number(role.scoreNum));

            if(role.scoreNum-Store->Nextstore_price<0)
            {
                Store->setChooseEnable=false;
            }
            else
            {
                Store->setChooseEnable=true;
            }
            Store->haveBuyTimer->stop();
        }
        //攻击力
        else if(Store->chooseOption==2)
        {
            role.atk+=2;
            atk->setText(QString::number(role.atk));
            score->setText(QString::number(role.scoreNum));

            if(role.scoreNum-Store->Nextstore_price<0)
            {
                Store->setChooseEnable=false;
            }
            else
            {
                Store->setChooseEnable=true;
            }
            Store->haveBuyTimer->stop();
        }
        //防御力
        else if(Store->chooseOption==3)
        {
            role.def+=4;
            def->setText(QString::number(role.def));
            score->setText(QString::number(role.scoreNum));

            if(role.scoreNum-Store->Nextstore_price<0)
            {
                Store->setChooseEnable=false;
            }
            else
            {
                Store->setChooseEnable=true;
            }
            Store->haveBuyTimer->stop();
        }
        else
        {
            return;
        }

    }
    return;
}
void Dungeon::changeHP(int num)
{
    if(role.hp+num>=MAX_HP)
    {
        role.hp=MAX_HP;
    }
    else
    {
        role.hp+=num;
    }
}
void Dungeon::changeMP(int num)
{
    if(role.mp+num>=MAX_MP)
    {
        role.mp=MAX_MP;
    }
    else
    {
        role.mp+=num;
    }
}

//问号箱子
void Dungeon::QuestionBox(int num)
{
    int temp1=0;
    int temp2=0;
    if(num==0)//up
    {
        temp1=x-1;
        temp2=y;
    }
    else if(num==1)//down
    {
        temp1=x+1;
        temp2=y;
    }
    else if(num==2)//left
    {
        temp1=x;
        temp2=y-1;
    }
    else if(num==3)//right
    {
        temp1=x;
        temp2=y+1;

    }
    //弹出确认对话框
    QMessageBox message(QMessageBox::Information,"发现箱子！","箱子上写着问号，不知道里面装着什么，是否打开？",QMessageBox::Yes|QMessageBox::No,ptr_MianWindow);
    message.setIconPixmap(QPixmap(":/info/image/information/box.png"));
    message.setWindowIcon(QIcon(":/info/image/information/提示.ico"));
    message.raise();
    if (message.exec()==QMessageBox::Yes)
    {
        //随机决定箱子内容
        int Num = rand() % 10 + 23; // 扩大范围到 23-32
        if (Num > 27) Num = 26; // 如果超出，设为绿鸭子或其他
        // 为了减少绿鸭子概率，调整逻辑
        int randVal = rand() % 100;
        if (randVal < 20) Num = 23; // 20% 小回血
        else if (randVal < 40) Num = 24; // 20% 回蓝
        else if (randVal < 55) Num = 25; // 15% 大回血
        else if (randVal < 75) Num = 26; // 20% 绿鸭子
        else Num = 27; // 25% 红鸭子
        switch(Num)
        {
        case 23://小回血蘑菇
        {
            itemNum=23;
            GetitemTimer->start(100);
            changeHP(100);
            updateStatusData();
            map[currentFloor-1][temp1][temp2]=1;
            MazeWidget[temp1][temp2]->setStyleSheet(DungeonStytle[1][0]);
            return;
        }
        break;
        case 24://回蓝蘑菇
        {
            itemNum=24;
            GetitemTimer->start(100);
            changeMP(20);
            updateStatusData();
            map[currentFloor-1][temp1][temp2]=1;
            MazeWidget[temp1][temp2]->setStyleSheet(DungeonStytle[1][0]);
            return;
        }
        break;
        case 25://大回血蘑菇
        {
            itemNum=25;
            GetitemTimer->start(100);
            MAX_HP+=100;
            changeHP(100);
            updateStatusData();
            map[currentFloor-1][temp1][temp2]=1;
            MazeWidget[temp1][temp2]->setStyleSheet(DungeonStytle[1][0]);
            return;
        }
        break;
        //绿鸭子：战斗
        case 26:
        {
            QMessageBox mes(QMessageBox::NoIcon, "你打开了箱子", "突然从箱子里跳出一只绿鸭子并冲向你，你不得不与之战斗！！！");
            mes.setWindowIcon(QIcon(":/info/image/information/提示.ico"));
            mes.setIconPixmap(QPixmap(":/info/image/information/greenDuck.png"));
            mes.raise();
            mes.exec();

            //检查是否能打过
            if(pk(enemy[2])==-2)
            {
                itemNum=0;
                GetitemTimer->start(100);
                map[currentFloor-1][temp1][temp2]=1; // 将未能战胜的绿鸭子格子清空为路
                MazeWidget[temp1][temp2]->setStyleSheet(DungeonStytle[1][0]);
            }
            else
            {
                MonsterNum=2;
                FightTimer->start(50);
                moveDirection=num;
            }
            return;
        }
        break;
        case 27:
        {
            QMessageBox mes(QMessageBox::NoIcon, "你打开了箱子", "突然从箱子里跳出一只红鸭子并飞向你，你不得不与之战斗！！！");
            mes.setWindowIcon(QIcon(":/info/image/information/提示.ico"));
            mes.setIconPixmap(QPixmap(":/info/image/information/flyDuck.png"));
            mes.raise();
            mes.exec();

            if(pk(enemy[3])==-2)
            {
                itemNum=0;
                GetitemTimer->start(100);
                map[currentFloor-1][temp1][temp2]=1; // 将未能战胜的红鸭子格子清空为路
                MazeWidget[temp1][temp2]->setStyleSheet(DungeonStytle[1][0]);
            }
            else
            {
                MonsterNum=3;
                FightTimer->start(50);
                moveDirection=num;
            }
            return;
        }
        break;
        }
    }
    else
    {
        return;
    }

}
//物品拾取处理
void Dungeon::items(int _itemNum, int _moveDirection)
{
    QLabel *target=new QLabel;
    int *tempNum=nullptr;
    switch(_itemNum)
    {
    case 23://小回血蘑菇
    {
        changeHP(100);
        hp->setText(QString::number(role.hp));
        music->SoundPlay(4);
    }
    break;
    case 24://回蓝蘑菇
    {
        changeMP(20);
        mp->setText(QString::number(role.mp));
        music->SoundPlay(4);
    }
    break;
    case 25://大回血蘑菇
    {
        MAX_HP+=50;
        changeHP(50);
        hp->setText(QString::number(role.hp));
        music->SoundPlay(4);
    }
    break;
    case 31://破墙镐
    {
        target=PickaxeNum;
        tempNum=&(role.Pickaxe);
        music->SoundPlay(3);
    }
    break;
    case 32://下楼器
    {
        target=DownstairsNum;
        tempNum=&(role.Downstairs);
        music->SoundPlay(3);
    }
    break;
    case 26://绿龟壳
    {
        role.def+=3;
        def->setText(QString::number(role.def));
        music->SoundPlay(3);
    }
    break;
    case 27://红龟壳
    {
        role.atk+=2;
        atk->setText(QString::number(role.atk));
        music->SoundPlay(3);
    }
    break;
    case 28://铁剑
    {
        role.atk+=8;
        atk->setText(QString::number(role.atk));
        music->SoundPlay(1);
    }
    break;
    case 29://花剑
    {
        role.atk+=16;
        atk->setText(QString::number(role.atk));
        music->SoundPlay(1);
    }
    break;
    case 30://金剑
    {
        role.atk+=24;
        atk->setText(QString::number(role.atk));
        music->SoundPlay(1);
    }
    break;
    }
    //显示物品获取提示
    itemNum=_itemNum;
    GetitemTimer->start(100);
    //移动角色
    moveDirection=_moveDirection;
    Move();
    //如果是工具类物品，增加数量
    if(itemNum==31||itemNum==32||itemNum==21)
    {
        (*tempNum)+=1;
        target->setText(QString::number(*tempNum));
    }
}

//怪物战斗处理
void Dungeon::Monsters(int _MonsterNum,int _moveDirection)
{
    if(pk(enemy[MonsterNum])==-2)
    {
        itemNum=0;//无法破防
        GetitemTimer->start(100);
        return;
    }
    else
    {
        MonsterNum=_MonsterNum;
        FightTimer->start(50);//自动战斗定时器
        moveDirection=_moveDirection;
    }
}

//上楼
void Dungeon::checkPrefloor()
{
    if(currentFloor==1)
    {
        QMessageBox message(QMessageBox::NoIcon, "对不起！", "你不能走出地牢！");
        message.setIconPixmap(QPixmap(":/info/image/information/sorry.png"));
        message.exec();
        return;
    }
    isPre=true;
    initialCharacterPos();//初始化角色位置
    People->hide();
    currentFloor-=1;
    Floor->setText(QString::number(currentFloor));
    ShowDungeon();
    ShowFog();
    music->SoundPlay(2);//放在尾部才会音效持久
}

void Dungeon::checkNextfloor()
{
    if(currentFloor==Total_Floor)
    {
        QMessageBox message(QMessageBox::NoIcon, "你想干啥？", "这是地牢最后一层了！");
        message.setIconPixmap(QPixmap(":/info/image/information/question.png"));
        message.exec();
        return;
    }
    isNext=true;
    initialCharacterPos();
    People->hide();
    currentFloor+=1;
    Floor->setText(QString::number(currentFloor));
    ShowDungeon();
    ShowFog();
    music->SoundPlay(2);
}
//稿子
void Dungeon::breakWall(int _moveDirection)
{
    if(!PickaxeUse)
        return;
    if(role.Pickaxe==0)
        return;
    else if(role.mp<20)
    {
        itemNum=16;//魔法值不足提示
        GetitemTimer->start(100);
        return;
    }
    QMessageBox message(QMessageBox::Information,"你有破墙镐","是否使用？",QMessageBox::Yes|QMessageBox::No,ptr_MianWindow);
    message.setIconPixmap(QPixmap(":/info/image/information/Pickaxe.png"));
    message.setWindowIcon(QIcon(":/info/image/information/提示.ico"));
    if (message.exec()==QMessageBox::Yes)
    {
        role.Pickaxe--;
        role.mp-=20;
        updateStatusData();
        moveDirection=_moveDirection;
        Move();
    }
    else
        return;
}

//下楼
void Dungeon::DownstairsRun()
{
    if(role.Downstairs==0)
        return;
    else if(role.mp<40)
    {
        itemNum=17;
        GetitemTimer->start(100);
        return;
    }
    role.Downstairs--;
    role.mp-=20;
    updateStatusData();

    bool findflag=false;
    int i,j;
    for(i=0;i<12;i++)
    {
        for(j=0;j<16;j++)
        {
            if(map[currentFloor-1][i][j]==7)
            {
                findflag=true;
                break;
            }
        }
        if(findflag)
            break;
    }
    updateFogArea(currentFloor-1,i,j);
}


//增加指定区域的迷雾浓度
void Dungeon::addFogArea(int floor,int i,int j,int num)
{
    if(i>=0&&j>=0&&i<12&&j<16)
    {
        FogArr[floor][i][j]+=num;
        //限制迷雾范围（8和13以上都设为15）
        if(FogArr[floor][i][j]==8||FogArr[floor][i][j]>=13)
            FogArr[floor][i][j]=15;
        //更新迷雾显示
        QPalette palette;
        palette.setBrush(QPalette::Window,
                         QBrush(QImage(":/Dungeon/image/Dungeon/fog.png").
                                copy((FogArr[floor][i][j]/4)*54,(FogArr[floor][i][j]%4)*58,54,58)
                                ));
        FogWidget[i][j]->setPalette(palette);
    }
}

//更新玩家周围的迷雾区域
void Dungeon::updateFogArea(int floor,int i,int j)
{
    if(haveVisited[floor][i][j]==0)
    {
        addFogArea(floor,i,j,15);//当前位置迷雾全清
        addFogArea(floor,i-1,j,12);//上
        addFogArea(floor,i+1,j,3);//下
        addFogArea(floor,i,j-1,5);//左
        addFogArea(floor,i,j+1,10);//右
        //        addFogArea(floor,i-1,j-1,4);//左上
        //        addFogArea(floor,i-1,j+1,8);//右上
        //        addFogArea(floor,i+1,j-1,1);//左下
        //        addFogArea(floor,i+1,j+1,2);//右下
        haveVisited[floor][i][j]=1;
    }
}

void Dungeon::Operation(int num)
{
    bool flag;
    int _x,_y;
    //计算目标位置
    if(num==0)//up
    {
        flag=x>0;
        _x=x-1;
        _y=y;
    }
    else if(num==1)//down
    {
        flag=x<11;
        _x=x+1;
        _y=y;
    }
    else if(num==2)//left
    {
        flag=y>0;
        _x=x;
        _y=y-1;
    }
    else if(num==3)//right
    {
        flag=y<15;
        _x=x;
        _y=y+1;
    }
    else
        return;
    //pk模式
    if(currentFloor==8)
    {
        if(flag&&map[currentFloor-1][_x][_y]!=0)
        {
            //移动到对手位置，发起战斗
            if(_x==surface->sign->roomWidget->chatRoom->cur_x&&
                _y==surface->sign->roomWidget->chatRoom->cur_y)
            {
                surface->sign->roomWidget->chatRoom->FightRequest();
                //isFighting=true;
                return;
            }
            else
                surface->sign->roomWidget->chatRoom->udpkPosRequest(x,y,num);
        }
    }
    if(flag&&map[currentFloor-1][_x][_y]==0)
    {
        breakWall(num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==1)
    {
        moveDirection=num;
        Move();
    }
    else if(flag&&map[currentFloor-1][_x][_y]==6)
    {
        checkPrefloor();
    }
    else if(flag&&map[currentFloor-1][_x][_y]==7)
    {
        checkNextfloor();
    }
    else if(flag&&map[currentFloor-1][_x][_y]==12)
    {
        Store->show();
        Store->raise();
        if(role.scoreNum-Store->Currstore_price>=0)
        {
            Store->setChooseEnable=true;
        }
        Store->StoreWinisShow=true;
    }
    else if(flag&&map[currentFloor-1][_x][_y]==14)
    {
        moveDirection=num;
        Move();
        role.scoreNum+=10;
        score->setText(QString::number(role.scoreNum));
        music->SoundPlay(0);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==15)
    {
        Monsters(0,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==16)
    {
        Monsters(1,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==17)
    {
        QuestionBox(num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==18)
    {
        moveDirection=num;
        Move();
    }
    else if(flag&&map[currentFloor-1][_x][_y]==19)
    {
        moveDirection=num;
        Move();
    }
    else if(flag&&map[currentFloor-1][_x][_y]==20)
    {
        Monsters(4,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==21)
    {
        Monsters(5,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==22)
    {
        Monsters(6,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==23)
    {
        items(23,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==24)
    {
        items(24,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==25)
    {
        items(25,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==26)
    {
        items(26,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==27)
    {
        items(27,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==28)
    {
        items(28,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==29)
    {
        items(29,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==30)
    {
        items(30,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==31)
    {
        items(31,num);
    }
    else if(flag&&map[currentFloor-1][_x][_y]==32)
    {
        items(32,num);
    }
    else
    {
        return;
    }
}

//键盘事件

void Dungeon::keyPressEvent(QKeyEvent *event)
{
    if(isFighting)//keyevent 是实时检测的，只要在战斗结束后再设置isFighting为false，就可以了
    {
        return;
    }
    else if(Store->StoreWinisShow)
    {
        return;
    }
    else if(menu->MenuWinShow)
    {
        return;
    }

    if(event->key()==Qt::Key_W||event->key()==Qt::Key_Up)
    {
        if(currentFloor==8&&event->isAutoRepeat())//在pk地牢中,为了保证每次只移动一次，长按不响应
            return;
        Operation(0);
    }
    else if(event->key()==Qt::Key_S||event->key()==Qt::Key_Down)
    {
        if(currentFloor==8&&event->isAutoRepeat())
            return;
        Operation(1);
    }
    else if(event->key()==Qt::Key_A||event->key()==Qt::Key_Left)
    {
        if(currentFloor==8&&event->isAutoRepeat())
            return;
        Operation(2);
    }
    else if(event->key()==Qt::Key_D||event->key()==Qt::Key_Right)
    {
        if(currentFloor==8&&event->isAutoRepeat())
            return;
        Operation(3);
    }
    else if(event->key()==Qt::Key_P)
    {
        if(event->isAutoRepeat())
            return;
        PickaxeUse=!PickaxeUse;
    }
    else if(event->key()==Qt::Key_N)
        DownstairsRun();
    else if(event->key()==Qt::Key_Escape)
        menu->show();
    else
        return;
}

































































































































































































