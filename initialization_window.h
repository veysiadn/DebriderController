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
