#include "mainwindow.h"
#include "ui_mainwindow.h"
// 引入图表相关的头文件
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // === 界面美化代码 (QSS) 开始 ===
    // 1. 设置整体背景色 (淡雅的灰白)
    this->setStyleSheet("QMainWindow { background-color: #f0f2f5; }");

    // 2. 美化表格
    ui->tableView->setStyleSheet(
        "QTableView { "
        "   background-color: white; "
        "   alternate-background-color: #f9f9f9; " // 交替行颜色
        "   selection-background-color: #3d8ec9; " // 选中颜色(蓝色)
        "   border: 1px solid #dcdcdc; "
        "   border-radius: 5px; "
        "   padding: 5px; "
        "   gridline-color: #eeeeee; "
        "}"
        "QHeaderView::section { " // 表头样式
        "   background-color: #eaeff3; "
        "   border: none; "
        "   padding: 5px; "
        "   font-weight: bold; "
        "}"
        );
    ui->tableView->setAlternatingRowColors(true); // 开启隔行变色
    ui->tableView->verticalHeader()->setVisible(false); // 隐藏左边那个丑丑的行号

    // 3. 美化按钮 (通用样式)
    QString btnStyle =
        "QPushButton { "
        "   background-color: white; "
        "   border: 1px solid #dcdcdc; "
        "   border-radius: 4px; "
        "   padding: 6px 12px; "
        "   font-size: 13px; "
        "   color: #333; "
        "}"
        "QPushButton:hover { " // 鼠标悬停变色
        "   background-color: #e6f7ff; "
        "   border-color: #40a9ff; "
        "   color: #40a9ff; "
        "}"
        "QPushButton:pressed { " // 按下变色
        "   background-color: #bae7ff; "
        "}";

    // 给每个按钮应用样式
    ui->btnAdd->setStyleSheet(btnStyle);
    ui->btnDel->setStyleSheet(btnStyle);
    ui->btnExport->setStyleSheet(btnStyle);
    ui->btnChart->setStyleSheet(btnStyle);

    // 给“删除”按钮单独搞个红色，显眼一点
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
    // === 界面美化代码 结束 ===
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

void MainWindow::on_btnChart_clicked()
{
    // 1. 统计数据
    int total = model->rowCount();
    int doing = 0;
    int done = 0;

    for (int i = 0; i < total; ++i) {
        QString status = model->data(model->index(i, 3)).toString();
        if (status == "已完成") {
            done++;
        } else {
            doing++;
        }
    }

    // 2. 创建一个新窗口 (QDialog)
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("任务进度分析");
    dialog->resize(400, 300);

    // 3. 创建布局
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // 4. 显示文字统计
    QString text = QString("总任务数：%1\n\n进行中：%2\n已完成：%3")
                       .arg(total).arg(doing).arg(done);
    QLabel *label = new QLabel(text, dialog);
    // 设置字体大一点，居中显示
    QFont font;
    font.setPointSize(14);
    font.setBold(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    // 5. 加一个漂亮的进度条
    QProgressBar *bar = new QProgressBar(dialog);
    bar->setRange(0, total); // 范围从 0 到 总数
    bar->setValue(done);     // 当前进度是“已完成”的数量
    bar->setFormat("完成率 %p%"); // 显示百分比
    // 给进度条设置一点样式（绿色）
    bar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; } QProgressBar { text-align: center; }");
    layout->addWidget(bar);

    // 6. 显示窗口
    dialog->exec();
}

void MainWindow::on_btnSearch_clicked()
{
    // 1. 获取输入框里的字
    QString text = ui->txtSearch->text();

    // 2. 如果是空的，就显示所有数据
    if (text.isEmpty()) {
        model->setFilter(""); // 清空过滤器
    }
    else {
        // 3. 这里的语法是 SQL 语句： WHERE name LIKE '%关键词%'
        // 意思是：查找名字里包含这个字的
        QString filter = QString("name LIKE '%%1%'").arg(text);
        model->setFilter(filter);
    }

    // 4. 刷新表格
    model->select();
}
