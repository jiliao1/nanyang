#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "IRCNetSDK.h"
#include "VideoLabel.h"
#include <QDebug>
#include <QMessageBox>
#include <QCalendarWidget>
#include <QButtonGroup>
#include <set>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_downloadPage = new DownloadPage(this);
    m_group = new QButtonGroup(this);
    InitForm();
    ConnectSignalSlot();
}

MainWindow::~MainWindow()
{
    if (tr("Logout") == ui->loginBtn->text())
    {
        OnLoginBtnClicked();
    }
    delete[]m_downloadPage;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_tempAlarmPage->CloseAlarmPage();
    m_downloadPage->close();
    event->accept();
}

void MainWindow::ResetVideoLabel()
{
    ui->videoLabel->ResetParam();
}
void MainWindow::SetOncePointTempParamCheck()
{
    ui->videoLabel->SetDrawMode(VideoLabel::DRAW_ONCE);
    ui->drawPointBtn->setEnabled(false);
    ui->drawLineBtn->setEnabled(false);
    ui->drawAreaBtn->setEnabled(false);
    ui->drawCircleBtn->setEnabled(false);
    ui->drawPolygonBtn->setEnabled(false);
    ui->shieldedAreaBtn->setEnabled(false);
}

void MainWindow::SetOncePointTempParamUnCheck()
{
    ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    ui->drawPointBtn->setEnabled(true);
    ui->drawLineBtn->setEnabled(true);
    ui->drawAreaBtn->setEnabled(true);
    ui->drawCircleBtn->setEnabled(true);
    ui->drawPolygonBtn->setEnabled(true);
    ui->shieldedAreaBtn->setEnabled(true);
}


void MainWindow::InitForm()
{
    // 禁用最大化按钮
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(width(), height());
    ui->tabWidget_2->setEnabled(false);
    //ui->ctrlWidget->setEnabled(false);
    ui->drawWidget->hide();
    ui->drawWidget->setEnabled(false);
    ui->IPcomboBox->setEditable(true);
    ui->selectAllBtn->setEnabled(false);
    ui->downloadBtn->setEnabled(false);
    ui->backBtn->setEnabled(false);
    ui->nextBtn->setEnabled(false);
    ui->downProgressBtn->setEnabled(false);
    ui->onlineChannel0RadioBtn->setChecked(true);
    ui->videoRadioBtn->setChecked(true);
    QButtonGroup* pBtnG = new QButtonGroup();
    pBtnG->addButton(ui->playbackChannel0RadioBtn);
    pBtnG->addButton(ui->playbackChannel1RadioBtn);
    ui->playbackChannel0RadioBtn->setChecked(true); //为radio组设置初选项

    ui->endDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    QDateTime oneDayBefore = QDateTime::currentDateTime().addDays(-1);
    ui->startDateTimeEdit->setDateTime(oneDayBefore);

    m_group->addButton(ui->drawPointBtn);
    m_group->addButton(ui->drawLineBtn);
    m_group->addButton(ui->drawAreaBtn);
    m_group->addButton(ui->drawCircleBtn);
    m_group->addButton(ui->drawPolygonBtn);
    m_group->addButton(ui->shieldedAreaBtn);
    m_group->setExclusive(false);

    InitPages();
}

void MainWindow::InitPages()
{
    m_previewPage = new PreviewPage(this);
    m_tempAlarmPage = new TempAlarmPage(this);
    m_configPage = new ConfigPage(this);
    m_ptzPage = new PTZPage(this);
    m_controlPage = new ControlPage(this);
    m_peripheralPage = new PeripheralPage(this);
    ui->tabWidget->addTab(m_previewPage, tr("Preview"));
    ui->tabWidget->addTab(m_tempAlarmPage, tr("Temp Alarm"));
    ui->tabWidget->addTab(m_configPage, tr("Config"));
    ui->tabWidget->addTab(m_ptzPage, tr("PTZ"));
    ui->tabWidget->addTab(m_controlPage, tr("Swivel"));
    ui->tabWidget->addTab(m_peripheralPage, tr("Other"));

    ui->tabWidget_2->setCurrentIndex(0);
}

