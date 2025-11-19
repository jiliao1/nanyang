#include "ipconnect.h"
#include "ui_ipconnect.h"
#include <QIntValidator>

#pragma execution_character_set("utf-8")

IpConnect::IpConnect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IpConnect)
{
    ui->setupUi(this);
    QIntValidator* portValidator = new QIntValidator(1, 65535, this);
        ui->PortEdit->setValidator(portValidator);

        // 2. IP 地址掩码：提供一个格式，帮助用户输入
        //    (这不是一个严格的验证器，但能提高用户体验)
        ui->ipAddressEdit->setInputMask("000.000.000.000;");

        // 设置窗口标题
        setWindowTitle("连接到巡检车");
}

IpConnect::~IpConnect()
{
    delete ui;
}

void IpConnect::setIpAddress(const QString &ip)
{
    ui->ipAddressEdit->setText(ip);
}

void IpConnect::setPort(const QString &port)
{
    ui->PortEdit->setText(port);
}

QString IpConnect::getIpAddress() const
{
    // .trimmed() 用于移除用户可能意外输入的
    // 前后空格
    return ui->ipAddressEdit->text().trimmed();
}

/**
 * @brief 获取 PortEdit 控件中用户输入的端口号
 */
QString IpConnect::getPort() const
{
    return ui->PortEdit->text().trimmed();
}
