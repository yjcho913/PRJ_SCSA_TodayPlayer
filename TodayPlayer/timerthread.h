#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H

#include <QThread>

class timerThread : public QThread
{
    Q_OBJECT
public:
    explicit timerThread(QObject *parent = 0);
    void run();
    void stop();
    
private:
    volatile bool stopped;

signals:
    void sendCommand(int);

public slots:
    
};

#endif // TIMERTHREAD_H