void MainWindow::RefreshPagesUi()
{
    ui->tableWidget->clear();
    m_fileInfoMap.clear();
    m_fileDownloadMap.clear();
    m_previewPage->ClearUi();
    m_tempAlarmPage->ClearUi();
    m_configPage->ClearUi();
    m_ptzPage->ClearUi();
}

void MainWindow::ConnectSignalSlot()
{
    connect(this, SIGNAL(DevSearched(const QString&)), this, SLOT(OnDevSearched(const QString&)));
    connect(ui->loginBtn, SIGNAL(clicked()), this, SLOT(OnLoginBtnClicked()));
    connect(ui->drawPointBtn, SIGNAL(clicked()), this, SLOT(OnDrawPointBtnClicked()));
    connect(ui->drawLineBtn, SIGNAL(clicked()), this, SLOT(OnDrawLineBtnClicked()));
    connect(ui->drawAreaBtn, SIGNAL(clicked()), this, SLOT(OnDrawAreaBtnClicked()));
    connect(ui->drawCircleBtn, SIGNAL(clicked()), this, SLOT(OnDrawCircleBtnClicked()));
    connect(ui->drawPolygonBtn, SIGNAL(clicked()), this, SLOT(OnDrawPolygonBtnClicked()));
    connect(ui->shieldedAreaBtn, SIGNAL(clicked()), this, SLOT(OnShieldedAreaBtnClicked()));
    connect(m_previewPage, SIGNAL(UpdateFrame()), this, SLOT(OnUpdateFrame()));
    connect(m_previewPage, SIGNAL(PreviewOpen(bool)), this, SLOT(OnPreviewOpen(bool)));
    connect(ui->videoLabel, SIGNAL(GetOncePointTemp(QPoint)), this, SLOT(OnGetOncePointTemp(QPoint)));
    connect(ui->videoLabel, SIGNAL(AddRule(int, QList<QPoint>)), this, SLOT(OnAddRule(int, QList<QPoint>)));
    connect(ui->videoLabel, SIGNAL(AddShielded(int, QList<QPoint>)), this, SLOT(OnAddShielded(int, QList<QPoint>)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));
    connect(this, SIGNAL(Exception(int)), this, SLOT(OnException(int)));
	connect(ui->onlineChannel0RadioBtn, SIGNAL(clicked()), this, SLOT(OnChannelButtonClicked()));
	connect(ui->onlineChannel1RadioBtn, SIGNAL(clicked()), this, SLOT(OnChannelButtonClicked()));
	connect(ui->playbackChannel0RadioBtn, SIGNAL(clicked()), this, SLOT(OnChannelButtonClicked()));
	connect(ui->playbackChannel1RadioBtn, SIGNAL(clicked()), this, SLOT(OnChannelButtonClicked()));
	connect(ui->videoRadioBtn, SIGNAL(clicked()), this, SLOT(OnFileTypeButtonClicked()));
	connect(ui->imageRadioBtn, SIGNAL(clicked()), this, SLOT(OnFileTypeButtonClicked()));
	connect(ui->ipSearchBtn, SIGNAL(clicked()), this, SLOT(OnIPSearchButtonClicked()));
	connect(ui->sdCardSearchBtn, SIGNAL(clicked()), this, SLOT(OnSDcardSearchButtonClicked()));
	connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(OnBackButtonClicked()));
	connect(ui->nextBtn, SIGNAL(clicked()), this, SLOT(OnNextButtonClicked()));
	connect(ui->selectAllBtn, SIGNAL(clicked()), this, SLOT(OnSellectAllButtonClicked()));
	connect(ui->downloadBtn, SIGNAL(clicked()), this, SLOT(OnDownloadButtonClicked()));
    connect(m_ptzPage, SIGNAL(Position3D(bool)), this, SLOT(OnPosition3D(bool)));
    connect(m_ptzPage, SIGNAL(PositionRegionFocus(bool)), this, SLOT(OnPositionRegionFocus(bool)));
    connect(m_ptzPage, SIGNAL(PositionManualTrack(bool)), this, SLOT(OnPositionManualTrack(bool)));
    connect(ui->videoLabel, SIGNAL(Postion(int, QList<QPoint>)), this, SLOT(OnPostion(int, QList<QPoint>)));
    connect(ui->videoLabel, SIGNAL(RegionFocus(QList<QPoint>)), this, SLOT(OnRegionFocus(QList<QPoint>)));
    connect(ui->videoLabel, SIGNAL(ManualTrack(QList<QPoint>)), this, SLOT(OnManualTrack(QList<QPoint>)));
    connect(ui->downProgressBtn, SIGNAL(clicked()), this, SLOT(OnDownloadProgressButtonClicked()));
}

