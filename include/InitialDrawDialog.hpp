#ifndef INITIAL_DRAW_DIALOG_HPP
#define INITIAL_DRAW_DIALOG_HPP

#include <QDialog>
#include <QPushButton>
#include <vector>
#include <array>

#include "TileMap.hpp" 
#include "GameTypes.hpp"
#include "Constants.hpp"

class InitialDrawDialog : public QDialog {
    Q_OBJECT

public:
    explicit InitialDrawDialog(QWidget *parent = nullptr);
    std::vector<Tile> getDrawnTiles() const;

private slots:
    void onTileClicked();
    void onFinishClicked();

private:
    void setupUi();
    void updateButtonVisuals(QPushButton* btn, int count);
    // Uses GenericTileMap to store how many of each tile are selected (0, 1, or 2)
    TileMap selectionCounts; 
    // An array to store pointers to the buttons, indexed the same way as the TileMap
    std::array<QPushButton*, N_DIFF_TILES> tileButtons;

    int totalSelected = 0;
    QPushButton* finishButton;
};

#endif
