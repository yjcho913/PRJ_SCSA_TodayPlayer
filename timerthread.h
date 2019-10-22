#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H

#include <QThread>

class TimerThread : public QThread
{
    Q_OBJECT
public:
    explicit TimerThread(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // TIMERTHREAD_H
