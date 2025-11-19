#include "MainWindow.h"
#include <QApplication>
#include "IRCNetSDK.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IRC_NET_Init();
    int ret = -1;
    {
        MainWindow w;
        w.show();
        ret = a.exec();
    }
    IRC_NET_Deinit();
    return ret;
}