void MainWindow::SearchFile()
{
    ui->tableWidget->clear();
    m_fileInfoMap.clear();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList headers;
    headers << "Index" << "Name";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    IRC_NET_FILE_QUERY_PARAM param;
    int fileSize = 0;
    param.channel = m_playbackChannel;
    param.type = m_filetype;
    param.queryType = IRC_NET_FILE_QUERY_ALL;
    param.count = 50;
    param.offset = m_page;
    if (ui->startDateTimeEdit->dateTime() > ui->endDateTimeEdit->dateTime())
    {
        ui->backBtn->setEnabled(false);
        ui->nextBtn->setEnabled(false);
        ui->selectAllBtn->setEnabled(false);
        ui->downloadBtn->setEnabled(false);
        ui->downProgressBtn->setEnabled(false);
        QMessageBox::critical(this, tr("Error"), tr("StartTime is grater endTime!"));
        return;
    }
    strcpy(param.startTime, ui->startDateTimeEdit->dateTime().toString("yyyy-MM-dd+HH:mm:ss").toLocal8Bit().data());
    strcpy(param.endTime, ui->endDateTimeEdit->dateTime().toString("yyyy-MM-dd+HH:mm:ss").toLocal8Bit().data());
    int err = IRC_NET_QueryFileSize(m_handle, &param, &fileSize);
    QString temp = tr("QueryFileSize");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    if (0 == fileSize)
    {
        QMessageBox::critical(this, tr("Error"), tr("No SD file!"));
        ui->selectAllBtn->setEnabled(false);
        ui->downloadBtn->setEnabled(false);
        ui->backBtn->setEnabled(false);
        ui->nextBtn->setEnabled(false);
        ui->downProgressBtn->setEnabled(false);
        return;
    }
    else
    {
        ui->selectAllBtn->setEnabled(true);
        ui->downloadBtn->setEnabled(true);
        ui->downProgressBtn->setEnabled(true);
        IRC_NET_FILE_INFO* fileinfo = new IRC_NET_FILE_INFO[fileSize];
        memset(fileinfo, 0, fileSize * sizeof(IRC_NET_FILE_INFO));
        int outSize = 0;
        if (IRC_NET_ERROR_OK != IRC_NET_QueryFile(m_handle, &param, fileinfo, fileSize, &outSize))
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            delete[]fileinfo;
            fileinfo = nullptr;
            return;
        }
        ui->tableWidget->setRowCount(outSize);
        ui->tableWidget->setColumnCount(2);
        for (int i = 0; i < outSize; i++)
        {
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
            QString str = QString::fromLocal8Bit(fileinfo[i].path);
            QStringList list = str.split("/");
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(list.last()));
            m_fileInfoMap[i] = fileinfo[i];
        }
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //QStringList headers;
        headers << "Index" << "Name";
        ui->tableWidget->setHorizontalHeaderLabels(headers);

        outSize < 50 ? ui->nextBtn->setEnabled(false) : ui->nextBtn->setEnabled(true);
        m_page == 0 ? ui->backBtn->setEnabled(false) : ui->backBtn->setEnabled(true);
        delete[]fileinfo;
        fileinfo = nullptr;
    }

}

