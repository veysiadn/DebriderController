#include "include/initialization_window.h"
#include "ui_initialization_window.h"
#include "include/m_defines.h"

InitializationWindow::InitializationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InitializationWindow)
{
    ui->setupUi(this);
}

InitializationWindow::~InitializationWindow()
{
    delete ui;
}

void InitializationWindow::UpdateInitGUI(int state)
{
    switch (state) {
        case DEBRIDER_STATE_UNINIT:
            cur_state = DEBRIDER_STATE_UNINIT;
            ui->lbl_handpiece->setText("Waiting...");
            ui->lbl_pedal_con->setText("Waiting...");
            ui->lbl_system_status->setText("Waiting...");
            ui->lbl_init_switch->setText("Checking...");
            break;
        case DEBRIDER_STATE_INIT:
            cur_state = DEBRIDER_STATE_INIT;
            break;
        case DEBRIDER_STATE_READY:
            ui->lbl_handpiece->setText("Connected");
            ui->lbl_pedal_con->setText("Connected");
            ui->lbl_system_status->setText("Ready");
            ui->lbl_init_switch->setText("System Ready, press Initialization switch to start.");
            cur_state = DEBRIDER_STATE_READY;
            break;
        case DEBRIDER_STATE_EPOS_ERROR:
             ui->lbl_handpiece->setText("Not Connected");
             ui->lbl_system_status->setText("Missing Connection");
             cur_state = DEBRIDER_STATE_EPOS_ERROR;
            break;

        case DEBRIDER_STATE_SPI_ERROR:
                ui->lbl_pedal_con->setText("Not Connected");
                ui->lbl_system_status->setText("Missing Connection");
                cur_state = DEBRIDER_STATE_SPI_ERROR;
            break;
        case DEBRIDER_STATE_INITIALIZING:
            cur_state=DEBRIDER_STATE_INITIALIZING ;
            ui->lbl_handpiece->setText("Waiting...");
            ui->lbl_pedal_con->setText("Waiting...");
            ui->lbl_system_status->setText("Waiting...");
            ui->lbl_init_switch->setText("If initialization switch is pressed, release it\n and click \"Retry Initialization\" button.");
            break;
        default:
        ui->lbl_handpiece->setText("...");
        ui->lbl_pedal_con->setText("...");
        ui->lbl_system_status->setText("...");
        ui->lbl_init_switch->setText("...");

    }
}

void InitializationWindow::on_btn_reinit_clicked()
{
    emit(btn_reinit_clicked(cur_state));
    ui->lbl_handpiece->setText("Retrying...");
    ui->lbl_pedal_con->setText("Retrying...");
    ui->lbl_system_status->setText("Retrying...");
    ui->lbl_init_switch->setText("...");
}
