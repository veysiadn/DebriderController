#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <motorthread.h>
//#include <epos4_can.h>
#include "qstring.h"
#include "wiringPi.h"

MainWindow* theWindow = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // CKim - Initializing Variables
    debriderMotorTargetSpeed = 0;
    pumpMotorTargetSpeed = 0;
    pumpRunningStatus=false;

    // CKim - Initialize Raspberry Pi
    wiringPiSetup();                // CKim - Should be called first before using any wiringPi functions

    // CKim - Initialize RPi's GPIO
    pinMode(PUMP_ENABLE,OUTPUT);
    pinMode(PUMP_DIR,OUTPUT);
    pinMode(PUMP_HARDPWM,PWM_OUTPUT);
    pinMode(WATCHDOG_PIN,OUTPUT);
    pinMode(EMERGENCY_RELAY_CONTROL,INPUT);

    // CKim - Initialize RPi's pwm for Pump control
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(PUMP_PWM_RANGE);    //19.2 Oscillator freq / 480 / 2 = 20kHz
    pwmSetClock(2);
    digitalWrite(PUMP_ENABLE,0);

    // CKim - Connect Signals and Slots
    connect(&m_Thread, &motorThread::UpdateGUI, this, &MainWindow::stateChanged);
    connect(&emergencyWindow,&Emergency_Window::Emergency_Exit_Clicked,
            this,&MainWindow::exitEmergencyWindow);

    // CKim - Start Control Thread
    if(!m_Thread.isRunning()) m_Thread.start();
}

MainWindow::~MainWindow()
{
    //m_Thread.m_running = false;
    delete ui;
}

void MainWindow::exitEmergencyWindow(int a)
{
    m_Thread.guiEmergencyMode=a;
    m_Thread.ReInitialize();
    //stateChanged(DEBRIDER_STATE_ENABLED);
    std::cout << "Exit Clicked ON Emergency Window" << std::endl;
}

void MainWindow::stateChanged(int state)
{
    emergencyWindow.m_EmergencyStatus=state;
    switch(state)
    {
        case DEBRIDER_STATE_ENABLED:
            enableGUI();
            m_Thread.guiEmergencyMode=0;
            stopPumpMotor();
            showPedalBtnStates();
            printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
            break;

        case DEBRIDER_STATE_RUNNING:
            disableGUI();
            movePumpMotor();
            printStatus(m_Thread.m_DebriderInstantSpeed,pumpMotorSpeedPrintVal);
            break;

        case DEBRIDER_STATE_OSC:
            disableGUI();
            movePumpMotor();
            statusLabel.sprintf(" Debrider Motor in Oscillation MODE \n"
                                "Pump Motor Running : %% %d ",pumpMotorSpeedPrintVal);
            ui->lblStatusMsg->setText(statusLabel);
            break;

        case DEBRIDER_STATE_CLOSE_BLADE:
            disableGUI();
            printStatus(m_Thread.m_DebriderInstantSpeed,pumpMotorSpeedPrintVal);
        break;

        case DEBRIDER_STATE_EMERGENCY:
            disableGUI();
            callEmergencyWindow();
            on_radioCW_toggled(true);
            m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed=0;
            pumpMotorTargetSpeed=0;
            stopPumpMotor();
            printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
        break;

        case DEBRIDER_STATE_SERIAL_ERROR:
            disableGUI();
            callEmergencyWindow();
            on_radioCW_toggled(true);
            m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed=0;
            stopPumpMotor();
            pumpMotorTargetSpeed=0;
        break;

        case DEBRIDER_STATE_EPOS_ERROR:
            disableGUI();
            callEmergencyWindow();
            on_radioCW_toggled(true);
            m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed=0;
            stopPumpMotor();
            pumpMotorTargetSpeed=0;
        break;

        default:
             enableGUI();
             printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
    }
}

void MainWindow::on_btnDecreaseRPM_clicked()
{
    ui->radioMAXRPM->setChecked(false);
    debriderMotorTargetSpeed -= CHANGE_RPM_RATE;
    QString qstr;
    if(debriderMotorTargetSpeed < 0)
    {
        debriderMotorTargetSpeed = 0;
        ui->lblStatusMsg->setText(QString("error : cannot decrease RPM\n"
                                          "RPM cannot be less then 0 "));
    }
    else
        printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);

    if(ui->radioCCW->isChecked())
        m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed;
    else
        m_Thread.m_DebriderTargetSpeed = -debriderMotorTargetSpeed;
}

