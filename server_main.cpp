#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDir>

#define ENDFLAG '-'

struct Room {
    QString roomName;
    QString owner;
    QStringList players;
};

class GameServer : public QTcpServer
{
public:
    GameServer(QObject *parent = nullptr) : QTcpServer(parent) {
        loadUsers();  // 加载已注册用户
    }

    ~GameServer() {
        saveUsers();  // 保存用户数据
    }

    void start(quint16 port) {
        if (listen(QHostAddress::Any, port)) {
            qDebug() << "========================================";
            qDebug() << "游戏服务器启动成功！";
            qDebug() << "监听端口：" << port;
            qDebug() << "已加载用户数：" << m_users.size();
            qDebug() << "========================================";
        } else {
            qDebug() << "服务器启动失败：" << errorString();
        }
    }

    // 保存用户数据到文件
    void saveUsers() {
        QFile file("users.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (auto it = m_users.begin(); it != m_users.end(); ++it) {
                out << it.key() << ":" << it.value() << "\n";
            }
            file.close();
            qDebug() << "保存用户数据：" << m_users.size() << "个用户";
        }
    }

    // 从文件加载用户数据
    void loadUsers() {
        QFile file("users.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList parts = line.split(":");
                if (parts.size() >= 2) {
                    m_users[parts[0]] = parts[1];
                }
            }
            file.close();
            qDebug() << "加载用户数据：" << m_users.size() << "个用户";
        }
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);

        qDebug() << "新客户端连接：" << socket->peerAddress().toString();

        connect(socket, &QTcpSocket::readyRead, this, &GameServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &GameServer::onDisconnected);

        m_clients[socket] = "";
    }

private slots:
    void onReadyRead() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;

        QByteArray data = socket->readAll();
        QString msg = QString::fromUtf8(data);

        // 去掉结束标志 '-'
        if (msg.endsWith('-')) {
            msg.chop(1);
        }

        qDebug() << "收到消息：" << msg;

        // ========== 处理注册 (REG) ==========
        if (msg.startsWith("REG")) {
            QString content = msg.mid(3);
            QString username = content.section('\t', 0, 0);
            QString password = content.section('\t', 1, 1);

            qDebug() << "注册请求 - 用户名：" << username;

            if (m_users.contains(username)) {
                socket->write("The name has been registered-");
                qDebug() << "用户名已存在";
            } else {
                m_users[username] = password;
                saveUsers();  // 立即保存
                socket->write("REGISTER SUCCESS-");
                qDebug() << "注册成功：" << username;
            }
        }
        // ========== 处理登录 (LOG) ==========
        else if (msg.startsWith("LOG")) {
            QString content = msg.mid(3);
            QString username = content.section('\t', 0, 0);
            QString password = content.section('\t', 1, 1);

            qDebug() << "登录请求 - 用户名：" << username;

            if (!m_users.contains(username)) {
                socket->write("You have to register first-");
                qDebug() << "用户未注册";
            } else if (m_users[username] != password) {
                socket->write("password is wrong-");
                qDebug() << "密码错误";
            } else if (m_loggedInUsers.contains(username)) {
                socket->write("another client have Login this acount-");
                qDebug() << "账号已在其他地方登录";
            } else {
                m_loggedInUsers[username] = socket;
                m_clients[socket] = username;
                socket->write("LOG IN SUCCESS-");
                qDebug() << "登录成功：" << username;
            }
        }
        // ========== 刷新房间列表 (FRL) ==========
        else if (msg == "FRL") {
            sendRoomList(socket);
        }
        // ========== 创建房间 (CRO) ==========
        else if (msg.startsWith("CRO")) {
            QString content = msg.mid(3);
            QString roomName = content.section('\t', 0, 0);
            QString playerName = content.section('\t', 1, 1);

            if (m_rooms.contains(roomName)) {
                socket->write("duplicated name!-");
            } else if (hasRoom(playerName)) {
                socket->write("you already have a room!-");
            } else {
                Room newRoom;
                newRoom.roomName = roomName;
                newRoom.owner = playerName;
                newRoom.players.append(playerName);
                m_rooms[roomName] = newRoom;
                m_clients[socket] = playerName;

                socket->write("CraeteRoom done!-");
                qDebug() << "创建房间成功：" << roomName;

                // 延迟通知刷新房间列表
                QTimer::singleShot(10, this, [this]() {
                    notifyAllFlushRoomList();
                });
            }
        }
        // ========== 删除房间 (DRO) ==========
        else if (msg.startsWith("DRO")) {
            QString playerName = msg.mid(3);
            for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
                if (it->owner == playerName) {
                    m_rooms.erase(it);
                    socket->write("DeleteRoom done!-");
                    notifyAllFlushRoomList();
                    return;
                }
            }
            socket->write("No Room!-");
        }
        // ========== 进入房间 (ERO) ==========
        else if (msg.startsWith("ERO")) {
            QString content = msg.mid(3);
            QString roomName = content.section('\t', 0, 0);
            QString playerName = content.section('\t', 1, 1);

            if (!m_rooms.contains(roomName)) {
                socket->write("Enter Refused!-");
                return;
            }

            Room &room = m_rooms[roomName];
            if (room.players.size() >= 2) {
                socket->write("room is full-");
                return;
            }

            room.players.append(playerName);
            m_clients[socket] = playerName;

            QString response = "EnterRoom" + room.roomName + "\t" + room.owner + "\t" + playerName + "-";
            socket->write(response.toUtf8());
            qDebug() << "玩家进入房间：" << playerName;

            notifyRoomPlayers(roomName, "EnterRoom" + room.roomName + "\t" + room.owner + "\t" + playerName + "-");
            notifyAllFlushRoomList();
        }
        // ========== 离开房间 (LRO) ==========
        else if (msg.startsWith("LRO")) {
            QString playerName = msg.mid(3);
            for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
                if (it->players.contains(playerName)) {
                    it->players.removeAll(playerName);
                    notifyRoomPlayers(it->roomName, "LeaveRoom" + playerName + "-");
                    if (it->owner == playerName) {
                        m_rooms.erase(it);
                    }
                    break;
                }
            }
            notifyAllFlushRoomList();
        }
        // ========== 开始游戏 (SRQ) ==========
        else if (msg.startsWith("SRQ")) {
            QString content = msg.mid(3);
            QString playerName = content.section('\t', 0, 0);
            QString roomName = content.section('\t', 1, 1);
            if (m_rooms.contains(roomName) && m_rooms[roomName].owner == playerName) {
                notifyRoomPlayers(roomName, "游戏开始-");
                qDebug() << "游戏开始：" << roomName;
            }
        }
        else if (msg.startsWith("RRQ") || msg.startsWith("IRQ") || msg.startsWith("MRQ") ||
                 msg.startsWith("PRQ") || msg.startsWith("URQ") || msg.startsWith("USQ") ||
                 msg.startsWith("FRQ") || msg.startsWith("KRQ") || msg.startsWith("GRQ") || msg.startsWith("MES")) {
            notifyRoomPlayersByPlayer(m_clients[socket], msg + "-");
        }
        else if (msg.startsWith("YRQ")) {
            QString playerName = msg.mid(3);
            qDebug() << "玩家初始化：" << playerName;
        }
        else if (msg.startsWith("ORQ")) {
            QString playerName = msg.mid(3);
            qDebug() << "玩家离线：" << playerName;
        }
        else {
            socket->write(("OK-" + msg).toUtf8());
        }
    }

    void onDisconnected() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;

        QString playerName = m_clients[socket];
        qDebug() << "客户端断开：" << playerName;

        if (m_loggedInUsers.contains(playerName)) {
            m_loggedInUsers.remove(playerName);
        }

        for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
            if (it->players.contains(playerName)) {
                it->players.removeAll(playerName);
                notifyRoomPlayers(it->roomName, "LeaveRoom" + playerName + "-");
                if (it->owner == playerName) {
                    m_rooms.erase(it);
                }
                break;
            }
        }

        m_clients.remove(socket);
        socket->deleteLater();
        notifyAllFlushRoomList();
    }

