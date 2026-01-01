/**
 * @file LoginWindow.cpp
 * @brief 登录窗口类实现
 * @details 实现用户登录界面的所有交互逻辑,包括表单验证、网络请求、Token 存储等
 */

#include "LoginWindow.h"
#include "ui_LoginWindow.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>

/**
 * @brief 构造函数实现
 * @param _application 应用配置对象 (包含应用名称、Logo、API地址等)
 * @param parent 父窗口指针
 * @details
 * - 初始化 UI 界面 (setupUi)
 * - 设置窗口标题和 Logo
 * - 配置密码输入框为密文模式
 * - 设置"注册账号"和"忘记密码"超链接
 * - 连接登录按钮的信号槽
 * - 初始化网络请求管理器 QNetworkAccessManager
 */
LoginWindow::LoginWindow(const MediaEyeApplication & _application, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    app = _application;

    ui->setupUi(this);
    setWindowTitle(app.GetAppName());  // 设置窗口标题

    ui->error_label->setText("");  // 清空错误提示标签

    // 设置密码输入框为密文显示
    ui->password_line_edit->setEchoMode(QLineEdit::Password);
    // QFont font("宋体", 20, QFont::Thin);
    // ui->login_label->setFont(font);
    // ui->login_label->setText(" ");

    // 设置"注册账号"超链接 (点击后打开浏览器)
    ui->regiest_label->setText(("<a style='color:blue;' href='https://www.zzsin.com/register.html'>注册账号</a>"));
    // 设置"忘记密码"超链接
    ui->forget_pw_label->setText(("<a style='color:blue;' href='https://www.zzsin.com/find_pw.html'>忘记密码</a>"));
    connect(ui->regiest_label,SIGNAL(linkActivated(QString)),this,SLOT(openUrl(QString)));
    connect(ui->forget_pw_label,SIGNAL(linkActivated(QString)),this,SLOT(openUrl(QString)));
    // 连接登录按钮点击信号
    connect(ui->login_btn,       SIGNAL(clicked()),    this,   SLOT(LoginClickListener()));

    // 初始化网络请求管理器
    manage = new QNetworkAccessManager(this);       // 分配空间
    // 连接网络请求完成信号,用于接收登录响应
    connect(manage,SIGNAL(finished(QNetworkReply*)),this,SLOT(OnNetworkProcessFinished(QNetworkReply*)));

    // 加载并显示应用 Logo
    QImage img;
    img.load(app.GetLogoPath());
    img = img.scaledToWidth(100, Qt::SmoothTransformation);  // 缩放到宽度 100px
    ui->logo_label->setPixmap(QPixmap::fromImage(img));
}

/**
 * @brief 析构函数实现
 * @details 释放 UI 资源
 */
LoginWindow::~LoginWindow()
{
    delete ui;
}

/**
 * @brief 打开外部 URL 槽函数实现
 * @param url 要打开的网址
 * @details 使用系统默认浏览器打开指定 URL (如注册页面、找回密码页面)
 */
void LoginWindow::openUrl(QString url)
{
    QDesktopServices::openUrl(QUrl(url));
}

/**
 * @brief 登录按钮点击槽函数实现
 * @details
 * 登录流程:
 * 1. 验证邮箱和密码是否为空
 * 2. 将邮箱保存到本地缓存 (QSettings)
 * 3. 构造 HTTP POST 请求:
 *    - email: 用户邮箱 (URL 编码)
 *    - password: 用户密码 (URL 编码)
 *    - platform: 运行平台 (windows/macosx/linux)
 *    - app_name: 应用 ID
 *    - version: 应用版本号
 * 4. 发送网络请求到服务器验证用户名密码
 * 5. 等待 OnNetworkProcessFinished() 回调处理响应
 */
