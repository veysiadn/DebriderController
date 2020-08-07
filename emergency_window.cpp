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
}

void Emergency_Window::SetEmergencyText()
{
    QString txtEmergency;
    if(m_EmergencyStatus==DEBRIDER_STATE_EMERGENCY)
    {
        txtEmergency=("EMERGENCY MODE ACTIVE");
    }
    else if (m_EmergencyStatus==DEBRIDER_STATE_SERIAL_ERROR)
    {
        txtEmergency=("EMERGENCY MODE ACTIVE : SERIAL ERROR");
    }
    else if (m_EmergencyStatus==DEBRIDER_STATE_EPOS_ERROR)
    {
        txtEmergency=("EMERGENCY MODE ACTIVE : EPOS CONNECTION ERROR");
    }
    else
    {
        txtEmergency=("EMERGENCY MODE ACTIVE");
    }
    ui->lblEmergencyTxt->setText(txtEmergency);
}