void MainWindow::on_btnIncreaseRPM_clicked()
{
    debriderMotorTargetSpeed += CHANGE_RPM_RATE;

    if(debriderMotorTargetSpeed > BLDC_MAX_RPM)
    {
        debriderMotorTargetSpeed = BLDC_MAX_RPM;
        ui->lblStatusMsg->setText(QString("error : cannot increase more than MAX RPM\n"
                                          "Max RPM for Debrider : 15000 RPM"));
    }
    else
        printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);

    if(debriderMotorTargetSpeed==BLDC_MAX_RPM)
        ui->radioMAXRPM->setChecked(true);

    if(ui->radioCCW->isChecked())
      m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed;
    else
      m_Thread.m_DebriderTargetSpeed = -debriderMotorTargetSpeed;
}

void MainWindow::on_radioCW_toggled(bool checked)
{
    if(checked) 
    {
        m_Thread.m_DebriderTargetSpeed = -debriderMotorTargetSpeed;
        m_Thread.m_Oscillate = 0;
    }
}

void MainWindow::on_radioCCW_toggled(bool checked)
{
   if(checked)
    {
       m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed;
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

void MainWindow::on_btnDecreaseFlow_clicked()
{
    // CKim - Pump speed updated here, perhaps pwm command should be issued in main thread instead of here??
    if(pumpMotorTargetSpeed > 0)    pumpMotorTargetSpeed-=48;
    if(pumpRunningStatus) pwmWrite(PUMP_HARDPWM,pumpMotorTargetSpeed);
    pumpMotorSpeedPrintVal=int(pumpMotorTargetSpeed/4.8);
    printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
}

void MainWindow::on_btnIncreaseFlow_clicked()
{
    if(pumpMotorTargetSpeed < PUMP_MAX_PWM)    pumpMotorTargetSpeed+=48;
    if(pumpRunningStatus) pwmWrite(PUMP_HARDPWM,pumpMotorTargetSpeed);
    pumpMotorSpeedPrintVal=int(pumpMotorTargetSpeed/4.8);
    printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
}

void MainWindow::on_btnIrrigationMove_clicked()
{
    movePumpMotor();
    printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
}

void MainWindow::on_btnIrrigationStop_clicked()
{
    pumpMotorTargetSpeed=0;
    stopPumpMotor();
    printStatus(debriderMotorTargetSpeed,pumpMotorSpeedPrintVal);
}

void MainWindow::on_btnCloseBlade_clicked()
{
   m_Thread.guiBtnCloseBlade=1;
}

void MainWindow::on_radioMAXRPM_clicked()
{
    debriderMotorTargetSpeed = BLDC_MAX_RPM;
    printStatus(debriderMotorTargetSpeed,pumpMotorTargetSpeed);
    m_Thread.m_DebriderTargetSpeed = debriderMotorTargetSpeed;
}

void MainWindow::stopPumpMotor()
{
    pumpRunningStatus=false;
    pwmWrite(PUMP_HARDPWM,0);
    //digitalWrite(PUMP_ENABLE,0);
}

void MainWindow::movePumpMotor()
{
    int pumpSpeed;
    pumpSpeed=int(pumpMotorTargetSpeed);
    pumpRunningStatus=true;
    pwmWrite(PUMP_HARDPWM,pumpSpeed);
    digitalWrite(PUMP_ENABLE,1);
}

void MainWindow::printStatus(int dSpeed, int pSpeed)
{
    speedLabel.sprintf(" %d ", debriderMotorTargetSpeed);
    ui->p_BLDCspeedInfo->setText(speedLabel);
    speedLabel.sprintf(" %d ",pSpeed);
    ui->p_MotorSpeedInfo->setText(speedLabel);
    if(pumpRunningStatus==false)
    statusLabel.sprintf(" Debrider Motor Set :  %d RPM \n Pump Motor Set : %% %d  ",dSpeed,pSpeed);
    else
    statusLabel.sprintf(" Debrider Motor Set:  %d RPM \n Pump Motor Running at : %% %d  ",dSpeed,pSpeed);
    if(m_Thread.m_DebriderInstantSpeed < 0)
    {
        statusLabel.sprintf(" Debrider Motor Running at CW MODE at :  %d RPM \n "
                            "Pump Motor Running at : %% %d  ",dSpeed*(-1),pSpeed);
    }
    else if(m_Thread.m_DebriderInstantSpeed > 0)
    {
        statusLabel.sprintf(" Debrider Motor Running at CCW MODE at :  %d RPM \n "
                            "Pump Motor Running at : %% %d  ",dSpeed,pSpeed);
    }
    ui->lblStatusMsg->setText(statusLabel);
}

void MainWindow::enableGUI()
{
    ui->btnDecreaseRPM->setEnabled(true);
    ui->btnIncreaseRPM->setEnabled(true);
    ui->btnCloseBlade->setEnabled(true);
    ui->radioCCW->setEnabled(true);
    ui->radioCW->setEnabled(true);
    ui->radioOSC->setEnabled(true);
    ui->radioMAXRPM->setEnabled(true);
    ui->btnDecreaseFlow->setEnabled(true);
    ui->btnIncreaseFlow->setEnabled(true);
    ui->btnIrrigationMove->setEnabled(true);
    ui->btnIrrigationStop->setEnabled(true);
    ui->p_BLDCspeedInfo->setEnabled(true);
    ui->p_MotorSpeedInfo->setEnabled(true);
}

void MainWindow::disableGUI()
{
    ui->btnDecreaseRPM->setEnabled(false);
    ui->btnIncreaseRPM->setEnabled(false);
    ui->btnCloseBlade->setEnabled(false);
    ui->radioCCW->setEnabled(false);
    ui->radioCW->setEnabled(false);
    ui->radioOSC->setEnabled(false);
    ui->radioMAXRPM->setEnabled(false);
    ui->btnDecreaseFlow->setEnabled(false);
    ui->btnIncreaseFlow->setEnabled(false);
    ui->btnIrrigationMove->setEnabled(false);
    ui->btnIrrigationStop->setEnabled(false);
    ui->p_BLDCspeedInfo->setEnabled(false);
    ui->p_MotorSpeedInfo->setEnabled(false);
}

// CKim - Updates gui from the button inputs
void MainWindow::showPedalBtnStates()
{
    // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS START  ###########
    if (m_Thread.guiBtnMaxRPM && !(ui->radioMAXRPM->isChecked()))
    {
        ui->radioMAXRPM->setChecked(true);
        on_radioMAXRPM_clicked();
        m_Thread.guiBtnMaxRPM=0;
    }
    else if (m_Thread.guiBtnMaxRPM && (ui->radioMAXRPM->isChecked()))
    {
        ui->radioMAXRPM->setChecked(false);
        m_Thread.guiBtnMaxRPM=0;
    }
   // #########  HARDWARE MAXRPM BUTTON CLICKED SETTINGS FINISH   #########

    // #########  HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS START   #########
    if(m_Thread.guiBtnChangeDirection)
    {
        if(ui->radioCW->isChecked())
        {
            ui->radioCCW->setChecked(true);
            on_radioCCW_toggled(true);
            m_Thread.guiBtnChangeDirection=0;
        }
        else if(ui->radioCCW->isChecked())
        {
            ui->radioOSC->setChecked(true);
            on_radioOSC_toggled(true);
            m_Thread.guiBtnChangeDirection=0;
        }
        else
        {
            ui->radioCW->setChecked(true);
            on_radioCW_toggled(true);
            m_Thread.guiBtnChangeDirection=0;
        }
    }
    emergencyWindow.close();
}
// #########  HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS FINISH   #########

void MainWindow::callEmergencyWindow()
{
    m_Thread.guiEmergencyMode=1;
    emergencyWindow.SetEmergencyText();
    emergencyWindow.setModal(true);
    emergencyWindow.setWindowState(Qt::WindowFullScreen);
    emergencyWindow.exec();
}
