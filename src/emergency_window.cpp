#include "include/emergency_window.h"
#include "ui_emergencywindow.h"

EmergencyWindow::EmergencyWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EmergencyWindow)
{
    ui->setupUi(this);
    emergency_status_=0;
}

EmergencyWindow::~EmergencyWindow()
{
    delete ui;
}

void EmergencyWindow::on_ButtonEmergencyExit_clicked()
{
//    EmergencyWindow::close();
    emit EmergencyExitClicked(1);
}

void EmergencyWindow::SetEmergencyText()
{
    QString txtEmergency;
    if(emergency_status_==DEBRIDER_STATE_EMERGENCY)
    {
        txtEmergency=(" \n \t ! WARNING ! \n \n !EMERGENCY MODE ACTIVE! \n \n In case of : \n"
                      " Emergency Switch or Initialization Switch \n is pressed\n "
                      "release the switches and click \n 'EXIT' to restart the controller");
    }
    else if (emergency_status_==DEBRIDER_STATE_SPI_ERROR)
    {
        txtEmergency=("\n \t! EMERGENCY MODE ACTIVE ! \n \n \t SPI CONNECTION ERROR \n \t"
                      "CLICK 'EXIT' TO TRY TO RECONNECT ");
    }
    else if (emergency_status_==DEBRIDER_STATE_EPOS_ERROR)
    {
        txtEmergency=("\n \t! EMERGENCY MODE ACTIVE ! \n \n \t EPOS CONNECTION ERROR \n \t "
                      "CLICK 'EXIT' TO TRY TO RECONNECT");
    }
    else
    {
        txtEmergency=(" \n !EMERGENCY MODE ACTIVE!");
    }
    ui->lblEmergencyTxt->setText(txtEmergency);
}

void EmergencyWindow::SetEmergencyStatus(int status)
{
    emergency_status_ = status;
}

int EmergencyWindow::GetEmergencyStatus()
{
    return  emergency_status_ ;
}
