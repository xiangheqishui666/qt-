#include "workerthread.h"

WorkerThread::WorkerThread()
{
    // 【修改点】初始化 isWork
    isWork = true;
}

void WorkerThread::stop()
{
    // 【修改点】修改标志位
    isWork = false;
}

void WorkerThread::run()
{
    // 【修改点】循环判断 isWork
    while (isWork) {
        // 每隔 5 秒醒来一次
        sleep(5);

        // 发送信号给主界面
        emit notifyCheck();
    }
}
