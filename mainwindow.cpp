#include "mainwindow.h"
#include "ui_mainwindow.h"
// 引入必要的头文件
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ===========================
    // 1. 界面美化代码 (QSS)
    // ===========================
    this->setStyleSheet("QMainWindow { background-color: #f0f2f5; }");

    // 美化表格
    ui->tableView->setStyleSheet(
        "QTableView { "
        "   background-color: white; "
        "   alternate-background-color: #f9f9f9; "
        "   selection-background-color: #3d8ec9; "
        "   border: 1px solid #dcdcdc; "
        "   border-radius: 5px; "
        "   padding: 5px; "
        "   gridline-color: #eeeeee; "
        "}"
        "QHeaderView::section { "
        "   background-color: #eaeff3; "
        "   border: none; "
        "   padding: 5px; "
        "   font-weight: bold; "
        "}"
        );
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);

    // 美化按钮 (通用样式)
    QString btnStyle =
        "QPushButton { "
        "   background-color: white; "
        "   border: 1px solid #dcdcdc; "
        "   border-radius: 4px; "
        "   padding: 6px 12px; "
        "   font-size: 13px; "
        "   color: #333; "
        "}"
        "QPushButton:hover { "
        "   background-color: #e6f7ff; "
        "   border-color: #40a9ff; "
        "   color: #40a9ff; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #bae7ff; "
        "}";

    // 应用样式到所有按钮 (包括搜索和统计)
    ui->btnAdd->setStyleSheet(btnStyle);
    ui->btnDel->setStyleSheet(btnStyle);
    ui->btnExport->setStyleSheet(btnStyle);
    ui->btnChart->setStyleSheet(btnStyle);
    ui->btnSearch->setStyleSheet(btnStyle); // 搜索按钮也美化

    // 红色删除按钮
    ui->btnDel->setStyleSheet(
        "QPushButton { "
        "   background-color: #fff1f0; "
        "   border: 1px solid #ffa39e; "
        "   border-radius: 4px; "
        "   padding: 6px 12px; "
        "   color: #ff4d4f; "
        "}"
        "QPushButton:hover { background-color: #ff7875; color: white; }"
        );

    // ===========================
    // 2. 数据库与模型初始化
    // ===========================
    initDatabase();

    model = new QSqlTableModel(this, db);
    model->setTable("tasks");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0, true); // 隐藏ID列

    model->setHeaderData(1, Qt::Horizontal, "任务内容");
    model->setHeaderData(2, Qt::Horizontal, "截止日期");
    model->setHeaderData(3, Qt::Horizontal, "状态");

    // ===========================
    // 3. 多线程后台检测 (自动判断过期)
    // ===========================
    worker = new WorkerThread();
    // 连接信号：线程喊“检查” -> 主界面就执行 handleCheck
    connect(worker, &WorkerThread::notifyCheck, this, &MainWindow::handleCheck);
    worker->start();
}

MainWindow::~MainWindow()
{
    // 退出前安全停止线程
    if (worker->isRunning()) {
        worker->stop();
        worker->wait();
    }
    delete worker;
    delete ui;
}

void MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tasks.db");

    if (!db.open()) {
        QMessageBox::critical(this, "错误", "无法打开数据库！");
        return;
    }

    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS tasks ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "name TEXT, "
                  "date TEXT, "
                  "status TEXT)";
    if (!query.exec(sql)) {
        qDebug() << "创建表失败：" << query.lastError();
    }
}

void MainWindow::on_btnAdd_clicked()
{
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 1), "请输入任务内容");
    model->setData(model->index(row, 2), QDate::currentDate().toString("yyyy-MM-dd"));
    model->setData(model->index(row, 3), "进行中");
    model->submitAll();
}

void MainWindow::on_btnDel_clicked()
{
    int curRow = ui->tableView->currentIndex().row();
    if (curRow < 0) {
        QMessageBox::warning(this, "提示", "请先点击选中一行任务！");
        return;
    }
    int ret = QMessageBox::question(this, "确认删除", "确定要删除这条任务吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        model->removeRow(curRow);
        model->submitAll();
    }
}

void MainWindow::on_btnExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出任务", "tasks.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法保存文件！");
        return;
    }

    QTextStream out(&file);
    out << "ID,Task Content,Due Date,Status\n";

    int rowCount = model->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QString id = model->data(model->index(i, 0)).toString();
        QString name = model->data(model->index(i, 1)).toString();
        QString date = model->data(model->index(i, 2)).toString();
        QString status = model->data(model->index(i, 3)).toString();
        out << id << "," << name << "," << date << "," << status << "\n";
    }
    file.close();
    QMessageBox::information(this, "成功", "任务列表已导出！");
}

// 搜索功能
void MainWindow::on_btnSearch_clicked()
{
    QString text = ui->txtSearch->text();
    if (text.isEmpty()) {
        model->setFilter("");
    } else {
        QString filter = QString("name LIKE '%%1%'").arg(text);
        model->setFilter(filter);
    }
    model->select();
}

// 统计图表 (已更新：包含过期状态)
void MainWindow::on_btnChart_clicked()
{
    int total = model->rowCount();
    int doing = 0;
    int done = 0;
    int expired = 0; // 新增过期计数

    for (int i = 0; i < total; ++i) {
        QString status = model->data(model->index(i, 3)).toString();
        if (status == "已完成") {
            done++;
        } else if (status == "已过期") {
            expired++;
        } else {
            doing++;
        }
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("任务全景分析");
    dialog->resize(420, 320);
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // 显示详细文字统计
    QString text = QString(
                       "📊 任务数据统计\n\n"
                       "🟢 已完成： %1\n"
                       "🔵 进行中： %2\n"
                       "🔴 已过期： %3\n\n"
                       "总任务数： %4"
                       ).arg(done).arg(doing).arg(expired).arg(total);

    QLabel *label = new QLabel(text, dialog);
    QFont font;
    font.setPointSize(12); // 字号大一点
    font.setBold(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    // 进度条显示完成率
    QLabel *tip = new QLabel("总体完成率：", dialog);
    layout->addWidget(tip);

    QProgressBar *bar = new QProgressBar(dialog);
    bar->setRange(0, total);
    bar->setValue(done);
    bar->setFormat("%p%"); // 显示百分比
    bar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; }");
    layout->addWidget(bar);

    dialog->exec();
}

// 核心逻辑：自动检测日期，更新状态 (由后台线程触发)
void MainWindow::handleCheck()
{
    QDate today = QDate::currentDate();
    int rowCount = model->rowCount();
    bool hasDueTask = false;

    for (int i = 0; i < rowCount; ++i) {
        QString dateStr = model->data(model->index(i, 2)).toString();
        QString status = model->data(model->index(i, 3)).toString();

        QDate taskDate = QDate::fromString(dateStr, "yyyy-MM-dd");

        // 逻辑：如果没完成，且日期早于今天 -> 自动变“已过期”
        if (status != "已完成" && taskDate < today && status != "已过期") {
            model->setData(model->index(i, 3), "已过期");
        }

        // 逻辑：如果是今天到期 -> 提醒
        if (taskDate == today && status != "已完成") {
            hasDueTask = true;
        }
    }

    // 提交自动修改到数据库
    model->submitAll();

    // 更新标题栏提示
    if (hasDueTask) {
        this->setWindowTitle("任务管理系统 - 【有任务今天到期！】");
    } else {
        this->setWindowTitle("任务管理系统 - 运行中");
    }
}
