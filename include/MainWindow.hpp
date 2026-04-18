#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QInputDialog>
#include <QMessageBox>
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

private slots:
    void onResetClicked();
    void onDoneClicked();
    void onDrawTileClicked();
    void onSpawnTileClicked();

private:
    void drawBoard();
    void startNextTurn();
    void processAITurn();

    void setupGame(); // Replaces the setup part of your old main()

    // --- GUI Components ---
    QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsTextItem* aiStatusText;
    QPushButton* resetBtn;
    QPushButton* doneBtn;
    QPushButton* drawTileBtn;
    QPushButton* spawnTileBtn;
    
    Board initialBoard;
    std::vector<Tile> initialHand;
    int currentPlayerIndex;

    // --- Game State ---
    Board board;
    TilesBag bag;
    std::vector<std::unique_ptr<Player>> players;
    int timeLimit;
};

#endif
