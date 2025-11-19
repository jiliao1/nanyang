#include "signup.h"
#include "ui_signup.h"
static bool opened = false;

signup::signup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::signup)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    ui->btn_show->setIcon(QIcon("D:/nm/neimeng03/icons/hide.png"));
    ui->btn_show->setStyleSheet("border-radius: 2px");
    OpenDB();
    connect(ui->btn_back, SIGNAL(clicked()), this, SLOT(back()));
    connect(ui->btn_signup, SIGNAL(clicked()), this, SLOT(useregister()));
    connect(ui->btn_show, SIGNAL(pressed()), this, SLOT(show_pressed()));
    connect(ui->btn_show, SIGNAL(released()), this, SLOT(show_released()));
}

signup::~signup()
{
    delete ui;
}

void signup::back()
{
    ui->le_name->clear();
    ui->le_username->clear();
    ui->le_password->clear();
    ui->le_pwagain->clear();
    this->close();
    emit isTimeToShow();
}

void signup::useregister()
{
    QString name = ui->le_name->text(); //姓名
    QString username = ui->le_username->text(); //用户名
    QString password = ui->le_password->text();
    QString pw_again = ui->le_pwagain->text();
    QString situation = ("正常");
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    static int count = 0;
    QString id = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString::number(count++);
    QString purview = ui->comboBox->currentText();
    QSqlQuery query(db);
    bool tt1, tt2, tt3, tt4, tt5;
    tt1 = true;
    tt2 = true;
    tt3 = true;
    tt4 = true;
    tt5 = true;
    bool tttt = false;
    if (name.isEmpty()) {
        QMessageBox::warning(this, ("注册失败"), ("姓名不能为空！"), ("确认"));
        ui->le_name->setFocus();
        tt1 = false;
        return;
    }
    if (username.isEmpty()) {
        QMessageBox::warning(this, ("注册失败"), ("用户名不能为空！"), ("确认"));
        ui->le_username->setFocus();
        tt2 = false;
        return;
    }
    if (password.isEmpty()) {
        QMessageBox::warning(this, ("注册失败"), ("密码不能为空！"), ("确认"));
        ui->le_password->setFocus();
        tt3 = false;
        return;
    }
    if (pw_again.isEmpty()) {
        QMessageBox::warning(this, ("注册失败"), ("确认密码不能为空！"), ("确认"));
        ui->le_pwagain->setFocus();
        tt4 = false;
        return;
    }

    if (password != pw_again) {
        QMessageBox::warning(this, ("注册失败"), ("两次输入的密码不一致！请重新输入！"), ("确认"));
        ui->le_password->clear();
        ui->le_pwagain->clear();
        ui->le_password->setFocus();
        tt5 = false;
        return;
    }

    if (tt1 && tt2 && tt3 && tt4 & tt5) {
        query.exec("select username from user");
        while (query.next()) {
            QString uid = query.value(0).toString();
            qDebug() << uid;
            if (username == uid) {
                tttt = true;
                QMessageBox::warning(this, ("注册失败"), ("用户名已存在！请重新输入！"), ("确认"));
                ui->le_username->clear();
                ui->le_password->clear();
                ui->le_pwagain->clear();
                ui->le_username->setFocus();
                return;
            }
        }
        if (tttt == false) {
            query.clear();
            query.prepare("insert into user (username, password, name,purview,situation,time) values(?,?,?,?,?,?)");
            query.addBindValue(username);
            query.addBindValue(password);
            query.addBindValue(name);
            query.addBindValue(purview);
            query.addBindValue(situation);
            query.addBindValue(time);
            qDebug() << query.boundValues();
            if (!query.exec()) {
                QMessageBox::warning(this, ("注册失败"), ("创建账号失败！请重试！"), ("确认"));
                qDebug() << query.lastError();
                return;
            }
            else {
                QMessageBox::information(this, ("注册成功"), ("注册成功！"), ("确认"));
                query.clear();
                ui->le_name->clear();
                ui->le_password->clear();
                ui->le_pwagain->clear();
                ui->le_username->clear();
                this->hide();


            }
            emit isTimeToShow();
        }
    }

}

void signup::show_pressed()
{
    ui->btn_show->setIcon(QIcon("D:/nm/neimeng03/icons/show.png"));
    ui->btn_show->setStyleSheet("border-radius: 2px");
    ui->le_password->setEchoMode(QLineEdit::Normal);
    ui->le_pwagain->setEchoMode(QLineEdit::Normal);
}

void signup::show_released()
{
    ui->btn_show->setIcon(QIcon("D:/nm/neimeng03/icons/hide.png"));
    ui->btn_show->setStyleSheet("border-radius: 2px");
    ui->le_password->setEchoMode(QLineEdit::Password);
    ui->le_pwagain->setEchoMode(QLineEdit::Password);
}

void signup::OpenDB()
{
    if (!opened) {
        opened = true;
        db = QSqlDatabase::addDatabase("QMYSQL", "connection_l1");

        db.setHostName("localhost");
        db.setPort(3306);
        db.setUserName("root");
        db.setPassword("123456");
        db.setDatabaseName("robot");
        if (!db.open()) {
            qDebug() << db.lastError();
        }
    }
}
