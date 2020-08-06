#include "emergency_window.h"
#include "ui_emergency_window.h"
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
