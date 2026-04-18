#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Create and show the main game window
    MainWindow window;
    window.resize(1280, 720);
    window.setWindowTitle("Rummikub");
    window.show();

    return app.exec();
}
