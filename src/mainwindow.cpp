#include "include/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    on_CallInitWindow();

    /// Initialize wiringpi library and pinmapping.
    InitializeIO();

    /// CKim - Connect signals and sloths.
    connect(&motor_thread_, &MotorThread::UpdateGUI, this, &MainWindow::on_StateChanged);

    connect(&emergency_window_,&EmergencyWindow::EmergencyExitClicked,this,&MainWindow::on_ExitEmergencyClicked);

    connect(this,&MainWindow::InitStateChanged, &init_window_, &InitializationWindow::UpdateInitGUI);

    connect(&init_window_, &InitializationWindow::btn_reinit_clicked,this,&MainWindow::on_ReinitClicked);

    if(!motor_thread_.isRunning()) motor_thread_.start();
}

MainWindow::~MainWindow()
{
    //m_Thread.m_running = false;
    delete ui;
}

void MainWindow::on_ExitEmergencyClicked(int a)
{
    if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW){
        std::cout << "Emergency button not released\n" << std::endl;
        return;
    }
    if(digitalRead(INITIALIZATION_SWTICH)==HIGH){
        std::cout << "Initialization Switch not released\n" << std::endl;
        return;
    }
    std::cout << "Exit Clicked ON Emergency Window\n" << std::endl;
    motor_thread_.m_GuiEmergencyMode=a;
    //stateChanged(DEBRIDER_STATE_ENABLED);
    motor_thread_.ReInitialize();
    on_CallInitWindow();
}

void MainWindow::on_ReinitClicked(int state)
{
    if(state < DEBRIDER_STATE_INIT){
        motor_thread_.ReInitialize();
    }
}

void MainWindow::on_StateChanged(int state)
{
    emergency_window_.SetEmergencyStatus(state);
    switch(state)
    {
        case DEBRIDER_STATE_READY:
            SetDefaultUI();
            StopPumpMotor();
            DisableValve();
            PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
            emit InitStateChanged(DEBRIDER_STATE_READY);
            break;

        case DEBRIDER_STATE_ENABLED:
            ShowControlUI();
            EnableGUI();
            motor_thread_.m_GuiEmergencyMode=0;
            StopPumpMotor();
            DisableValve();
            ShowPedalButtonStates();
            PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
            buzzer_running_status=false;
            break;

        case DEBRIDER_STATE_RUNNING:
            DisableGUI();
            MovePumpMotor();
            EnableValve();
            PrintStatus(motor_thread_.m_DebriderInstantSpeed,pump_motor_printed_speed_val_);
            EnableBuzzer();
            break;

        case DEBRIDER_STATE_OSC:
            DisableGUI();
            MovePumpMotor();
            EnableValve();
            EnableBuzzer();
            status_label_.sprintf(" Debrider Motor in Oscillation MODE \n"
                                "Pump Motor Running : %% %d ",pump_motor_printed_speed_val_);
            ui->lblStatusMsg->setText(status_label_);
            break;

        case DEBRIDER_STATE_CLOSE_BLADE:
            DisableGUI();
            EnableBuzzer();
            PrintStatus(motor_thread_.m_DebriderInstantSpeed,pump_motor_printed_speed_val_);
        break;

        case DEBRIDER_STATE_EMERGENCY:
            SetDefaultUI();
            StopPumpMotor();
            DisableValve();
            PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
            digitalWrite(VDD_RESET,LOW);
            delay(10);
            digitalWrite(VDD_RESET,HIGH);
            delay(10);
            on_CallEmergencyWindow();
        break;

        case DEBRIDER_STATE_SPI_ERROR:
            SetDefaultUI();
            StopPumpMotor();
            DisableValve();
            PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
            emit InitStateChanged(DEBRIDER_STATE_SPI_ERROR);
        break;

        case DEBRIDER_STATE_EPOS_ERROR:
            SetDefaultUI();
            StopPumpMotor();
            PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
            emit InitStateChanged(DEBRIDER_STATE_EPOS_ERROR);
            break;

        case DEBRIDER_STATE_INITIALIZING:
            SetDefaultUI();
            StopPumpMotor();
            PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
            emit InitStateChanged(DEBRIDER_STATE_INITIALIZING);
            break;

        default:
             EnableGUI();
             PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
    }
}

