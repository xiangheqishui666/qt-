#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QSqlError>
#include <QMainWindow>
// 引入必要的头文件
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
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
    // 这里后面会放按钮点击的函数
    void on_btnAdd_clicked();
    void on_btnDel_clicked();
    void on_btnExport_clicked();

private:
    Ui::MainWindow *ui;

    // 定义数据库连接
    QSqlDatabase db;
    // 定义数据模型（Model），它是连接数据库和界面的桥梁
    QSqlTableModel *model;

    // 初始化数据库的函数
    void initDatabase();
};
#endif // MAINWINDOW_H
