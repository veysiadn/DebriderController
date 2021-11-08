#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow* theWindow = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitializeIO();

    connect(&m_Thread, &motorThread::UpdateGUI, this, &MainWindow::on_StateChanged);
    connect(&emergencyWindow,&Emergency_Window::EmergencyExitClicked,this,&MainWindow::on_ExitEmergencyClicked);

    if(!m_Thread.isRunning()) m_Thread.start();
}

MainWindow::~MainWindow()
{
    //m_Thread.m_running = false;
    delete ui;
}

void MainWindow::InitializeIO()
{
    // CKim - Initializing Variables
    debrider_motor_target_speed = 0;
    pump_motor_target_speed = 0;
    pump_running_status=false;

    // CKim - Initialize Raspberry Pi
    wiringPiSetupGpio(); // VysAdn, instead of WiringPi pin numbering this will use GPIO pin numbering.

    // CKim - Initialize RPi's GPIO
    pinMode(PUMP_HARDPWM,PWM_OUTPUT);
    pinMode(WATCHDOG_PIN,OUTPUT);
    pinMode(EMERGENCY_RELAY_CONTROL,INPUT);

    //VysAdn Valve enable/disable input init.
    pinMode(VALVE_ENABLE,OUTPUT);
    DisableValve();
    // VysAdn ADC SPI input/output init.
    pinMode(ADC_SPI_CLK,OUTPUT);
    pinMode(ADC_SPI_MOSI,OUTPUT);
    pinMode(ADC_SPI_CS,OUTPUT);
    pinMode(ADC_SPI_MISO,INPUT);
    // CKim - Initialize RPi's pwm for Pump control
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(PUMP_PWM_RANGE);    //19.2 Oscillator freq / 480 / 2 = 20kHz
    pwmSetClock(2);
}

void MainWindow::on_ExitEmergencyClicked(int a)
{
    m_Thread.guiEmergencyMode=a;
    m_Thread.ReInitialize();
    //stateChanged(DEBRIDER_STATE_ENABLED);
    std::cout << "Exit Clicked ON Emergency Window" << std::endl;
}

void MainWindow::on_StateChanged(int state)
{
    emergencyWindow.emergency_status=state;
    switch(state)
    {
        case DEBRIDER_STATE_ENABLED:
            EnableGUI();
            m_Thread.guiEmergencyMode=0;
            StopPumpMotor();
            DisableValve();
            ShowPedalButtonStates();
            PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
            break;

        case DEBRIDER_STATE_RUNNING:
            DisableGUI();
            MovePumpMotor();
            EnableValve();
            PrintStatus(m_Thread.m_DebriderInstantSpeed,pump_motor_printed_speed_val);
            break;

        case DEBRIDER_STATE_OSC:
            DisableGUI();
            MovePumpMotor();
            EnableValve();
            status_label.sprintf(" Debrider Motor in Oscillation MODE \n"
                                "Pump Motor Running : %% %d ",pump_motor_printed_speed_val);
            ui->lblStatusMsg->setText(status_label);
            break;

        case DEBRIDER_STATE_CLOSE_BLADE:
            DisableGUI();
            PrintStatus(m_Thread.m_DebriderInstantSpeed,pump_motor_printed_speed_val);
        break;

        case DEBRIDER_STATE_EMERGENCY:
            DisableGUI();
            ui->radioMAXRPM->setChecked(false);
            ui->radioCW->setChecked(true);
            on_radioCW_toggled(true);
            m_Thread.m_DebriderTargetSpeed=0;
            debrider_motor_target_speed=0;
            pump_motor_target_speed=0;
            pump_motor_printed_speed_val=0;
            StopPumpMotor();
            DisableValve();
            PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
            on_CallEmergencyWindow();
        break;

        case DEBRIDER_STATE_SPI_ERROR:
            DisableGUI();
            ui->radioMAXRPM->setChecked(false);
            ui->radioCW->setChecked(true);
            on_radioCW_toggled(true);
            m_Thread.m_DebriderTargetSpeed = 0;
            debrider_motor_target_speed=0;
            StopPumpMotor();
            DisableValve();
            pump_motor_target_speed=0;
            pump_motor_printed_speed_val=0;
            PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
            on_CallEmergencyWindow();
        break;

        case DEBRIDER_STATE_EPOS_ERROR:
            DisableGUI();
            ui->radioMAXRPM->setChecked(false);
            ui->radioCW->setChecked(true);
            on_radioCW_toggled(true);
            m_Thread.m_DebriderTargetSpeed = 0;
            debrider_motor_target_speed=0;
            StopPumpMotor();
            pump_motor_target_speed=0;
            pump_motor_printed_speed_val=0;
            PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
            on_CallEmergencyWindow();
        break;

        default:
             EnableGUI();
             PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
    }
}