void MainWindow::SetDefaultUI()
{
    DisableGUI();
    ui->radioMAXRPM->setChecked(false);
    ui->radioCW->setChecked(true);
    on_radioCW_toggled(true);
    motor_thread_.m_DebriderTargetSpeed=0;
    debrider_motor_target_speed_=0;
    pump_motor_target_speed_=0;
    pump_motor_printed_speed_val_=0;
}

void MainWindow::ShowControlUI()
{
    if(!this->isVisible()){
        this->setWindowState(Qt::WindowFullScreen);
        this->show();
        init_window_.hide();
    }
}
void MainWindow::on_btnDecreaseRPM_clicked()
{
    ui->radioMAXRPM->setChecked(false);
    debrider_motor_target_speed_ -= CHANGE_RPM_RATE;
    QString qstr;
    if(debrider_motor_target_speed_ < 0)
    {
        debrider_motor_target_speed_ = 0;
        ui->lblStatusMsg->setText(QString("error : cannot decrease RPM\n"
                                          "RPM cannot be less then 0 "));
    }
    else
        PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);

    if(ui->radioCCW->isChecked())
        motor_thread_.m_DebriderTargetSpeed = debrider_motor_target_speed_;
    else
        motor_thread_.m_DebriderTargetSpeed = -debrider_motor_target_speed_;
}

void MainWindow::on_btnIncreaseRPM_clicked()
{
    debrider_motor_target_speed_ += CHANGE_RPM_RATE;

    if(debrider_motor_target_speed_ > BLDC_MAX_RPM)
    {
        debrider_motor_target_speed_ = BLDC_MAX_RPM;
        ui->lblStatusMsg->setText(QString("error : cannot increase more than MAX RPM\n"
                                          "Max RPM for Debrider : 12000 RPM"));
    }
    else
        PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);

    if(debrider_motor_target_speed_==BLDC_MAX_RPM)
        ui->radioMAXRPM->setChecked(true);

    if(ui->radioCCW->isChecked())
      motor_thread_.m_DebriderTargetSpeed = debrider_motor_target_speed_;
    else
      motor_thread_.m_DebriderTargetSpeed = -debrider_motor_target_speed_;
}

void MainWindow::on_radioCW_toggled(bool checked)
{
    if(checked) 
    {
        motor_thread_.m_DebriderTargetSpeed = -debrider_motor_target_speed_;
        motor_thread_.m_Oscillate = 0;
    }
}

void MainWindow::on_radioCCW_toggled(bool checked)
{
   if(checked)
    {
       motor_thread_.m_DebriderTargetSpeed = debrider_motor_target_speed_;
       motor_thread_.m_Oscillate = 0;
    }
}

void MainWindow::on_radioOSC_toggled(bool checked)
{
    if(checked)
    {
        motor_thread_.m_Oscillate = 1;
    }
}

void MainWindow::on_btnDecreaseFlow_clicked()
{
    // CKim - Pump speed updated here, perhaps pwm command should be issued in main thread instead of here??
    if(pump_motor_target_speed_ >= 120)    pump_motor_target_speed_-=40;
    if(pump_running_status_) pwmWrite(SUCTION_MOTOR_PWM,pump_motor_target_speed_);
    pump_motor_printed_speed_val_=int(((pump_motor_target_speed_-120.0)/4.0)+10.0);
    PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
}

void MainWindow::on_btnIncreaseFlow_clicked()
{
    if(pump_motor_printed_speed_val_==0) pump_motor_target_speed_=80;
    if(pump_motor_target_speed_ < PUMP_PWM_RANGE)    pump_motor_target_speed_+=40;
    if(pump_running_status_) pwmWrite(SUCTION_MOTOR_PWM,pump_motor_target_speed_);
    pump_motor_printed_speed_val_=int(((pump_motor_target_speed_-120.0)/4.0)+10);
    PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
}

void MainWindow::on_btnIrrigationMove_clicked()
{
    MovePumpMotor();
    PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
}

void MainWindow::on_btnIrrigationStop_clicked()
{
    StopPumpMotor();
    PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
}

