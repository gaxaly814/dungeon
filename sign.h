#ifndef SIGN_H
#define SIGN_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "request.h"
#include "factory.h"
#include "rooms.h"
class Sign: public QDialog
{
    Q_OBJECT
public:
    explicit Sign(QWidget*parent=nullptr);
    QTcpSocket* getTcpSocket() { return tcpSocket; }
    rooms *roomWidget;
protected:
    void showEvent(QShowEvent* event) override;
private:
    void initialSignWidget();
    factory* fac;
    QTcpSocket* tcpSocket;
    bool isConnected;
    QGridLayout *mainLayout;
    QLabel *nameLabel;
    QLabel *passwordLabel;
    QLabel *statusLabel;
    QLineEdit *nameLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *registerBtn;
    QPushButton *LogInBtn;
    QString username;
private slots:
    void slotRegister();
    void slotLogIn();
    void dataReceived();
    void socketConnected();
    void socketError(QAbstractSocket::SocketError socketError);
};

#endif // SIGN_H
