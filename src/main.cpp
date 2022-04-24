#include <include/mainwindow.h>
#include <QApplication>
#include <sys/mman.h>
int main(int argc, char *argv[])
{
    setvbuf(stdout,nullptr,_IONBF,BUFSIZ);
    QApplication a(argc, argv);
    MainWindow w;
//    w.setWindowTitle("Medicaretec Debrider");
//    w.setWindowState(Qt::WindowFullScreen);
//    w.show();

    return a.exec();
}
