#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.resize(1280, 720);
    window.setWindowTitle("Rummikub");
    window.show();

    return app.exec();
}