private:
    void sendRoomList(QTcpSocket *socket) {
        if (m_rooms.isEmpty()) {
            socket->write("rooms empty!-");
            return;
        }

        QStringList roomInfo;
        for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
            roomInfo.append(it->roomName + ":" + it->owner);
        }

        QString response = QString::number(roomInfo.size()) + "#";
        for (const QString &info : roomInfo) {
            response += info + "#";
        }
        response += "-";
        socket->write(response.toUtf8());
    }

    void notifyRoomPlayers(const QString &roomName, const QString &msg) {
        if (!m_rooms.contains(roomName)) return;

        Room &room = m_rooms[roomName];
        for (QTcpSocket *client : m_clients.keys()) {
            if (room.players.contains(m_clients[client])) {
                client->write(msg.toUtf8());
            }
        }
    }

    void notifyRoomPlayersByPlayer(const QString &playerName, const QString &msg) {
        for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
            if (it->players.contains(playerName)) {
                for (QTcpSocket *client : m_clients.keys()) {
                    if (it->players.contains(m_clients[client]) && m_clients[client] != playerName) {
                        client->write(msg.toUtf8());
                    }
                }
                break;
            }
        }
    }

    void notifyAllFlushRoomList() {
        for (QTcpSocket *client : m_clients.keys()) {
            client->write("FRL-");
        }
    }

    bool hasRoom(const QString &playerName) {
        for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
            if (it->owner == playerName || it->players.contains(playerName)) {
                return true;
            }
        }
        return false;
    }

private:
    QMap<QTcpSocket*, QString> m_clients;
    QMap<QString, Room> m_rooms;
    QMap<QString, QString> m_users;
    QMap<QString, QTcpSocket*> m_loggedInUsers;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    GameServer server;
    server.start(2021);

    return a.exec();
}
