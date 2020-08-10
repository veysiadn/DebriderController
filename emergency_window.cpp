#include "emergency_window.h"
#include "ui_emergency_window.h"
#include "m_defines.h"
Emergency_Window::Emergency_Window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Emergency_Window)
{
    ui->setupUi(this);
}

Emergency_Window::~Emergency_Window()
{
    delete ui;
}

void Emergency_Window::on_btnEmergencyExit_clicked()
{
    Emergency_Window::close();
    m_secEmergency=0;
    emit Emergency_Exit_Clicked(0);
}
void Emergency_Window::SetEmergencyText()
{
    QString txtEmergency;
    if(m_EmergencyStatus==DEBRIDER_STATE_EMERGENCY)
    {
        txtEmergency=(" \n \t !EMERGENCY MODE ACTIVE!");
    }
    else if (m_EmergencyStatus==DEBRIDER_STATE_SERIAL_ERROR)
    {
        txtEmergency=("\n \t! EMERGENCY MODE ACTIVE ! \n \n \t SERIAL CONNECTION ERROR");
    }
    else if (m_EmergencyStatus==DEBRIDER_STATE_EPOS_ERROR)
    {
        txtEmergency=("\n \t! EMERGENCY MODE ACTIVE ! \n \n \t EPOS CONNECTION ERROR");
    }
    else
    {
        txtEmergency=(" \n \t !EMERGENCY MODE ACTIVE!");
    }
    ui->lblEmergencyTxt->setText(txtEmergency);
}
