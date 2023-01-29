#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QString"
#include "QMenu"
#include "QDate"
#include "QTextStream"
#include "QProcess"
#include "QTemporaryFile"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "childprocess.h"

/*
 *  1 HostName
 *  2 IP
 *  3 Score
 *  4 Ping
 *  5 Speed
 *  6 CountryLong
 *  7 CountryShort
 *  8 NumVpnSessions
 *  9 Uptime
 * 10 TotalUsers
 * 11 TotalTraffic
 * 12 LogType
 * 13 Operator
 * 14 Message
 * 15 OpenVPN_ConfigData_Base64
 */

#define COL_COUNT 11
#define MEGA 1048576

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)


{

    ui->setupUi(this);

    setWindowTitle(BUILD);

    ui->tableWidget->setColumnCount(COL_COUNT);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList({"Узел","IP","Протокол","Порт","Страна","Счёт","Пинг","Скорость","Конфигурация","Страна_Кратко", "Ключ"}));
    ui->tableWidget->setColumnHidden(8, true);
    ui->tableWidget->setColumnHidden(9, true);
    ui->tableWidget->setColumnHidden(10, true);

    ui->tableWidget->setColumnWidth(0, 130);
    ui->tableWidget->setColumnWidth(1, 128);
    ui->tableWidget->setColumnWidth(2, 80);
    ui->tableWidget->setColumnWidth(3, 60);
    ui->tableWidget->setColumnWidth(4, 137);
    ui->tableWidget->setColumnWidth(5, 100);
    ui->tableWidget->setColumnWidth(6, 100);
    ui->tableWidget->setColumnWidth(7, 104);

    ui->lineEdit->setText(QDir::currentPath());
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(loadFile()));
    connect(ui->bFileSelect, SIGNAL(clicked()), this, SLOT(selectFile()));

    tf = new QTemporaryFile("vpnconfig");

}


MainWindow::~MainWindow()
{
    tf->close();
    delete ui;
}


void MainWindow::selectFile()
{

    QString fl = QFileDialog::getOpenFileName(this, "Укажите файл данных", QDir::currentPath(), "vpngate*.csv");

    if (!fl.isNull()) {

        QFileInfo fi(fl);
        QDir::setCurrent(fi.absolutePath());
        ui->lineEdit->setText(fl);

    }

}

void MainWindow::parse(char* buf, QMap<fields, QString> *mp) {
    int field = 0;
    int pos = 0;
    QString out = "";
    char ch = '0';
    qint64 sp = 0;

    while (true) {

        ch = buf[pos];

        if (ch == ',' || ch == '\n') {

                if (fields(field) == fields::SPEED) {
                    sp = out.toLongLong();
                    sp = sp / MEGA;
                    out = "%1 Mbit";
                    out = out.arg(sp);
                }

                mp->insert(fields(field), QString(out));
                field++;
                out = "";
                pos++;

                if (ch == '\n') {
                    break;
                }
                continue;
        }

        out = out + ch;
        pos++;

    }

}

void MainWindow::getiport(QString* conf, QMap<fields, QString>* vpn) {
    QStringList sl = conf->split('\n', QString::SkipEmptyParts, Qt::CaseInsensitive);
    QString line;

    //yes, strings may need to be trimmed..
    foreach (line, sl) {

       line.remove(0x0d);
       line.remove(0x0a);

       if (line.left(5) == "proto") {
           QStringList proto = line.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive);
           vpn->insert(fields::PROTO, proto[1]);
       }

       if (line.left(6) == "remote") {
           QStringList port = line.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive);
           vpn->insert(fields::PORT, port[2]);
       }

    }

}

