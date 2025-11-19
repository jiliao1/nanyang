#ifndef SIGNUP_H
#define SIGNUP_H

#include <QDialog>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QtSql>
#include <QSqlDatabase>
#include <memory>

namespace Ui {
class signup;
}

class signup : public QDialog
{
    Q_OBJECT

public:
    explicit signup(QWidget *parent = nullptr);
    ~signup();

private:
    Ui::signup *ui;

public slots:
    void back();
    void useregister();
    void show_pressed();
    void show_released();
private:
    void OpenDB();
    QSqlDatabase db;
signals:
    void isTimeToShow();
};

#endif // SIGNUP_H
