#ifndef CHILDPROCESS_H
#define CHILDPROCESS_H

#include <QProcess>
#include "mainwindow.h"
#include <unistd.h>
#include <stdio.h>

class MainWindow;

class ChildProcess : public QProcess
{
    Q_OBJECT

private:

public:
    ChildProcess(QObject* ob);
    uid_t euid;
    uid_t egid;

protected:
    void setupChildProcess() override;
};

#endif // CHILDPROCESS_H
