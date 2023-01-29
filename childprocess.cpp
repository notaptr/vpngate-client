#include "mainwindow.h"
#include "childprocess.h"

ChildProcess::ChildProcess(QObject* ob) : QProcess(ob) {}

void ChildProcess::setupChildProcess()
{
    seteuid(euid);
    setegid(egid);
}
