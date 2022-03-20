/**
 ***************************************************************************
 * \file  initialization_window.h
 * \brief Header file includes initialization window class, checks if system
 * is ready for running, by checking pedal connection, motor connection and
 * waiting for operator to press initialization switch.
 * In case system is not ready operator have to click retry initialization
 * button on GUI.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *****************************************************************************
**/


#ifndef INITIALIZATION_WINDOW_H
#define INITIALIZATION_WINDOW_H

#include <QMainWindow>

namespace Ui {
class InitializationWindow;
}

class InitializationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InitializationWindow(QWidget *parent = nullptr);
    ~InitializationWindow();

public slots:
    /**
     * @brief UpdateInitGUI Update initialization window based on current state.
     * It gives feedback to user about system state, e.g. system ready, handpiece is connected or not
     * pedal is connected or not.
     * @param state Current state of debrider.
     */
    void UpdateInitGUI(int state);

public slots:
    /**
     * @brief on_btn_reinit_clicked In case user clicks reinitialization, system will try to reinitialize
     * communications with motor and pedal.
     */
    void on_btn_reinit_clicked();

signals:
    /**
     * @brief btn_reinit_clicked This signal is emitted when user clicks reinitialization button to notify
     * other threads about reinitialization.
     * @param state Current state of debrider.
     */
    void btn_reinit_clicked(int state);

private:
    Ui::InitializationWindow *ui;
    /// Current state flag of debrider.
    int cur_state;
};

#endif // INITIALIZATION_WINDOW_H
