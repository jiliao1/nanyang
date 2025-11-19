#include "robot.h"

#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QDebug>
#include <QSqlDatabase>
#include <QDir>
#include <QPluginLoader>
#include <QFile>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // MySQL驱动诊断已移动到robot.cpp中的testDatabaseConnection()函数
    
    // 设置现代化UI风格 - Fusion样式
    a.setStyle(QStyleFactory::create("Fusion"));
    
    // 设置现代深色主题样式表
    QString styleSheet = R"(
        /* 主窗口背景 */
        QMainWindow {
            background-color: #2b2b2b;
        }
        
        /* 通用Widget背景 */
        QWidget {
            background-color: #2b2b2b;
            color: #e0e0e0;
            font-family: "Microsoft YaHei UI", "Segoe UI", Arial;
            font-size: 9pt;
        }
        
        /* 按钮样式 */
        QPushButton {
            background-color: #3d3d3d;
            border: 1px solid #5a5a5a;
            border-radius: 4px;
            padding: 6px 12px;
            color: #e0e0e0;
            min-height: 24px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
            border: 1px solid #6a6a6a;
        }
        QPushButton:pressed {
            background-color: #2d2d2d;
        }
        QPushButton:disabled {
            background-color: #333333;
            color: #666666;
        }
        
        /* 主要操作按钮 */
        QPushButton#connectButton,
        QPushButton#btn_startVideo,
        QPushButton#startInfrared {
            background-color: #0d7377;
            border: 1px solid #14a085;
            color: white;
            font-weight: bold;
        }
        QPushButton#connectButton:hover,
        QPushButton#btn_startVideo:hover,
        QPushButton#startInfrared:hover {
            background-color: #14a085;
        }
        

        
        /* 滚动条样式 */
        QScrollBar:vertical {
            background-color: #2b2b2b;
            width: 12px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: #5a5a5a;
            border-radius: 6px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #6a6a6a;
        }
        QScrollBar:horizontal {
            background-color: #2b2b2b;
            height: 12px;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: #5a5a5a;
            border-radius: 6px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: #6a6a6a;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            border: none;
            background: none;
        }
        
        /* 分组框样式 */
        QGroupBox {
            border: 1px solid #5a5a5a;
            border-radius: 4px;
            margin-top: 12px;
            padding-top: 12px;
            color: #e0e0e0;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            background-color: #2b2b2b;
        }
        
        /* 标签样式 */
        QLabel {
            color: #e0e0e0;
            background-color: transparent;
        }
        
        /* 进度条样式 */
        QProgressBar {
            border: 1px solid #5a5a5a;
            border-radius: 4px;
            text-align: center;
            background-color: #3d3d3d;
            color: #e0e0e0;
        }
        QProgressBar::chunk {
            background-color: #0d7377;
            border-radius: 3px;
        }
        

    )";
    
    a.setStyleSheet(styleSheet);
    
    robot w;
    
    // 强制全屏显示
//    w.showFullScreen();
    w.show();
    return a.exec();
}
