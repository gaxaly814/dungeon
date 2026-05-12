#ifndef REQUEST_H
#define REQUEST_H

#define SERVER_IP "127.0.0.1"//服务器IP（本地测试用）
#define SERVER_PORT 2021//服务器端口号
#define ENDFLAG '-'//消息结束标志
//用户认证相关
#define REGISTER    "REG"
#define LOGIN       "LOG"
//房间管理
#define FLUSHROOMLIST "FRL" //刷新房间列表
#define CREATEROOM  "CRO"
#define DELETEROOM "DRO"
#define ENTERROOM   "ERO"
#define LEAVEROOM "LRO"
//聊天
#define MESSAGE "MES"
//游戏准备
#define STARTRQ "SRQ"//开始游戏请求
#define READYRQ "RRQ"//准备就绪请求
//地图
#define MAPRQ   "MRQ"
//战斗
#define PKRQ    "PRQ"
#define UDPKPOSRQ "URQ"
#define UDPKSTATUSRQ "USQ"
#define FIGHTRQ "FRQ"
#define PKRESRQ "KRQ"
#define GAMEOVERRQ  "GRQ"
//初始化
#define INITREQDYRQ "IRQ" //初始化准备就绪
#define INITPLAYERRQ "YRQ"
#define OFFLINERQ   "ORQ"//离线通知

#endif // REQUEST_H
