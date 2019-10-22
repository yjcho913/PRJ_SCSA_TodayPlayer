#include "timerthread.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

timerThread::timerThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    ui->startLabel->setText(QString("aaaaaaaaaaaaaaaaaaa"));
}

void timerThread::run()
{
    stopped = false;
}

void timerThread::stop(){
    stopped = true;
}
