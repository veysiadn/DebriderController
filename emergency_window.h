#ifndef EMERGENCY_WINDOW_H
#define EMERGENCY_WINDOW_H

#include <QDialog>
namespace Ui {
class Emergency_Window;
}

class Emergency_Window : public QDialog
{
    Q_OBJECT

public:
    explicit Emergency_Window(QWidget *parent = nullptr);
    ~Emergency_Window();
    int m_secEmergency;
    int m_EmergencyStatus;
    void SetEmergencyText();
private slots:
    void on_btnEmergencyExit_clicked();

private:
    Ui::Emergency_Window *ui;
};

#endif // EMERGENCY_WINDOW_H