void MainWindow::on_btnCloseBlade_clicked()
{
   motor_thread_.m_GuiBtnCloseBlade=1;
}

void MainWindow::on_radioMAXRPM_clicked()
{
    debrider_motor_target_speed_ = BLDC_MAX_RPM;
    PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
    if(ui->radioCCW->isChecked())
      motor_thread_.m_DebriderTargetSpeed = debrider_motor_target_speed_;
    else
      motor_thread_.m_DebriderTargetSpeed = -debrider_motor_target_speed_;
}

void MainWindow::StopPumpMotor()
{
    pump_running_status_=false;
    pwmWrite(SUCTION_MOTOR_PWM,0);
}

void MainWindow::MovePumpMotor()
{
    int pumpSpeed;
    pumpSpeed=int(pump_motor_target_speed_);
    pump_running_status_=true;
    pwmWrite(SUCTION_MOTOR_PWM,pumpSpeed);
}

void MainWindow::PrintStatus(int dSpeed, int pSpeed)
{
    speed_label_.sprintf(" %d ", debrider_motor_target_speed_);
    ui->p_BLDCspeedInfo->setText(speed_label_);
    speed_label_.sprintf(" %d ",pSpeed);
    ui->p_MotorSpeedInfo->setText(speed_label_);
    if(pump_running_status_==false)
    status_label_.sprintf(" Debrider Motor Set :  %d RPM \n Pump Motor Set : %% %d  ",dSpeed,pSpeed);
    else
    status_label_.sprintf(" Debrider Motor Set:  %d RPM \n Pump Motor Running at : %% %d  ",dSpeed,pSpeed);
    if(motor_thread_.m_DebriderInstantSpeed < 20 && pump_running_status_==true)
    {
        status_label_.sprintf(" Debrider Motor Running at CW MODE at :  %d RPM \n "
                            "Pump Motor Running at : %% %d  ",motor_thread_.m_DebriderInstantSpeed*(-1),pSpeed);
    }
    else if(motor_thread_.m_DebriderInstantSpeed > 20 && pump_running_status_==true)
    {
        status_label_.sprintf(" Debrider Motor Running at CCW MODE at :  %d RPM \n "
                            "Pump Motor Running at : %% %d  ",motor_thread_.m_DebriderInstantSpeed,pSpeed);
    }
    ui->lblStatusMsg->setText(status_label_);
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
    int debriderMotorSetSpeed=debrider_motor_target_speed_;
    if(motor_thread_.m_GuiChangePresetRPM){
        EnableBuzzer();
        if((debrider_motor_target_speed_ < 1000)  || debrider_motor_target_speed_==BLDC_MAX_RPM) debriderMotorSetSpeed=1000;
        if((debrider_motor_target_speed_ >= 1000) && debrider_motor_target_speed_ < 3000) debriderMotorSetSpeed=3000;
        if((debrider_motor_target_speed_ >= 3000) && debrider_motor_target_speed_ < 5000) debriderMotorSetSpeed=5000;
        if((debrider_motor_target_speed_ >= 5000) && debrider_motor_target_speed_ < 7000) debriderMotorSetSpeed=7000;
        if((debrider_motor_target_speed_ >= 7000) && debrider_motor_target_speed_ < 10000) debriderMotorSetSpeed=10000;
        if((debrider_motor_target_speed_ >= 10000) && debrider_motor_target_speed_ < 12000) debriderMotorSetSpeed=BLDC_MAX_RPM;
        debrider_motor_target_speed_=debriderMotorSetSpeed;
        if(debrider_motor_target_speed_!=BLDC_MAX_RPM){ ui->radioMAXRPM->setChecked(false);  }
        else                               { ui->radioMAXRPM->setChecked(true);   }

        motor_thread_.m_GuiChangePresetRPM=0;
        if(ui->radioCCW->isChecked())
          motor_thread_.m_DebriderTargetSpeed = debrider_motor_target_speed_;
        else if(ui->radioCW->isChecked())
          motor_thread_.m_DebriderTargetSpeed = -debrider_motor_target_speed_;
        else motor_thread_.m_DebriderTargetSpeed = debrider_motor_target_speed_;
        PrintStatus(debrider_motor_target_speed_,pump_motor_printed_speed_val_);
    }
   // #########  HARDWARE MAXRPM BUTTON CLICKED SETTINGS FINISH   #########

    // #########  HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS START   #########
    if(motor_thread_.m_GuiBtnChangeDirection)
    {
        EnableBuzzer();
        if(ui->radioCW->isChecked())
        {
            ui->radioCCW->setChecked(true);
            on_radioCCW_toggled(true);
            motor_thread_.m_GuiBtnChangeDirection=0;
        }
        else if(ui->radioCCW->isChecked())
        {
            ui->radioOSC->setChecked(true);
            on_radioOSC_toggled(true);
            motor_thread_.m_GuiBtnChangeDirection=0;
        }
        else
        {
            ui->radioCW->setChecked(true);
            on_radioCW_toggled(true);
            motor_thread_.m_GuiBtnChangeDirection=0;
        }
    }
}
// #########  HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS FINISH   #########