void MainWindow::loadFile()
{

    QFile fl(ui->lineEdit->text());

    QMap<fields, QString> mp, vpn;
    Qt::ItemFlags ifl = 0;
    QTableWidgetItem* twi = nullptr;
    int rc = 0;
    qint64 lln = 0;
    QString conf = "";
    QString fn, cs, ip, pr, pt;

    if (fl.open(QFile::ReadOnly)) {

        //ui->tableWidget->
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setSortingEnabled(false);

        char buf[65535]; //some strings long, it'll be quite enouth
        while (true) {

            lln = fl.readLine(buf, sizeof(buf));
            if (lln <= 0) {break;}

            if (buf[0] == '#' || buf[0] == '*') {continue;}

            mp.clear();
            parse(buf, &mp);

            conf = (QString)QByteArray::fromBase64(mp[fields::CONFIG].toUtf8());
            vpn.clear();
            getiport(&conf, &vpn);

            rc = ui->tableWidget->rowCount();
            rc++;
            ui->tableWidget->setRowCount(rc);

            //enum fields{HOST, IP, PROTO, PORT, SCORE, PING, SPEED, CL, CS, NVS, UPT, TU, TTR, LT, OP, MESS, CONFIG};
            //   0     1       2        3       4        5     6        7            8              9            10
            //{"Узел","IP","Протокол","Порт","Страна","Счёт","Пинг","Скорость","Конфигурация","Страна_Кратко", "Ключ"}

            twi = new QTableWidgetItem(mp[fields::HOST]);
            ui->tableWidget->setItem(rc-1, 0, twi);
            ifl = twi->flags();
            twi->setFlags((ifl | Qt::ItemIsUserCheckable) ^ Qt::ItemIsEditable);
            twi->setCheckState(Qt::Unchecked);

            //cs, ip, pr, pt
            cs = mp[fields::CS].toUpper();
            ip = mp[fields::IP];
            pr = vpn[fields::PROTO].toLower();
            pt = vpn[fields::PORT];

            ui->tableWidget->setItem(rc-1, 1, new QTableWidgetItem(ip));
            ui->tableWidget->setItem(rc-1, 2, new QTableWidgetItem(pr));
            ui->tableWidget->setItem(rc-1, 3, new QTableWidgetItem(pt));
            ui->tableWidget->setItem(rc-1, 4, new QTableWidgetItem(mp[fields::CL]));
            ui->tableWidget->setItem(rc-1, 5, new QTableWidgetItem(mp[fields::SCORE]));
            ui->tableWidget->setItem(rc-1, 6, new QTableWidgetItem(mp[fields::PING]));
            ui->tableWidget->setItem(rc-1, 7, new QTableWidgetItem(mp[fields::SPEED]));
            ui->tableWidget->setItem(rc-1, 8, new QTableWidgetItem(conf));
            ui->tableWidget->setItem(rc-1, 9, new QTableWidgetItem(cs));

            fn = "";
            fn.append(cs)
                    .append("_")
                    .append(ip)
                    .append("_")
                    .append(pr)
                    .append("_")
                    .append(pt)
                    .append(".ovpn");

            ui->tableWidget->setItem(rc-1, 10, new QTableWidgetItem(fn));

            for (int i = 1; i < COL_COUNT; i++) {

                ui->tableWidget->item(rc-1, i)->setFlags(ifl ^ Qt::ItemIsEditable);

            }

        }

        ui->tableWidget->setSortingEnabled(true);

        markUnloaded();

    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл", QMessageBox::Ok);
        return;
    }

    fl.close();

}

void MainWindow::markUnloaded() {

    QString cp = QDir::currentPath();
    QString fn, cs, cf;
    int rc = ui->tableWidget->rowCount();
    int cc = ui->tableWidget->columnCount();
    Qt::ItemFlags ifl = 0;

    for (int i= 0; i < rc; i++) {

        fn = "";

        cs = ui->tableWidget->item(i, 10)->text();
        cf = ui->tableWidget->item(i, 8)->text();

        fn = fn.append(cp)
                .append("/")
                .append(cs);

        QFile fl(fn);
        if (fl.exists()) {

            for (int k = 0; k < cc; k++) {

                ui->tableWidget->item(i, k)->setTextColor(QColor(Qt::green));

            }

            ifl = ui->tableWidget->item(i, 0)->flags();
            ui->tableWidget->item(i, 0)->setCheckState(Qt::Unchecked);
            ui->tableWidget->item(i, 0)->setFlags(ifl ^ Qt::ItemIsUserCheckable);

        }

    }

}


