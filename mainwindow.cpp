#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <motorthread.h>
#include <epos4_can.h>
#include "qstring.h"

MainWindow* theWindow = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   // CKim - Initialize variables
    target_vel = 0;
    // CKim - wiringPi Set
    wiringPiSetup();
    m_Thread.m_emergencyMainWindow=0;
    //VYS ADD START
    pump_MotorSpeed = 0;
    speedLabel.sprintf(" %d ",pump_MotorSpeed);
    ui->p_MotorSpeedInfo->setText(speedLabel);
    pinMode(PUMP_ENABLE,OUTPUT);
    pinMode(PUMP_DIR,OUTPUT);
    pinMode(PUMP_HARDPWM,PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(480);
    pwmSetClock(2);
    digitalWrite(PUMP_ENABLE,0);
    //VYS ADD FINISH
    connect(&m_Thread, &motorThread::UpdateGUI, this, &MainWindow::StateChanged);
    connect(&emergencyWindow,&Emergency_Window::Emergency_Exit_Clicked,this,&MainWindow::Emergency_Window_Exit);
    if(!m_Thread.isRunning()) m_Thread.start();
}

MainWindow::~MainWindow()
{
    m_Thread.m_running = false;
    delete ui;
}
void MainWindow::Emergency_Window_Exit(int a)
{
    m_Thread.m_emergencyMainWindow=a;
}
void MainWindow::StateChanged(int state)
{
    if (state==DEBRIDER_STATE_EMERGENCY && m_Thread.m_emergency)
    {
        emergencyWindow.m_EmergencyStatus=state;
        on_btnEmergencyMode_clicked();
        on_radioCW_toggled(true);
        m_Thread.m_TargetVel = target_vel=0;
        QString qstr;
        qstr = QString("%1").arg(target_vel);
        ui->p_BLDCspeedInfo->setText(qstr);
    }
    if(state==DEBRIDER_STATE_ENABLED)
    {
        m_Thread.m_emergencyMainWindow=0;
        // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS START  ###########
        if (m_Thread.btn_MAXRPM_GUI && !(ui->radioMAXRPM->isChecked()))
        {
            ui->radioMAXRPM->setChecked(true);
            on_radioMAXRPM_clicked();
            m_Thread.btn_MAXRPM_GUI=0;
        }
        else if (m_Thread.btn_MAXRPM_GUI && (ui->radioMAXRPM->isChecked()))
        {
            ui->radioMAXRPM->setChecked(false);
            m_Thread.btn_MAXRPM_GUI=0;
        }
       // #########  HARDWARE MAXRPM BUTTON CLICKED SETTINGS FINISH   #########

        // #########  HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS START   #########
        if(m_Thread.btn_ChangeDir_GUI)
        {
            if(ui->radioCW->isChecked())
            {
                ui->radioCCW->setChecked(true);
                on_radioCCW_toggled(true);
                m_Thread.btn_ChangeDir_GUI=0;
            }
            else if(ui->radioCCW->isChecked())
            {
                ui->radioOSC->setChecked(true);
                on_radioOSC_toggled(true);
                m_Thread.btn_ChangeDir_GUI=0;
            }
            else
            {
                ui->radioCW->setChecked(true);
                on_radioCW_toggled(true);
                m_Thread.btn_ChangeDir_GUI=0;
            }
        }
        emergencyWindow.close();
    }
    // #########  HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS FINISH   #########

    if(state == DEBRIDER_STATE_RUNNING || state == DEBRIDER_STATE_OSC)
    {
        ui->btnDecreaseRPM->setEnabled(false);
        ui->btnIncreaseRPM->setEnabled(false);
        ui->btnCloseBlade->setEnabled(false);
        ui->radioCCW->setEnabled(false);
        ui->radioCW->setEnabled(false);
        ui->radioOSC->setEnabled(false);
        ui->radioMAXRPM->setEnabled(false);
        emergencyWindow.close();
    }
    else if(state == DEBRIDER_STATE_SERIAL_ERROR || state == DEBRIDER_STATE_EPOS_ERROR)
    {
        on_btnEmergencyMode_clicked();
        on_radioCW_toggled(true);
        m_Thread.m_TargetVel = target_vel=0;
    }
        else 
    {
        ui->btnDecreaseRPM->setEnabled(true);
        ui->btnIncreaseRPM->setEnabled(true);
        ui->btnCloseBlade->setEnabled(true);
        ui->radioCCW->setEnabled(true);
        ui->radioCW->setEnabled(true);
        ui->radioOSC->setEnabled(true);
        ui->radioMAXRPM->setEnabled(true);
    }
    if(m_Thread.m_NewTargetVel < 0)
    {
        statusLabel.sprintf(" CW Debrider Velocity:  %ld RPM \n Pump Motor : %d RPM ",
                        m_Thread.m_NewTargetVel*(-1),pump_MotorSpeed);
        ui->lblMsg->setText(statusLabel);
    }
    else if(m_Thread.m_NewTargetVel)
     {
        statusLabel.sprintf(" CCW Debrider Velocity:  %ld RPM \n Pump Motor : %d RPM ",
                            m_Thread.m_NewTargetVel,pump_MotorSpeed);
        ui->lblMsg->setText(statusLabel);
    }
    else 
    {
        statusLabel.sprintf(" Debrider Velocity Set:  %ld RPM \n Pump Motor : %d RPM ",
                            target_vel,pump_MotorSpeed);
        ui->lblMsg->setText(statusLabel);
    }
    emergencyWindow.m_EmergencyStatus=state;
}

