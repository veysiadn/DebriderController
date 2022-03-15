/*****************************************************************************
 * \file  initialization_window.h
 * \brief Header file includes initialization window class, checks if system
 * is ready for running, by checking pedal connection, motor connection and
 * waiting for operator to press initialization switch.
 * In case system is not ready operator have to click retry initialization
 * button on GUI.
 *
 * Last Updated : 2022.03.15 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *******************************************************************************/


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
    void UpdateInitGUI(int state);
public slots:
    void on_btn_reinit_clicked();

signals:
    void btn_reinit_clicked(int state);
private:
    Ui::InitializationWindow *ui;
    int cur_state;
};

#endif // INITIALIZATION_WINDOW_H