void MainWindow::on_btnDecreaseRPM_clicked()
{
    ui->radioMAXRPM->setChecked(false);
    debrider_motor_target_speed -= CHANGE_RPM_RATE;
    QString qstr;
    if(debrider_motor_target_speed < 0)
    {
        debrider_motor_target_speed = 0;
        ui->lblStatusMsg->setText(QString("error : cannot decrease RPM\n"
                                          "RPM cannot be less then 0 "));
    }
    else
        PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);

    if(ui->radioCCW->isChecked())
        m_Thread.m_DebriderTargetSpeed = debrider_motor_target_speed;
    else
        m_Thread.m_DebriderTargetSpeed = -debrider_motor_target_speed;
}

void MainWindow::on_btnIncreaseRPM_clicked()
{
    debrider_motor_target_speed += CHANGE_RPM_RATE;

    if(debrider_motor_target_speed > BLDC_MAX_RPM)
    {
        debrider_motor_target_speed = BLDC_MAX_RPM;
        ui->lblStatusMsg->setText(QString("error : cannot increase more than MAX RPM\n"
                                          "Max RPM for Debrider : 12000 RPM"));
    }
    else
        PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);

    if(debrider_motor_target_speed==BLDC_MAX_RPM)
        ui->radioMAXRPM->setChecked(true);

    if(ui->radioCCW->isChecked())
      m_Thread.m_DebriderTargetSpeed = debrider_motor_target_speed;
    else
      m_Thread.m_DebriderTargetSpeed = -debrider_motor_target_speed;
}

void MainWindow::on_radioCW_toggled(bool checked)
{
    if(checked) 
    {
        m_Thread.m_DebriderTargetSpeed = -debrider_motor_target_speed;
        m_Thread.m_Oscillate = 0;
    }
}

void MainWindow::on_radioCCW_toggled(bool checked)
{
   if(checked)
    {
       m_Thread.m_DebriderTargetSpeed = debrider_motor_target_speed;
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
    if(pump_motor_target_speed >= 120)    pump_motor_target_speed-=40;
    if(pump_running_status) pwmWrite(PUMP_HARDPWM,pump_motor_target_speed);
    pump_motor_printed_speed_val=int(((pump_motor_target_speed-120.0)/4.0)+10.0);
    PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
}

void MainWindow::on_btnIncreaseFlow_clicked()
{
    if(pump_motor_printed_speed_val==0) pump_motor_target_speed=80;
    if(pump_motor_target_speed < PUMP_PWM_RANGE)    pump_motor_target_speed+=40;
    if(pump_running_status) pwmWrite(PUMP_HARDPWM,pump_motor_target_speed);
    pump_motor_printed_speed_val=int(((pump_motor_target_speed-120.0)/4.0)+10);
    PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
}

void MainWindow::on_btnIrrigationMove_clicked()
{
    MovePumpMotor();
    PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
}

void MainWindow::on_btnIrrigationStop_clicked()
{
    //pumpMotorTargetSpeed=0;
    StopPumpMotor();
   // pumpMotorSpeedPrintVal=0;
    PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
}

void MainWindow::on_btnCloseBlade_clicked()
{
   m_Thread.guiBtnCloseBlade=1;
}

void MainWindow::on_radioMAXRPM_clicked()
{
    debrider_motor_target_speed = BLDC_MAX_RPM;
    PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);
    if(ui->radioCCW->isChecked())
      m_Thread.m_DebriderTargetSpeed = debrider_motor_target_speed;
    else
      m_Thread.m_DebriderTargetSpeed = -debrider_motor_target_speed;
}

