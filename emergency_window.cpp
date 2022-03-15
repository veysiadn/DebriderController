#include "emergency_window.h"
#include "ui_emergency_window.h"

EmergencyWindow::EmergencyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Emergency_Window)
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
    emit EmergencyExitClicked(0);
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