void MainWindow::GetAbility()
{
    int err = IRC_NET_GetDevAbility(m_handle, m_onlineChannel, IRC_NET_DEV_ABILITY_PTZ, &m_devAbility);
    QString temp = tr("GetDevAbility");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void MainWindow::GetAuxState()
{
    memset(&m_ptzAuxState, 0, sizeof(IRC_NET_PTZ_AUX_STATE));
    if (IRC_NET_ERROR_OK != IRC_NET_GetPtzAuxState(m_handle, &m_ptzAuxState))
    {
        return;
    }
}

void MainWindow::RefreshPages()
{
    if (1 == m_devInfo.channelNum)
    {
        ui->onlineChannel1RadioBtn->setEnabled(false);
        ui->playbackChannel1RadioBtn->setEnabled(false);
    }
    else
    {
        ui->onlineChannel1RadioBtn->setEnabled(true);
        ui->playbackChannel1RadioBtn->setEnabled(true);
    }
    if (m_onlineChannel == m_devInfo.irChannel)
    {
        //m_tempAlarmPage->SetWidgetEnable(true);
        m_tempAlarmPage->SetAlarmCallParamShow();
        m_peripheralPage->SetDayOrNightShow(false);
        if (1 == ui->tabWidget->currentIndex())
        {
            ui->drawWidget->show();
        }
        else
        {
            ui->drawWidget->hide();
        }
    }
    else
    {
        //m_tempAlarmPage->SetWidgetEnable(false);
        ui->drawWidget->hide();
        m_tempAlarmPage->SetAlarmCallParamHide(); 
        m_peripheralPage->SetDayOrNightShow(true);
    }
    GetAbility();
    GetAuxState();
    m_tempAlarmPage->InitButton();
    m_ptzPage->InitButton();
    m_ptzPage->GetAuxiState();
}

void MainWindow::SetLoginInfoEnable(bool enable)
{
    ui->ipSearchBtn->setEnabled(enable);
    ui->IPcomboBox->setEnabled(enable);
    ui->portEdit->setEnabled(enable);
    ui->userEdit->setEnabled(enable);
    ui->passwordEdit->setEnabled(enable);
}

void MainWindow::HandleDevSearchInfo(IRC_NET_DEV_SEARCH_INFO* searchInfo)
{
    if (searchInfo != nullptr)
    {
        QString ip = searchInfo->ip;
        emit DevSearched(ip);
    }
}

void MainWindow::uncheckOthers(QAbstractButton* button, QButtonGroup* group)
{
    for (int i = 0; i < group->buttons().size(); ++i) {
        QPushButton* otherButton = qobject_cast<QPushButton*>(group->buttons()[i]);
        if (otherButton && otherButton != button) {
            otherButton->setChecked(false);
        }
    }
}

void MainWindow::uncheckAll(QButtonGroup* group)
{
    for (int i = 0; i < group->buttons().size(); ++i) {
        QPushButton* otherButton = qobject_cast<QPushButton*>(group->buttons()[i]);
        if (otherButton) {
            otherButton->setChecked(false);
        }
    }
}

void MainWindow::OnDevSearched(const QString& ip)
{
    ui->IPcomboBox->addItem(ip);
}

void MainWindow::OnLoginBtnClicked()
{
    QString loginState = ui->loginBtn->text();
    if (loginState == tr("Login"))
    {
        QString ip = ui->IPcomboBox->currentText();
        QString port = ui->portEdit->text();
        QString user = ui->userEdit->text();
        QString password = ui->passwordEdit->text();
        IRC_NET_LOGIN_INFO loginInfo;
        memcpy(&loginInfo.ip, ip.toLocal8Bit().data(), ip.length() + 1);
        loginInfo.port = port.toInt();
        memcpy(&loginInfo.username, user.toLocal8Bit().data(), user.length() + 1);
        memcpy(&loginInfo.password, password.toLocal8Bit().data(), password.length() + 1);
        int err = IRC_NET_Login(&loginInfo, &m_handle);
        QString temp = tr("Login");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        err = IRC_NET_SetExceptionCallback(m_handle, ExceptionCallback, this);
        temp = tr("SetExceptionCallback");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        err = IRC_NET_GetDevInfo(m_handle, &m_devInfo);
        temp = tr("GetDevInfo");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui->onlineChannel0RadioBtn->setChecked(true);
        ui->playbackChannel0RadioBtn->setChecked(true);
        OnChannelButtonClicked();
        ui->loginBtn->setText(tr("Logout"));
        SetLoginInfoEnable(false);
        //ui->loginInfoWidget->setEnabled(false);
        ui->tabWidget_2->setEnabled(true);
        GetAbility();
        GetAuxState();
        m_tempAlarmPage->InitButton();
        m_ptzPage->InitButton();
        m_ptzPage->GetAuxiState();
    }
    else
    {
        m_previewPage->StopPreview();
        //如果有正在下载的任务 停止下载
        QString temp = tr("StopDownloadFile");
        for (auto iter = m_fileDownloadMap.constBegin(); iter != m_fileDownloadMap.constEnd(); iter++)
        {
            int err = IRC_NET_StopDownloadFile(m_handle, iter.key());
            if (IRC_NET_ERROR_OK != err)
            {
                MSG_BOX.ShowErrorMessage(temp, err);
                return;
            }
        }
        m_tempAlarmPage->CloseAlarmPage();
        m_downloadPage->close();
        RefreshPagesUi();
        IRC_NET_Logout(m_handle);
        m_handle = 0;
        ui->loginBtn->setText(tr("Login"));
        SetLoginInfoEnable(true);
        //ui->loginInfoWidget->setEnabled(true);
        ui->tabWidget_2->setEnabled(false);
    }

    emit ChangeHandle(m_handle);
}

void MainWindow::OnDrawPointBtnClicked()
{
    if (ui->drawPointBtn->isChecked())
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_POINT);
        uncheckOthers(ui->drawPointBtn, m_group);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnDrawLineBtnClicked()
{
    if (ui->drawLineBtn->isChecked())
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_LINE);
        uncheckOthers(ui->drawLineBtn, m_group);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnDrawAreaBtnClicked()
{
    if (ui->drawAreaBtn->isChecked())
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_AREA);
        uncheckOthers(ui->drawAreaBtn, m_group);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnDrawCircleBtnClicked()
{
    if (ui->drawCircleBtn->isChecked())
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_CIRCLE);
        uncheckOthers(ui->drawCircleBtn, m_group);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnDrawPolygonBtnClicked()
{
    if (ui->drawPolygonBtn->isChecked())
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_POLYGON);
        uncheckOthers(ui->drawPolygonBtn, m_group);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnShieldedAreaBtnClicked()
{
    if (ui->shieldedAreaBtn->isChecked())
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_SHIELDED);
        uncheckOthers(ui->shieldedAreaBtn, m_group);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnUpdateFrame()
{
    ui->videoLabel->SetImg(m_previewPage->GetImg());
}

void MainWindow::OnGetOncePointTemp(QPoint point)
{
    float pointTemp = 0;
    int x = point.x();
    x = x < 0 ? 0 : x > ui->videoLabel->width() ? ui->videoLabel->width() : x;
    int y = point.y();
    y = y < 0 ? 0 : y > ui->videoLabel->height() ? ui->videoLabel->height() : y;
    IRC_NET_POINT* pointTmp = new IRC_NET_POINT();
    pointTmp->x = 8191 * x / ui->videoLabel->width();
    pointTmp->y = 8191 * y / ui->videoLabel->height();

    int err = IRC_NET_QueryRandomTemp(m_handle, pointTmp,&pointTemp);
    QString temp = tr("AddTempRule");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    m_tempAlarmPage->SetOnceTempEdit(pointTemp);
    SetOncePointTempParamUnCheck();
   // ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);

}

void MainWindow::OnAddRule(int ruleType, QList<QPoint> points)
{
    if (IRC_NET_TEMP_RULE_RECT == ruleType)
    {
        QRect rect(points.at(0), points.at(1));
        points[0] = rect.topLeft();
        points[1] = rect.topRight();
        points.append(rect.bottomRight());
        points.append(rect.bottomLeft());
    }

    IRC_NET_TEMP_RULE_INFO ruleInfo;
    IRC_NET_TEMP_RULE_INFO_G1 ruleInfoG1;
    memset(&ruleInfo, 0, sizeof(IRC_NET_TEMP_RULE_INFO));
    memset(&ruleInfoG1, 0, sizeof(IRC_NET_TEMP_RULE_INFO_G1));
    strcpy(ruleInfo.name, "");
    ruleInfo.presetId = m_tempAlarmPage->GetCurPresetIndex();
    ruleInfo.enable = true;
    ruleInfo.type = ruleType;
    ruleInfo.pointNum = points.size();
    for (int i = 0; i < ruleInfo.pointNum; i++)
    {
        int x = points.at(i).x();
        x = x < 0 ? 0 : x > ui->videoLabel->width() ? ui->videoLabel->width() : x;
        int y = points.at(i).y();
        y = y < 0 ? 0 : y > ui->videoLabel->height() ? ui->videoLabel->height() : y;
        IRC_NET_POINT point{ 8191 * x / ui->videoLabel->width(), 8191 * y / ui->videoLabel->height() };
        ruleInfo.points[i] = point;
    }
    ruleInfoG1.type = ruleType;
    if (IRC_NET_TEMP_RULE_POINT == ruleType)
    {
        ruleInfoG1.startPoint.x = ruleInfo.points[0].x;
        ruleInfoG1.startPoint.y = ruleInfo.points[0].y;
        ruleInfoG1.endPoint.x = ruleInfo.points[0].x;
        ruleInfoG1.endPoint.y = ruleInfo.points[0].y;
    }
    else if (IRC_NET_TEMP_RULE_LINE == ruleType)
    {
        ruleInfoG1.startPoint.x = ruleInfo.points[0].x;
        ruleInfoG1.startPoint.y = ruleInfo.points[0].y;
        ruleInfoG1.endPoint.x = ruleInfo.points[1].x;
        ruleInfoG1.endPoint.y = ruleInfo.points[1].y;
    }
    else if (IRC_NET_TEMP_RULE_RECT == ruleType)
    {
        ruleInfoG1.startPoint.x = ruleInfo.points[0].x;
        ruleInfoG1.startPoint.y = ruleInfo.points[0].y;
        ruleInfoG1.endPoint.x = ruleInfo.points[2].x;
        ruleInfoG1.endPoint.y = ruleInfo.points[2].y;
    }
    int err = IRC_NET_AddTempRule(m_handle, &ruleInfo);
    QString temp = tr("AddTempRule");
    if (IRC_NET_ERROR_OK != err)
    {
        if (IRC_NET_ERROR_NOT_SUPPORTED == err)
        {
            err = IRC_NET_AddTempRule_G1(m_handle, &ruleInfoG1);
            temp = tr("AddTempRule");
            if (IRC_NET_ERROR_OK != err)
            {
                MSG_BOX.ShowErrorMessage(temp, err);
                return;
            }
        }
        else
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
    }
}

void MainWindow::OnAddShielded(int ruleType, QList<QPoint> points)
{
    QList<QPoint> pointList;
    for (const auto& point : points)
    {
        int x = 8191 * point.x() / ui->videoLabel->width();
        int y = 8191 * point.y() / ui->videoLabel->height();
        pointList.push_back(QPoint(x, y));
    }
    emit Shielded(ruleType, pointList);
}

void MainWindow::OnTabChanged(int index)
{
    ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    uncheckAll(m_group);
    RefreshPages();
    SetOncePointTempParamUnCheck();
}

void MainWindow::OnPreviewOpen(bool open)
{
    ui->videoLabel->setEnabled(open);
    ui->drawWidget->setEnabled(open);
}

void MainWindow::OnException(int type)
{
    switch (type)
    {
    case IRC_NET_EXCEPTION_DEV_OFFLINE:
        QMessageBox::critical(this, tr("Exception"), QString::number(type) + tr(":dev offline."));
        break;
    case IRC_NET_EXCEPTION_PREVIEW_OFFLINE:
        QMessageBox::critical(this, tr("Exception"), QString::number(type) + tr(":Preview offline."));
        break;
    case IRC_NET_EXCEPTION_ALARM_OFFLINE:
        QMessageBox::critical(this, tr("Exception"), QString::number(type) + tr(":Alarm offline."));
        break;
    case IRC_NET_EXCEPTION_TEMP_OFFLINE:
        QMessageBox::critical(this, tr("Exception"), QString::number(type) + tr(":temp offline."));
        break;
    default:
        break;
    }
}

void MainWindow::ExceptionCallback(IRC_NET_HANDLE handle, int exceptionType, void* userData)
{
    qDebug() << "exception" << exceptionType;
    MainWindow* thisPtr = static_cast<MainWindow*>(userData);
    thisPtr->Exception(exceptionType);
}

void MainWindow::DevSearchCallback(IRC_NET_DEV_SEARCH_INFO* searchInfo, void* userData)
{
    MainWindow* thisPtr = static_cast<MainWindow*>(userData);
    thisPtr->HandleDevSearchInfo(searchInfo);
}

void MainWindow::OnChannelButtonClicked()
{
    m_previewPage->StopPreview();
	if (ui->onlineChannel0RadioBtn->isChecked())
	{
		m_onlineChannel = 0;
	}

	if (ui->onlineChannel1RadioBtn->isChecked())
	{
		m_onlineChannel = 1;
	}

	if (ui->playbackChannel0RadioBtn->isChecked())
	{
		m_playbackChannel = 0;
	}

	if (ui->playbackChannel1RadioBtn->isChecked())
	{
		m_playbackChannel = 1;
	}
    RefreshPages();
}

void MainWindow::OnFileTypeButtonClicked()
{
	if (ui->videoRadioBtn->isChecked())
	{
		m_filetype = IRC_NET_FILE_VIDEO;
	}

	if (ui->imageRadioBtn->isChecked())
	{
		m_filetype = IRC_NET_FILE_PICTURE;
	}
}

void MainWindow::OnIPSearchButtonClicked()
{
    ui->IPcomboBox->clear();

    int timeout = 8000;
    int err = IRC_NET_SearchDev(DevSearchCallback, this, timeout);
    QString temp = tr("SearchDev");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void MainWindow::OnSDcardSearchButtonClicked()
{
    m_page = 0;
    SearchFile();
    //只要点击搜索 selectAllBtn按钮文字变为Select All
    ui->selectAllBtn->setText("Select All");
}

void MainWindow::OnBackButtonClicked()
{
    m_page--;
    SearchFile();
}

void MainWindow::OnNextButtonClicked()
{
    m_page++;
    SearchFile();
}

void MainWindow::OnSellectAllButtonClicked()
{
    int top = 0, left = 0, bottom = ui->tableWidget->rowCount() - 1, right = ui->tableWidget->columnCount()-1;
    QTableWidgetSelectionRange range = QTableWidgetSelectionRange(top, left, bottom, right);

    if (ui->selectAllBtn->text() == "Select All")
    {
        ui->tableWidget->setRangeSelected(range, true);
        ui->selectAllBtn->setText("Deselect All");
    }
    else
    {
        ui->tableWidget->setRangeSelected(range, false);
        ui->selectAllBtn->setText("Select All");
    }
}

void MainWindow::OnDownloadButtonClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Choose directory", "./");
    if (folderPath.isEmpty())
    {
        return;
    }
    IRC_NET_FILE_DOWNLOAD_INFO downloadInfo;
    std::set<int> rowSet;
    QList<QTableWidgetItem*> selections = ui->tableWidget->selectedItems();
    for (int i = 0; i < selections.size(); i++)
    {
        rowSet.insert(selections.at(i)->row());
    }

    for (auto row : rowSet)
    {
        memcpy(&downloadInfo.fileInfo, &m_fileInfoMap[row], sizeof(IRC_NET_FILE_INFO));
        QString path = QString::fromLocal8Bit(m_fileInfoMap[row].path);
        QStringList list = path.split("/");
        QString downloadPath = folderPath + "/" + list.last();
        QByteArray pathArray = downloadPath.toLocal8Bit();
        strcpy(downloadInfo.downloadPath, pathArray.data());
        int err = IRC_NET_StartDownloadFile(m_handle, &downloadInfo);
        QString temp = tr("StartDownloadFile");
        if (err < 0)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            continue;
        }
        m_fileDownloadMap[err] = downloadInfo;

    }
    emit SendDownload(m_fileDownloadMap);
}