void MainWindow::on_btnDecreaseRPM_clicked()
{
    ui->radioMAXRPM->setChecked(false);
    target_vel -= CHANGE_RPM_RATE;
    QString qstr;
    if(target_vel < 0)
    {
        target_vel = 0;
        ui->lblMsg->setText(QString("error : cannot decrease RPM"));
    }
    else
    {
        statusLabel.sprintf(" Debrider Motor Set:  %ld RPM \n Pump Motor : %d RPM ",target_vel,pump_MotorSpeed);
        ui->lblMsg->setText(statusLabel);
    }
     qstr = QString("%1").arg(target_vel);
     ui->p_BLDCspeedInfo->setText(qstr);

    if(ui->radioCCW->isChecked())
    {
        m_Thread.m_TargetVel = target_vel;
    }
    else
    {
        m_Thread.m_TargetVel = -target_vel;
    }
}

void MainWindow::on_btnIncreaseRPM_clicked()
{
    if(target_vel!=15000) ui->radioMAXRPM->setChecked(false);

        target_vel += CHANGE_RPM_RATE;
        QString qstr;

        if(target_vel > MAX_RPM)
        {
            target_vel = MAX_RPM;
            ui->lblMsg->setText(QString("error : cannot increase \n more than MAX RPM"));
            //cout << "error : cannot increase more than MAX RPM" << endl;
        }
        else 
        {
            statusLabel.sprintf(" Debrider Motor Set:  %ld RPM \n Pump Motor : %d RPM ",target_vel,pump_MotorSpeed);
            ui->lblMsg->setText(statusLabel);
        }

        qstr = QString("%1").arg(target_vel);
        ui->p_BLDCspeedInfo->setText(qstr);

        if(ui->radioCCW->isChecked())
        {
            m_Thread.m_TargetVel = target_vel;
        }
        else
        {
            m_Thread.m_TargetVel = -target_vel;
        }
}

void MainWindow::on_radioCW_toggled(bool checked)
{
    if(checked) 
    {
        m_Thread.m_TargetVel = -target_vel;
        m_Thread.m_Oscillate = 0;
    }
}

