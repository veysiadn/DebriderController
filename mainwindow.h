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

    void StateChanged(int state);

    void on_radioCW_toggled(bool checked);

    void on_radioCCW_toggled(bool checked);

    void on_radioOSC_toggled(bool checked);

    void on_btnDecreaseFlow_clicked();

    void on_btnIncreaseFlow_clicked();

    void on_btnIrrigationMove_clicked();

    void on_btnIrrigationStop_clicked();
    
    void on_btnEmergencyMode_clicked();
    
    void on_btnCloseBlade_clicked();

    void on_radioMAXRPM_clicked();

private:
    Ui::MainWindow *ui;
      int pump_MotorSpeed;
      bool Pump_Status=false;
      QString speedLabel;
      QString statusLabel;
    // CKim - Motor class

    // CKim - Variables
    long target_vel;    // Target rpm of the debrider
    int current_pos;
    int btn_ChangdirLastState;
    Emergency_Window emergencyWindow;
    motorThread m_Thread;
};

#endif // MAINWINDOW_H