void MainWindow::OnPosition3D(bool state)
{
    if (state)
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_POSITION);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnPositionRegionFocus(bool state)
{
    if (state)
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_FOCUS);
    }
    else
    {
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnPositionManualTrack(bool state)
{
    if (state)
    {
        ui->videoLabel->SetDrawMode(VideoLabel::DRAW_MANUAL);
    }
    else
    {
        IRC_NET_PTZ_MANUAL_TRACK_PARAM positionManualParam;
        positionManualParam.channel = m_onlineChannel;
        positionManualParam.startPoint.x = 0;
        positionManualParam.startPoint.y = 0;
        positionManualParam.endPoint.x = 0;
        positionManualParam.endPoint.y = 0; 
        positionManualParam.stop = 1;
        int err = IRC_NET_PtzManualTrack(m_handle, &positionManualParam);
        QString temp = tr("PtzManualTrack");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
    }
}

void MainWindow::OnPostion(int ruleType, QList<QPoint> points)
{
    const int coordinate = 8192;
    QList<QPoint> pointList;
    for (const auto& point : points)
    {
        int x = coordinate * point.x() / ui->videoLabel->width();
        int y = coordinate * point.y() / ui->videoLabel->height();
        pointList.push_back(QPoint(x, y));
    }

    IRC_NET_PTZ_3D_POSITION_PARAM positionConfig;
    positionConfig.channel = m_onlineChannel;
    if (points.size() == 2)
    {
        positionConfig.startPoint.x = coordinate * points[0].x() / ui->videoLabel->width();
        positionConfig.startPoint.y = coordinate * points[0].y() / ui->videoLabel->height();
        positionConfig.endPoint.x = coordinate * points[1].x() / ui->videoLabel->width();
        positionConfig.endPoint.y = coordinate * points[1].y() / ui->videoLabel->height();
    }
    int err = IRC_NET_Ptz3DPosition(m_handle, &positionConfig);
    QString temp = tr("Ptz3DPosition");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void MainWindow::OnRegionFocus(QList<QPoint> points)
{
    const int coordinate = 8192;
    QList<QPoint> pointList;
    for (const auto& point : points)
    {
        int x = coordinate * point.x() / ui->videoLabel->width();
        int y = coordinate * point.y() / ui->videoLabel->height();
        pointList.push_back(QPoint(x, y));
    }

    IRC_NET_PTZ_REGION_FOCUS_PARAM positionFocusParam;
    positionFocusParam.channel = m_onlineChannel;
    if (points.size() == 2)
    {
        positionFocusParam.startPoint.x = coordinate * points[0].x() / ui->videoLabel->width();
        positionFocusParam.startPoint.y = coordinate * points[0].y() / ui->videoLabel->height();
        positionFocusParam.endPoint.x = coordinate * points[1].x() / ui->videoLabel->width();
        positionFocusParam.endPoint.y = coordinate * points[1].y() / ui->videoLabel->height();
    }
    positionFocusParam.stop = 0;
    int err = IRC_NET_PtzRegionFocus(m_handle, &positionFocusParam);
    QString temp = tr("PtzRegionFocus");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui->videoLabel->SetDrawMode(VideoLabel::NO_DRAW);
}

void MainWindow::OnManualTrack(QList<QPoint> points)
{
    const int coordinate = 8192;
    QList<QPoint> pointList;
    for (const auto& point : points)
    {
        int x = coordinate * point.x() / ui->videoLabel->width();
        int y = coordinate * point.y() / ui->videoLabel->height();
        pointList.push_back(QPoint(x, y));
    }

    IRC_NET_PTZ_MANUAL_TRACK_PARAM positionManualParam;
    positionManualParam.channel = m_onlineChannel;
    if (points.size() == 2)
    {
        positionManualParam.startPoint.x = coordinate * points[0].x() / ui->videoLabel->width();
        positionManualParam.startPoint.y = coordinate * points[0].y() / ui->videoLabel->height();
        positionManualParam.endPoint.x = coordinate * points[1].x() / ui->videoLabel->width();
        positionManualParam.endPoint.y = coordinate * points[1].y() / ui->videoLabel->height();
    }
    positionManualParam.stop = 0;
    int err = IRC_NET_PtzManualTrack(m_handle, &positionManualParam);
    QString temp = tr("PtzManualTrack");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void MainWindow::OnDownloadProgressButtonClicked()
{
    m_downloadPage->show();
}


