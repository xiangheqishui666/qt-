#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
// 引入必要的库
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

// 【关键】引入线程头文件
#include "workerthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 按钮的槽函数
    void on_btnAdd_clicked();
    void on_btnDel_clicked();
    void on_btnExport_clicked();
    void on_btnChart_clicked();  // 统计图表
    void on_btnSearch_clicked(); // 搜索功能

    // 【关键】接收线程信号的槽函数（必须声明！）
    void handleCheck();

private:
    Ui::MainWindow *ui;

    // 数据库相关
    QSqlDatabase db;
    QSqlTableModel *model;

    // 【关键】线程对象指针（必须声明！）
    WorkerThread *worker;

    // 初始化数据库函数
    void initDatabase();
};
#endif // MAINWINDOW_H
