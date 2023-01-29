#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QTimer>
#include <QProcess>
#include <QTemporaryFile>
#include <QGridLayout>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include "childprocess.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define BUILD "vpngate клиент v0.3.2 (c) 2021-2022 atlast [Qt 5.15.2]"

class ChildProcess;
enum fields{HOST, IP, SCORE, PING, SPEED, CL, CS, NVS, UPT, TU, TTR, LT, OP, MESS, CONFIG, PROTO, PORT};
enum tabs{VPNGATE, OPENVPN};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    int pp[2] = {0};
    int timer = 0;
    pid_t child_pid = 0;
    ChildProcess* openvpn = nullptr;
    QTemporaryFile* tf;
    Ui::MainWindow *ui;
    void parse(char*, QMap<fields, QString> *);
    void getiport(QString *, QMap<fields, QString> *);
    void markUnloaded();
    void timerEvent(QTimerEvent* event) override;

private slots:
    void loadFile();
    void selectFile();
    void onTabCMenu(const QPoint &);
    void startVPN();
    void doUnload();
    void doDownload();
    void disconnect();
    void onHTTPrq(QNetworkReply*);
    void onDownProgr(qint64, qint64);
};
#endif // MAINWINDOW_H
