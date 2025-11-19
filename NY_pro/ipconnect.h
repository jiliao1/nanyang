#ifndef IPCONNECT_H
#define IPCONNECT_H

#include <QDialog>

namespace Ui {
class IpConnect;
}

class IpConnect : public QDialog
{
    Q_OBJECT

public:
    explicit IpConnect(QWidget *parent = nullptr);
    ~IpConnect();
    void setIpAddress(const QString &ip);
    void setPort(const QString &port);
    QString getIpAddress() const;
    QString getPort() const;

private:
    Ui::IpConnect *ui;
};

#endif // IPCONNECT_H
