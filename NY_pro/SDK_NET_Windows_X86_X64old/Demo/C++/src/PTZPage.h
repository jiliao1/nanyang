#pragma once

#include <QWidget>
#include "ui_PTZPage.h"
#include "IRCNetSDKDef.h"

class MainWindow;
class PTZPage : public QWidget
{
    Q_OBJECT

public:
    PTZPage(QWidget *parent = nullptr);
    ~PTZPage();
    void InitButton();
    void GetAuxiState();
    void ClearUi();

signals:
    void Position3D(bool state);
    void PositionRegionFocus(bool state);
    void PositionManualTrack(bool state);

protected:
    void InitForm();
    void ConnectSignalSlot();
    bool GetPreset();
    bool GetCruise();
    void RefreshCruiseTable(int cruiseId);
    void RefreshRegionScan();
    void RefreshBootAction(IRC_NET_BOOT_ACTION_INFO bootActionInfo);
    void RefreshParkAction(IRC_NET_PARK_ACTION_INFO* parkActionInfo);
    void RefreshShowCombo(bool state,int id);

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void OnUpBtnPressed();
    void OnDownBtnPressed();
    void OnLeftBtnPressed();
    void OnRightBtnPressed();
    void OnUpLeftBtnPressed();
    void OnUpRightBtnPressed();
    void OnDownLeftBtnPressed();
    void OnDownRightBtnPressed();
    void OnBtnReleased();
    void OnZoomInBtnPressed();
    void OnZoomInBtnReleased();
    void OnZoomOutBtnPressed();
    void OnZoomOutBtnReleased();
    void OnFocusNearBtnPressed();
    void OnFocusNearBtnReleased();
    void OnFocusFarBtnPressed();
    void OnFocusFarBtnReleased();
    void OnIrisCloseBtnPressed();
    void OnIrisCloseBtnRealeased();
    void OnIrisOpenBtnPressed();
    void OnIrisOpenBtnRealeased();
    void OnWiperBtnClicked();
    void OnLightBtnClicked();
    void OnDefrostBtnClicked();
    void OnFogLampBtnClicked();
    void OnHearterBtnClicked();
    void OnFanBtnClicked();
    void OnThreeDimensionalBtnClicked();
    void OnRegionFocusBtnClicked();
    void OnManualTrackBtnClicked();
    void OnArrowsBtnClicked();
    void OnAutoMaticBtnClicked();
    void OnPresetGetAllButtonClicked();
    void OnPresetDeleteAllButtonClicked();
    void OnSaveButtonClicked();
    void OnMoveToButtonClicked();
    void OnGetCurrentPresetBtnClicked();
    void OnDeleteButtonClicked();
    void OnCruiseGetAllButtonClicked();
    void OnCruiseDeleteAllButtonClicked();
    void OnCruiseSaveButtonClicked();
    void OnCruiseDeleteButtonClicked();
    void OnCruiseStartButtonClicked();
    void OnAddPresetButtonClicked();
    void OnDeletePresetButtonClicked();
    void OnPatrolGetAllButtonClicked();
    void OnPatrolDeleteAllButtonClicked();
    void OnStartPatrolButtonClicked();
    void OnStartRecordButtonClicked();
    void OnSetButtonClicked();
    void OnPresetComboChanged(int index);
    void OnCruiseComboChanged(int index);
    void OnPatrolComboChanged(int index);
    void OnResetBtnClicked();
    void OnLensInitBtnClicked();
    void OnLaserDistanceBtnClicked();
    void OnGetAllAreaScanInfoBtnClicked();
    void OnGetOneAreaScanBtnClicked();
    void OnSetAreaScanInfoBtnClicked();
    void OnAreaScanStartBtnClicked();
    void OnAreaScanStopBtnClicked();
    void OnDeleteOneAreaScanBtnClicked();
    void OnDeleteAllAreaScanBtnClicked();
    void OnBootRefreshBtnClicked();
    void OnBootConfirmBtnClicked();
    void OnBootPresetRadioBtnClicked();
    void OnBootTourRadioBtnClicked();
    void OnBootPatternRadioBtnClicked();
    void OnBootAreaScanRadioBtnClicked();
    void OnBootLineScanRadioBtnClicked();
    void OnBootAutoResetRadioBtnClicked();
    void OnParkRefreshBtnClicked();
    void OnParkConfirmBtnClicked();
    void OnGetCoordinatesBtnClicked();
    void OnSetATBtnClicked();
    void OnGetZoomBtnClicked();
    void OnSetZoomBtnClicked();
    void OnTargetConfigGetBtnClicked();
    void OnTargetConfigSetBtnClicked();
    void OnGetBearingBtnClicked();


private:
    Ui::PTZPageClass ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;
    PTZPage* m_ptzPage = nullptr;
    QMap<int, IRC_NET_PTZ_PRESET_INFO> m_presetIndexMap;
    QMap<int, IRC_NET_PTZ_TOUR_INFO> m_cruiseIndexMap;
    QMap<int, IRC_NET_REGION_SCAN_INFO> m_regionScanInfoMap;
    QMap<int, IRC_NET_PTZ_PATTERN_INFO> m_patternInfoMap;
};
