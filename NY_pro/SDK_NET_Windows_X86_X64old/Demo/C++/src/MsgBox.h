#pragma once
#include <QDebug>
#include <QWidget>
#include <QMessageBox>

class MsgBox: public QWidget
{
    Q_OBJECT
public:
    enum ERRORCODE
    {
        IRC_NET_ERROR_OK = 0, ///< 成功
        IRC_NET_ERROR_FAILED = 1, ///< 失败
        IRC_NET_ERROR_NOT_SUPPORTED = 2, ///< 不支持
        IRC_NET_ERROR_PARAM_WRONG = 3, ///< 参数错误
        IRC_NET_ERROR_TEMP_CALLBACK_WRONG = 4, ///<温度回调未开启
        IRC_NET_ERROR_BLACK_LIST = 1001, ///< 用户不在白名单
        IRC_NET_ERROR_NONE_USER = 1002, ///< 用户名不存在
        IRC_NET_ERROR_PWD_WRONG = 1003, ///< 用户名或密码错误
        IRC_NET_ERROR_DEV_NOT_SUPPORTED = 1004, ///< 能力集无设备型号
        IRC_NET_ERROR_ACCOUNT_LOCK = 1005, ///< 账号锁定
        IRC_NET_ERROR_USER_LIMIT = 1006, ///< 用户数量超出限制
        IRC_NET_ERROR_SYSTEM_EXCEPTION = 1007, ///< 操作失败

        IRC_NET_ERROR_TEMP_RULE_LIMIT = 1101, ///< 测温规则上限

    };
    MsgBox(QWidget* parent = Q_NULLPTR);
    ~MsgBox();

    static MsgBox& GetInstance()
    {
        static MsgBox instance;
        return instance;
    }
    void ShowErrorMessage(QString& str, int errorCode);
};

#define MSG_BOX (MsgBox::GetInstance())