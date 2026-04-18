#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <qwidget.h>
#include <vector>
#include <memory>

#include "GameTypes.hpp"
#include "Player.hpp"
#include "TilesBag.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    void setupGame(); // Replaces the setup part of your old main()

    // --- GUI Components ---
    QGraphicsView* view;
    QGraphicsScene* scene;

    // --- Game State ---
    Board board;
    TilesBag bag;
    std::vector<std::unique_ptr<Player>> players;
    int timeLimit;
};

#endif
