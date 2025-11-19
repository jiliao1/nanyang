#include "MsgBox.h"


MsgBox::MsgBox(QWidget* parent)
{
}
MsgBox::~MsgBox()
{
}
void MsgBox::ShowErrorMessage(QString& str,int errorCode)
{
    QString ret;
    switch (errorCode)
    {
    case IRC_NET_ERROR_OK:
        ret = str + " Success!";
        QMessageBox::critical(this, tr("OK"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_FAILED:
        ret = str + " Failed!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_NOT_SUPPORTED:
        ret = str + " Not Supported!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_PARAM_WRONG:
        ret = str + " Param Wrong!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_TEMP_CALLBACK_WRONG:
        ret = "Temperature callback not enabled!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_BLACK_LIST:
        ret = str + " Black List!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_NONE_USER:
        ret = str + " None User!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_PWD_WRONG:
        ret = str + " Pwd Wrong!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_DEV_NOT_SUPPORTED:
        ret = str + " Device Not Supported !";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_TEMP_RULE_LIMIT:
        ret = str + " Rule Limit!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_ACCOUNT_LOCK:
        ret = str + " Account Lock!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_USER_LIMIT:
        ret = str + " User Limit!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;
    case IRC_NET_ERROR_SYSTEM_EXCEPTION:
        ret = str + " System Exception!";
        QMessageBox::critical(this, tr("Error"), tr(ret.toUtf8().constData()));
        break;

    default:
        break;
    }
}