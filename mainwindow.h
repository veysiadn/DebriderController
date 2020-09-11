#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
// CKim - Headers
#include "epos4_can.h"
#include <motorthread.h>
#include <QElapsedTimer>
#include <m_defines.h>
#include "emergency_window.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    private slots:
    void on_btnDecreaseRPM_clicked();

    void on_btnIncreaseRPM_clicked();

    void on_radioCW_toggled(bool checked);

    void on_radioCCW_toggled(bool checked);

    void on_radioOSC_toggled(bool checked);

    void on_btnDecreaseFlow_clicked();

    void on_btnIncreaseFlow_clicked();

    void on_btnIrrigationMove_clicked();

    void on_btnIrrigationStop_clicked();
    
    void on_btnCloseBlade_clicked();

    void on_radioMAXRPM_clicked();

    void stateChanged(int state);

    void callEmergencyWindow();

    void exitEmergencyWindow(int a);

private:
      Ui::MainWindow *ui;
      QString speedLabel;
      QString statusLabel;

      float pumpMotorTargetSpeed;
      int pumpMotorSpeedPrintVal=0;
      bool pumpRunningStatus=false;
      int debriderMotorTargetSpeed;

      Emergency_Window emergencyWindow;
      motorThread m_Thread;

private:
      void printStatus(int dSpeed, int pSpeed);

      void stopPumpMotor();

      void movePumpMotor();

      void enableGUI();

      void disableGUI();

      void showPedalBtnStates();

};

#endif // MAINWINDOW_H
