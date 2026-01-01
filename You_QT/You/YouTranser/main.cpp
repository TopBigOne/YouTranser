/**
 * @file main.cpp
 * @brief YouTranser 视频转码工具主程序入口
 * @details 这是一个跨平台的视频转码应用程序，支持 H.264/H.265/ProRes 等多种编码格式
 * @author zzsin.com
 */

#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QStyleFactory>      // Qt 样式工厂,用于设置应用外观
#include <QFontDatabase>      // 字体数据库,用于加载自定义字体
#include <QApplication>

#ifdef WIN32
// #include <QDesktopWidget>  // Windows 平台下的桌面小部件(已废弃)
#endif

#include <MainWindow.h>

#include "YouTransLoading.hpp"       // 加载界面
#include "YouTransMainWindow.hpp"    // 主窗口
#include "YouTransAppConfig.hpp"     // 应用配置
#include "YouTransAboutWindow.hpp"   // 关于窗口

/**
 * @brief 程序主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出码，0表示正常退出
 */
int main(int argc, char *argv[])
{
    // 创建 Qt 应用程序实例
    QApplication a(argc, argv);

    // 调试信息：打印系统支持的所有 Qt 样式
    QStringList list = QStyleFactory::keys();
    foreach(const QString &str,list) {
        qDebug() << str << Qt::endl;
    }

    // 可选：设置应用程序样式为 Fusion (跨平台现代风格)
    // QStyle* style = QStyleFactory::create("Fusion");
    // a.setStyle(style);

    /*
     * 可选：加载自定义字体
     * 从资源文件中加载 SmileySans-Oblique.otf 字体
     */
    /*
    int font_Id = QFontDatabase::addApplicationFont(":/font/SmileySans-Oblique.otf");
    QStringList font_list = QFontDatabase::applicationFontFamilies(font_Id);
    qDebug()<<font_Id;
    qDebug()<<font_list;
    if(!font_list.isEmpty())
    {
        QFont f;
        f.setFamily(font_list[0]);
        a.setFont(f);
    }
    */

    // 可选：设置中文字体
    // QFont f("微软雅黑");
    // a.setFont(f);

    /*
     * 可选：从 QSS 文件加载样式表
     * QSS (Qt Style Sheet) 类似于 CSS，用于定义界面样式
     */
    /*
// #ifndef WIN32
    QFile qss(":/you-trans.qss");
    if( qss.open(QFile::ReadOnly)) {
        qDebug() << "open success" << Qt::endl;
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else {
        qDebug() << "open failed" << Qt::endl;
    }
// #endif
    */

    // 创建应用配置对象 (包含应用名称、版本、Logo等信息)
    Eyer::YouTransAppConfig app;

    // 创建并显示加载窗口 (启动画面)
    Eyer::YouTransLoading loading(app);
#ifdef WIN32
    // Windows 平台下将窗口居中显示 (QDesktopWidget 已废弃,建议使用 QScreen)
    // QDesktopWidget *desktop = QApplication::desktop();
    // loading.move((desktop->width() - loading.width())/ 2, (desktop->height() - loading.height()) /2);
#endif
    loading.show();

    // 调试代码：直接显示主窗口或关于窗口
    // YouTransMainWindow mainWin;
    // mainWin.show();

    // YouTransAboutWindow about;
    // about.show();

    // 进入 Qt 事件循环，等待用户交互
    // 程序将在此处保持运行，直到所有窗口关闭
    return a.exec();
}