void MainWindow::on_CallEmergencyWindow()
{
    motor_thread_.m_GuiEmergencyMode=1;
    std::cout << "Emergency window called\n";
    if(!emergency_window_.isVisible()){
        emergency_window_.SetEmergencyText();
        emergency_window_.setWindowState(Qt::WindowFullScreen);
        emergency_window_.show();
        emergency_window_.setWindowTitle("Emergency Window");
        if(init_window_.isVisible()) init_window_.close();
        if(this->isVisible()) this->close();
    }
}

void MainWindow::on_CallInitWindow()
{
    std::cout << "Init windows called." << std::endl;
    init_window_.setWindowState(Qt::WindowFullScreen);
    init_window_.show();
    init_window_.setWindowTitle("Flexdeb Initialization");
    if(this->isVisible()) this->hide();
    if(emergency_window_.isVisible()) emergency_window_.hide();
}

void MainWindow::EnableValve()
{
    digitalWrite(SOLENOID_VALVE_ENABLE,HIGH);
}

void MainWindow::DisableValve()
{
    digitalWrite(SOLENOID_VALVE_ENABLE,LOW);
}

void MainWindow::EnableBuzzer()
{
    if(!buzzer_running_status){
        softToneWrite(PEDAL_BUZZER,1000);
        delay(200);
        softToneWrite(PEDAL_BUZZER,0);
        buzzer_running_status = true;
    }

}

void MainWindow::InitializeIO()
{
    // CKim - Initializing Variables
    debrider_motor_target_speed_ = 0;
    pump_motor_target_speed_ = 0;
    pump_running_status_=false;

    /// VysAdn, instead of WiringPi pin numbering this will use GPIO pin numbering.
    wiringPiSetupGpio();

    /// SGN signal, output of watchdog and emergency button.
    pinMode(EMERGENCY_RELAY_CONTROL,INPUT);

    /// Watchdog pin declaration
    pinMode(WATCHDOG_PIN,OUTPUT);

    /// VysAdn wiringpi softpwm init. softPwmCreate(int pin_number,int initial_val,int pwm_range)
    softToneCreate(PEDAL_BUZZER);
    /// VysAdn pedal button pin declarations.
    pinMode(FOOT_PEDAL_L_BUTTON,INPUT);
    pinMode(FOOT_PEDAL_R_BUTTON,INPUT);
    pinMode(FOOT_PEDAL_L_PEDAL_BUTTON,INPUT);
    pinMode(FOOT_PEDAL_R_PEDAL_BUTTON,INPUT);

    /// Watchdog power reset pin declaration.
    pinMode(VDD_RESET, OUTPUT);

    /// VysAdn operating signal, for init window.
    pinMode(INITIALIZATION_SWTICH,INPUT);

    /// Suction motor output declaration.
    pinMode(SUCTION_MOTOR_PWM,PWM_OUTPUT);

    /// Solenoid valve output declaration.
    pinMode(SOLENOID_VALVE_ENABLE,OUTPUT);

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

    digitalWrite(VDD_RESET,LOW);
    delay(10);
    digitalWrite(VDD_RESET,HIGH);
    delay(10);
}
