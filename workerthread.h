#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread();
    void stop();

protected:
    void run() override;

signals:
    void notifyCheck();

private:
    // 【修改点】改了个名字，防止和系统函数重名
    bool isWork;
};

#endif // WORKERTHREAD_H
