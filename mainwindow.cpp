#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化数据库
    initDatabase();

    // 2. 设置表格模型
    // Model/View 核心：创建模型，关联数据库表
    model = new QSqlTableModel(this, db);
    model->setTable("tasks"); // 指定要显示的表名
    model->setEditStrategy(QSqlTableModel::OnFieldChange); // 允许直接修改
    model->select(); // 查询数据

    // 3. 把模型给到界面上的表格控件
    ui->tableView->setModel(model);

    // 隐藏第一列 ID (通常用户不需要看ID)
    ui->tableView->setColumnHidden(0, true);

    // 设置一下表头名字（让界面好看点）
    model->setHeaderData(1, Qt::Horizontal, "任务内容");
    model->setHeaderData(2, Qt::Horizontal, "截止日期");
    model->setHeaderData(3, Qt::Horizontal, "状态");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 初始化数据库的具体实现
void MainWindow::initDatabase()
{
    // 添加 SQLite 数据库驱动
    db = QSqlDatabase::addDatabase("QSQLITE");
    // 设置数据库文件名为 tasks.db
    db.setDatabaseName("tasks.db");

    if (!db.open()) {
        QMessageBox::critical(this, "错误", "无法打开数据库！");
        return;
    }

    // 创建数据表（如果不存在的话）
    // 包含字段：id(主键), name(任务名), date(日期), status(状态)
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
    // 1. 获取当前共有多少行
    int row = model->rowCount();

    // 2. 在最后插入一行空白记录
    model->insertRow(row);

    // 3. 设置默认值（为了方便用户，默认填好日期和状态）
    model->setData(model->index(row, 1), "请输入任务内容"); // 第1列是任务名
    model->setData(model->index(row, 2), QDate::currentDate().toString("yyyy-MM-dd")); // 第2列是日期
    model->setData(model->index(row, 3), "进行中"); // 第3列是状态

    // 4. 立即提交到数据库
    model->submitAll();
}
void MainWindow::on_btnDel_clicked()
{
    // 1. 获取当前选中的行号
    int curRow = ui->tableView->currentIndex().row();

    // 2. 如果没有选中任何行，弹窗提示
    if (curRow < 0) {
        QMessageBox::warning(this, "提示", "请先点击选中一行任务！");
        return;
    }

    // 3. 询问是否确认删除
    int ret = QMessageBox::question(this, "确认删除", "确定要删除这条任务吗？",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // 4. 删除该行并提交
        model->removeRow(curRow);
        model->submitAll();
    }
}
void MainWindow::on_btnExport_clicked()
{
    // 1. 弹出保存文件对话框，让用户选择保存位置
    // 默认文件名为 tasks.csv
    QString fileName = QFileDialog::getSaveFileName(this, "导出任务", "tasks.csv", "CSV Files (*.csv)");

    // 如果用户点了取消（没有文件名），就直接退出，啥也不做
    if (fileName.isEmpty()) {
        return;
    }

    // 2. 尝试打开文件准备写入
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法保存文件！可能文件正在被占用。");
        return;
    }

    // 3. 使用文本流往文件里写字
    QTextStream out(&file);

    // 写入表头 (用英文逗号隔开，Excel能识别)
    out << "ID,Task Content,Due Date,Status\n";

    // 4. 遍历表格里所有行，一行行写进去
    int rowCount = model->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        // 获取每一列的数据
        QString id = model->data(model->index(i, 0)).toString();
        QString name = model->data(model->index(i, 1)).toString();
        QString date = model->data(model->index(i, 2)).toString();
        QString status = model->data(model->index(i, 3)).toString();

        // 写入文件
        out << id << "," << name << "," << date << "," << status << "\n";
    }

    // 5. 关闭文件并提示成功
    file.close();
    QMessageBox::information(this, "成功", "任务列表已成功导出到：" + fileName);
}
