#include "emergency_window.h"
#include "ui_emergency_window.h"

Emergency_Window::Emergency_Window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Emergency_Window)
{
    ui->setupUi(this);
    emergency_status=0;
}

Emergency_Window::~Emergency_Window()
{
    delete ui;
}

void Emergency_Window::on_ButtonEmergencyExit_clicked()
{
    Emergency_Window::close();
    emit EmergencyExitClicked(0);
}
void Emergency_Window::SetEmergencyText()
{
    QString txtEmergency;
    if(emergency_status==DEBRIDER_STATE_EMERGENCY)
    {
        txtEmergency=(" \n \t ! WARNING ! \n \n !EMERGENCY MODE ACTIVE! \n \n If emergency switch is pressed\n "
                      "release the switch and click \n 'EXIT' to restart the controller");
    }
    else if (emergency_status==DEBRIDER_STATE_SPI_ERROR)
    {
        txtEmergency=("\n \t! EMERGENCY MODE ACTIVE ! \n \n \t SPI CONNECTION ERROR \n \t"
                      "CLICK 'EXIT' TO TRY TO RECONNECT ");
    }
    else if (emergency_status==DEBRIDER_STATE_EPOS_ERROR)
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