void LoginWindow::LoginClickListener()
{
    // 获取用户输入的邮箱
    QString email = ui->email_line_edit->text();
    if(email == ""){
        ui->error_label->setText("请输入邮箱");
        // QMessageBox::information(NULL, "请输入邮箱", "请输入邮箱");
        return;
    }
    // 获取用户输入的密码
    QString password = ui->password_line_edit->text();
    if(password == ""){
        ui->error_label->setText("请输入密码");
        // QMessageBox::information(NULL, "请输入密码", "请输入密码");
        return;
    }
    // 本地缓存设置保存 EMAIL，到了网络回调，如果登录失败就再去掉
    QSettings setting(app.GetCompanyName(), app.GetAppId());
    setting.setValue(app.GetKEY_EMAIL(), email);

    // 构造 HTTP POST 请求
    QNetworkRequest req;
    req.setUrl(QUrl(app.GetLoginURL()));  // 设置登录 API 地址

    // 构造 POST 请求体参数
    QByteArray post_data;

    // 添加邮箱参数 (URL 编码防止特殊字符导致问题)
    post_data.append("email=" + QUrl::toPercentEncoding(email) + "&");
    // 添加密码参数 (URL 编码)
    post_data.append("password=" +  QUrl::toPercentEncoding(password) + "&");
    // 添加平台参数 (根据编译目标平台自动选择)
#if defined(Q_OS_WIN)
    post_data.append("platform=windows&");
#elif defined(Q_OS_OSX)
    post_data.append("platform=macosx&");
#elif defined(Q_OS_LINUX)
    post_data.append("platform=linux&");
#else
    post_data.append("platform=other&");
#endif
    // 添加应用名称和版本号参数
    QString params = QString("app_name=") + app.GetAppId() + "&" + QString("version=") + app.GetVersion() + "";
    post_data.append(params.toUtf8());
    // post_data.append("app_name=" + application.GetAppId() + "&");
    // post_data.append("version=" + application.GetVersion() + "");

    // 设置 HTTP 请求头
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::ContentLengthHeader, post_data.length());

    // 发送 POST 请求到服务器
    manage->post(req, post_data);
}

/**
 * @brief 网络请求完成槽函数实现
 * @param reply 网络响应对象 (包含服务器返回的数据)
 * @details
 * 登录响应处理流程:
 * 1. 检查网络错误:
 *    - 如果网络连接失败,显示"网络错误"并清空邮箱缓存
 * 2. 解析 JSON 响应:
 *    - code == 200: 登录成功
 *      ✅ 从响应中提取 token
 *      ✅ 将 token 和更新时间保存到 QSettings
 *      ✅ 调用 GotoMain() 跳转主窗口
 *      ✅ 关闭登录窗口
 *    - code != 200: 登录失败
 *      ❌ 显示"用户名或密码错误"
 *      ❌ 清空邮箱缓存
 */
void LoginWindow::OnNetworkProcessFinished(QNetworkReply * reply)
{
    // 获取 HTTP 状态码 (如 200, 404, 500 等)
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid()){
        qDebug() << "status code=" << statusCode.toInt();
    }

    // 获取 HTTP 状态描述 (如 "OK", "Not Found" 等)
    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid()){
        qDebug() << "reason=" << reason.toString();
    }

    QSettings setting(app.GetCompanyName(), app.GetAppId());
    QNetworkReply::NetworkError err = reply->error();

    // 检查是否有网络错误 (如连接超时、DNS 解析失败等)
    if(err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
        ui->error_label->setText("网络错误");
        // QMessageBox::information(NULL, "网络错误", "网络错误");
        setting.setValue(app.GetKEY_EMAIL(), "");  // 清空缓存的邮箱
    } else {
        // 网络请求成功,读取响应数据
        QByteArray res = reply->readAll();

        qDebug() << QString(res) << Qt::endl;

        // 解析 JSON 响应
        QJsonObject jsonObj = QJsonDocument::fromJson(res).object();
        int code = jsonObj.find("code")->toInt();  // 获取业务状态码

        qDebug() << jsonObj << Qt::endl;

        qDebug() << "code:" << code << Qt::endl;
        if(code == 200){
            // 登录成功 (code == 200)
            // 从响应 JSON 中提取 token
            QJsonObject model = jsonObj.find("model")->toObject();
            QString token = model.find("token")->toString();

            // 将 token 和更新时间保存到本地缓存 (QSettings)
            QSettings setting(app.GetCompanyName(), app.GetAppId());
            setting.setValue(app.GetKEY_TOKEN(), token);  // 保存 token
            long long nowTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            setting.setValue(app.GetKEY_TOKEN_UPDATE_TIME(), nowTime);  // 保存 token 更新时间戳

            // 跳转到主窗口 (由子类实现具体逻辑)
            GotoMain();

            // 关闭登录窗口
            close();
        }
        else{
            // 登录失败 (用户名或密码错误)
            setting.setValue(app.GetKEY_EMAIL(), "");  // 清空缓存的邮箱
            ui->error_label->setText("用户名或密码错误");
            // QMessageBox::information(NULL, "用户名或密码错误", "用户名或密码错误");
        }
    }
}
