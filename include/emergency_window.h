/**
 ***************************************************************************
 * \file  emergency_window.h
 * \brief Header file includes emergency window class which includes
 * emergency messages, in case of any error occurence in the software,
 * such as motor driver error, SPI communication error, watchdog error.
 * Additionally clicking emergency button will wake this window as well.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 ****************************************************************************
**/

#ifndef EMERGENCY_WINDOW_H
#define EMERGENCY_WINDOW_H

#include <QMainWindow>
#include "m_defines.h"

namespace Ui {
class EmergencyWindow;
}

class EmergencyWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EmergencyWindow(QWidget *parent = nullptr);
    ~EmergencyWindow();
    /**
     * @brief Sets emergency text in Emergency Windows based on
     * current emergency state of debrider.
     */
    void SetEmergencyText();
    /**
     * @brief Gets emergency status of system.
     * @return emergency status.
     */
    int  GetEmergencyStatus();
    /**
     * @brief Sets emergency status of the system.
     * @param status Emergency status flag.
     */
    void SetEmergencyStatus(int status);

private slots:
    /**
     * @brief Gui emergency exit button clicked function callback. It will emit EmergencyExitClicked signal to notify MainWindow.
     */
    void on_ButtonEmergencyExit_clicked();

signals:
    /**
     * @brief Signal will be emitted to notify MainWindow, that emergency exit clicked.
     * @param s emergency state.
     */
    void EmergencyExitClicked(int s);

private:
    Ui::EmergencyWindow *ui;
    /// Flag for emergency status of the system.
    int emergency_status_;
};

#endif // EMERGENCY_WINDOW_H