void MainWindow::StopPumpMotor()
{
    pump_running_status=false;
    pwmWrite(PUMP_HARDPWM,0); 
}

void MainWindow::MovePumpMotor()
{
    int pumpSpeed;
    pumpSpeed=int(pump_motor_target_speed);
    pump_running_status=true;
    pwmWrite(PUMP_HARDPWM,pumpSpeed);
}

void MainWindow::PrintStatus(int dSpeed, int pSpeed)
{
    speed_label.sprintf(" %d ", debrider_motor_target_speed);
    ui->p_BLDCspeedInfo->setText(speed_label);
    speed_label.sprintf(" %d ",pSpeed);
    ui->p_MotorSpeedInfo->setText(speed_label);
    if(pump_running_status==false)
    status_label.sprintf(" Debrider Motor Set :  %d RPM \n Pump Motor Set : %% %d  ",dSpeed,pSpeed);
    else
    status_label.sprintf(" Debrider Motor Set:  %d RPM \n Pump Motor Running at : %% %d  ",dSpeed,pSpeed);
    if(m_Thread.m_DebriderInstantSpeed < 20 && pump_running_status==true)
    {
        status_label.sprintf(" Debrider Motor Running at CW MODE at :  %d RPM \n "
                            "Pump Motor Running at : %% %d  ",m_Thread.m_DebriderInstantSpeed*(-1),pSpeed);
    }
    else if(m_Thread.m_DebriderInstantSpeed > 20 && pump_running_status==true)
    {
        status_label.sprintf(" Debrider Motor Running at CCW MODE at :  %d RPM \n "
                            "Pump Motor Running at : %% %d  ",m_Thread.m_DebriderInstantSpeed,pSpeed);
    }
    ui->lblStatusMsg->setText(status_label);
}

void MainWindow::EnableGUI()
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

void MainWindow::DisableGUI()
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
void MainWindow::ShowPedalButtonStates()
{
    // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS START  ###########
int debriderMotorSetSpeed=debrider_motor_target_speed;
if(m_Thread.guiChangePresetRPM){
            if((debrider_motor_target_speed < 1000)  || debrider_motor_target_speed==BLDC_MAX_RPM) debriderMotorSetSpeed=1000;
            if((debrider_motor_target_speed >= 1000) && debrider_motor_target_speed < 3000) debriderMotorSetSpeed=3000;
            if((debrider_motor_target_speed >= 3000) && debrider_motor_target_speed < 5000) debriderMotorSetSpeed=5000;
            if((debrider_motor_target_speed >= 5000) && debrider_motor_target_speed < 7000) debriderMotorSetSpeed=7000;
            if((debrider_motor_target_speed >= 7000) && debrider_motor_target_speed < 10000) debriderMotorSetSpeed=10000;
            if((debrider_motor_target_speed >= 10000) && debrider_motor_target_speed < 12000) debriderMotorSetSpeed=BLDC_MAX_RPM;
            debrider_motor_target_speed=debriderMotorSetSpeed;
            if(debrider_motor_target_speed!=BLDC_MAX_RPM){ ui->radioMAXRPM->setChecked(false);  }
            else                               { ui->radioMAXRPM->setChecked(true);   }

            m_Thread.guiChangePresetRPM=0;
            if(ui->radioCCW->isChecked())
              m_Thread.m_DebriderTargetSpeed = debrider_motor_target_speed;
            else if(ui->radioCW->isChecked())
              m_Thread.m_DebriderTargetSpeed = -debrider_motor_target_speed;
            else m_Thread.m_DebriderTargetSpeed = debrider_motor_target_speed;
            PrintStatus(debrider_motor_target_speed,pump_motor_printed_speed_val);

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

void MainWindow::on_CallEmergencyWindow()
{
    m_Thread.guiEmergencyMode=1;
    emergencyWindow.SetEmergencyText();
    emergencyWindow.setModal(true);
    emergencyWindow.setWindowState(Qt::WindowFullScreen);
    emergencyWindow.exec();
}

void MainWindow::EnableValve()
{
    digitalWrite(VALVE_ENABLE,1);
}

void MainWindow::DisableValve()
{
    digitalWrite(VALVE_ENABLE,0);
}