//KR_121.128.238.46_tcp_995.ovpn
void MainWindow::doUnload() {

    QString cp = QDir::currentPath();
    QString fn, cs, cf;
    int rc = ui->tableWidget->rowCount();

    for (int i= 0; i < rc; i++) {

        if (ui->tableWidget->item(i, 0)->checkState() != Qt::Checked) {
            continue;
        }

        fn = "";

        cs = ui->tableWidget->item(i, 10)->text();
        cf = ui->tableWidget->item(i, 8)->text();

        fn = fn.append(cp)
                .append("/")
                .append(cs);

        QFile fl(fn);
        if (fl.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {

            fl.write(cf.toUtf8());
            fl.close();

        }

    }

    markUnloaded();

}

// TODO Проверка на административные права
//      Диалог запроса прав
//
void MainWindow::startVPN() {

    if (openvpn != nullptr) {

        QMessageBox::critical(this, "Ошибка", "Процесс openvpn уже запущен!", QMessageBox::Ok);
        return;
    }

    QList<QTableWidgetItem *> twi_l = ui->tableWidget->selectedItems();
    QTableWidgetItem* twi = twi_l[0];
    if (twi == nullptr) { return; }

    QString cf = ui->tableWidget->item(twi->row(), 8)->text();

    //создаём временный файл и запускаем с ним openvpn
    tf->open();

    tf->write(cf.toUtf8());
    tf->flush();
    tf->close();

    openvpn = new ChildProcess(parent());
    openvpn->euid = geteuid();
    openvpn->egid = getegid();
    openvpn->start("openvpn", QStringList() << tf->fileName());
    openvpn->waitForStarted();

    timer = startTimer(500);

    ui->tabWidget->setCurrentIndex(tabs::OPENVPN);
}

void MainWindow::timerEvent(QTimerEvent* te) {

    char buff[1024] = {0};

    qint64 nn = openvpn->read((char*)&buff, sizeof(buff));

    if (nn > 0) {
        buff[nn] = 0;
        ui->plainTextEdit->appendPlainText(QString::fromUtf8(buff, nn));
    }


}

void MainWindow::doDownload() {

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
             this, &MainWindow::onHTTPrq);

    //*** v0.3.2 замена сервера с базой серверов *** {
    //QNetworkReply* nr = manager->get(QNetworkRequest(QUrl("http://130.158.75.35/api/iphone/")));
    QNetworkReply* nr = manager->get(QNetworkRequest(QUrl("https://www.vpngate.net/api/iphone/")));
    //*** v0.3.2 замена сервера с базой серверов *** }

    connect(nr, &QNetworkReply::downloadProgress,
             this, &MainWindow::onDownProgr);

    ui->statusbar->showMessage("Отправили запрос, пробуем загрузить...", 30000);


}

void MainWindow::onDownProgr(qint64 rcv, qint64 bt) {

    QString out = "Загружено %1 кбайт";
    ui->statusbar->showMessage(out.arg(rcv/1024));

}

void MainWindow::onHTTPrq(QNetworkReply* nr) {

    if (nr->error() != QNetworkReply::NoError) {

        ui->statusbar->showMessage(nr->errorString(), 10000);

    } else {

        ui->statusbar->showMessage("Загрузили, обрабатываем...", 2000);

        //vpngate_2021-02-15.csv
        QString cp = QDir::currentPath(); //{v0.2}
        QString fn = "vpngate_" + QDate::currentDate().toString("yyyy-MM-dd") + ".csv";

        QFile fl(cp + "/" + fn);

        fl.open(QIODevice::WriteOnly);
        fl.write(nr->readAll());
        fl.flush();
        fl.close();

        ui->statusbar->showMessage("Файл успешно загружен!", 3000);

        ui->lineEdit->setText(fl.fileName());

    }

    nr->deleteLater();
}

void MainWindow::onTabCMenu(const QPoint &pos)
{
    QMenu menu(ui->centralwidget);
    menu.addAction("Выгрузить выбранные конфигурации", this, SLOT(doUnload()));
    menu.addAction("Запустить OpenVPN", this, SLOT(startVPN()));
    menu.addSeparator();
    menu.addAction("Загрузить сервера из интернета", this, SLOT(doDownload()));
    menu.exec(QCursor::pos());
}

void MainWindow::disconnect()
{
    if (openvpn == nullptr) { return; }

    killTimer(timer);
    openvpn->terminate();
    openvpn->waitForFinished();
    openvpn = nullptr;

    tf->remove();
    ui->plainTextEdit->clear();
    ui->tabWidget->setCurrentIndex(tabs::VPNGATE);
}