void MainWindow::on_radioCCW_toggled(bool checked)
{
   if(checked)
    {
       m_Thread.m_TargetVel = target_vel;
       m_Thread.m_Oscillate = 0;
    }
}

void MainWindow::on_radioOSC_toggled(bool checked)
{
    if(checked)
    {
        m_Thread.m_Oscillate = 1;
    }
}
//VYSADN DC MOTOR START
void MainWindow::on_btnDecreaseFlow_clicked()
{
    if(pump_MotorSpeed > 0)    pump_MotorSpeed-=20;
    if(Pump_Status) pwmWrite(PUMP_HARDPWM,pump_MotorSpeed);
    speedLabel.sprintf(" %d ",pump_MotorSpeed);
    ui->p_MotorSpeedInfo->setText(speedLabel);
    statusLabel.sprintf(" Debrider Motor Set:  %ld RPM \n Pump Motor Set: %d RPM ",target_vel,pump_MotorSpeed);
    ui->lblMsg->setText(statusLabel);
}

void MainWindow::on_btnIncreaseFlow_clicked()
{
    if(pump_MotorSpeed < 480)    pump_MotorSpeed+=20;
    if(Pump_Status) pwmWrite(PUMP_HARDPWM,pump_MotorSpeed);
    speedLabel.sprintf(" %d ",pump_MotorSpeed);
    ui->p_MotorSpeedInfo->setText(speedLabel);
    statusLabel.sprintf(" Debrider Motor Set:  %ld RPM \n Pump Motor Set : %d RPM ",
                        target_vel,pump_MotorSpeed);
    ui->lblMsg->setText(statusLabel);

}
void MainWindow::on_btnIrrigationMove_clicked()
{
    Pump_Status=true;
    pwmWrite(PUMP_HARDPWM,pump_MotorSpeed);
    digitalWrite(PUMP_ENABLE,1);
    speedLabel.sprintf(" %d ",pump_MotorSpeed);
    ui->p_MotorSpeedInfo->setText(speedLabel);
    statusLabel.sprintf(" Debrider Motor :  %ld RPM \n Pump Motor : %d RPM ",
                        target_vel,pump_MotorSpeed);
    ui->lblMsg->setText(statusLabel);
}
void MainWindow::on_btnIrrigationStop_clicked()
{
    Pump_Status=false;
    digitalWrite(PUMP_ENABLE,0);
    pwmWrite(PUMP_HARDPWM,0);
    pump_MotorSpeed=0;
    speedLabel.sprintf(" %d ",pump_MotorSpeed);
    ui->p_MotorSpeedInfo->setText(speedLabel);
    statusLabel.sprintf(" Debrider Motor Set :  %ld RPM \n Pump Motor Set : %d RPM ",
                        target_vel,pump_MotorSpeed);
    ui->lblMsg->setText(statusLabel);
}
//VYSADN DC MOTOR FINISH

void MainWindow::on_btnCloseBlade_clicked()
{
   m_Thread.m_closeBladeWindow=1;
}

void MainWindow::on_btnEmergencyMode_clicked()
{
    if (emergencyWindow.m_EmergencyStatus!=DEBRIDER_STATE_EMERGENCY)
        m_Thread.m_emergencyMainWindow=1;
        emergencyWindow.setModal(true);
        emergencyWindow.setWindowState(Qt::WindowFullScreen);
        emergencyWindow.SetEmergencyText();
        emergencyWindow.exec();
}

void MainWindow::on_radioMAXRPM_clicked()
{
    target_vel = MAX_RPM;
    QString qstr;
    qstr = QString("%1").arg(target_vel);
    ui->p_BLDCspeedInfo->setText(qstr);
    statusLabel.sprintf(" Debrider Motor Set :  %ld RPM \n Pump Motor : %d RPM ",target_vel,pump_MotorSpeed);
    ui->lblMsg->setText(statusLabel);
    m_Thread.m_TargetVel = target_vel;
}
