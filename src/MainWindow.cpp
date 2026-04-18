#include "MainWindow.hpp"
#include "InitialDrawDialog.hpp"
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QString>
#include <string>
#include <QApplication>

#include "Constants.hpp"
#include "Utils.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  scene = new QGraphicsScene(this);
  view = new QGraphicsView(scene, this);

  setCentralWidget(view);
  resize(1024, 768);
  setWindowTitle("Rummikub");

  // 2. Start the game setup process
  setupGame();
}

void MainWindow::setupGame() {
  bool ok;

  // Get number of players
  int nPlayers = QInputDialog::getInt(this, "Setup", "How many players are playing?", 2, 2, 4, 1, &ok);
  if (!ok) {
    std::exit(0);
    return;
  }

  // Get player names
  for (int i = 0; i < nPlayers; ++i) {
    std::string rawString = "Enter name for player " + std::to_string(i) + "\n(Player 0 is AI):";
    QString prompt = QString::fromStdString(rawString);
    QString qName = QInputDialog::getText(this, "Player Name", prompt, QLineEdit::Normal, "", &ok);

    // Handle empty names or cancellations
    std::string name;

    if (!ok) {
      std::exit(0);
      return;
    }

    if (!qName.trimmed().isEmpty()) {
      name = qName.toStdString();
    }
    else {
      QMessageBox::warning(this, "Invalid Input", "Invalid Name, try again.");
      --i;
      continue;
    }

    if (i == 0) {
      players.emplace_back(std::make_unique<AIPlayer>(name));
    } 
    else {
      players.emplace_back(std::make_unique<HumanPlayer>(name));
    }
  }

  // Get time limit
  timeLimit = QInputDialog::getInt(this, "Time Limit", "Wie lang magsch warte? (in seconds):", 30, 1, 60, 1, &ok);
  if (!ok) timeLimit = 20; // Default fallback

  QString infoStr = QString("Values go from %1 to %2.\n\n").arg(MIN_TILE_VALUE).arg(MAX_TILE_VALUE);
  infoStr += "Colors are: ";
  for (const Color& c : ALL_COLORS) {
    infoStr += QString::fromStdString(color_to_str(c)) + " ";
  }
  infoStr += "\n\nThere is NO case-sensitivity for user input of type string.";

  QMessageBox::information(this, "Game Rules", infoStr);

  InitialDrawDialog aiDiag(this);
  if (aiDiag.exec() == QDialog::Accepted) {
    std::vector<Tile> drawnTiles = aiDiag.getDrawnTiles();
    static_cast<AIPlayer*>(players[0].get())->set_hand(drawnTiles);
    for (const Tile& t : drawnTiles) {
      bag.draw(); 
    }
  } else {
    std::exit(0);
  }

  // Usually, humans just draw random tiles from the bag
  for (size_t i = 1; i < players.size(); ++i) {
    QString drawString = QString("%1, draw your tiles.").arg(players[i]->get_name());
    QMessageBox::information(this, "Drawing Tiles", drawString);
    players[i]->inital_draw(bag); 
  }

  QMessageBox::information(this, "Setup Complete", "All players have drawn!");

  // drawBoard();
  // startNextTurn(0);

}
