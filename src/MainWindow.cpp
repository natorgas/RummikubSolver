#include <sstream>
#include <cmath>
#include <algorithm>
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentPlayerIndex(-1) {
  QWidget* central = new QWidget(this);
  QVBoxLayout* mainLayout = new QVBoxLayout(central);

  scene = new QGraphicsScene(this);
  aiStatusText = new QGraphicsTextItem();
  aiStatusText->setDefaultTextColor(Qt::white);
  QFont f = aiStatusText->font();
  f.setPointSize(16);
  aiStatusText->setFont(f);
  scene->addItem(aiStatusText);
  view = new QGraphicsView(scene, this);
  mainLayout->addWidget(view);

  QHBoxLayout* btnLayout = new QHBoxLayout();
  resetBtn = new QPushButton("Reset", this);
  doneBtn = new QPushButton("Done", this);
  drawTileBtn = new QPushButton("Draw Tile", this);
  spawnTileBtn = new QPushButton("Spawn Hand Tile", this);

  btnLayout->addWidget(resetBtn);
  btnLayout->addWidget(doneBtn);
  btnLayout->addWidget(drawTileBtn);
  btnLayout->addWidget(spawnTileBtn);
  mainLayout->addLayout(btnLayout);

  setCentralWidget(central);
  resize(1024, 768);
  setWindowTitle("Rummikub");

  connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onResetClicked);
  connect(doneBtn, &QPushButton::clicked, this, &MainWindow::onDoneClicked);
  connect(drawTileBtn, &QPushButton::clicked, this, &MainWindow::onDrawTileClicked);
  connect(spawnTileBtn, &QPushButton::clicked, this, &MainWindow::onSpawnTileClicked);

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
  } 
  else {
    std::exit(0);
  }

  for (size_t i = 1; i < players.size(); ++i) {
    QString drawString = QString("%1, draw your tiles.").arg(players[i]->get_name());
    QMessageBox::information(this, "Drawing Tiles", drawString);
    players[i]->inital_draw(bag); 
  }

  QMessageBox::information(this, "Setup Complete", "All players have drawn!");

  currentPlayerIndex = players.size() - 1;
  startNextTurn();

}


class TileItem : public QGraphicsRectItem {
  public:
    Tile tile;
    TileItem(Tile t) : QGraphicsRectItem(0, 0, 40, 60), tile(t) {
      setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
      setBrush(Qt::white);
      setPen(QPen(Qt::black));

      QGraphicsTextItem* text = new QGraphicsTextItem(QString::number(t.value), this);
      QFont font = text->font();
      font.setBold(true);
      font.setPointSize(14); // Make it slightly larger for better visibility
      text->setFont(font);
      text->setPos(5, 5);
      if (t.isJoker) {
        text->setPlainText("J");
        text->setDefaultTextColor(Qt::black);
      } else {
        if (t.color == Color::Black) text->setDefaultTextColor(Qt::black);
        else if (t.color == Color::Blue) text->setDefaultTextColor(Qt::blue);
        else if (t.color == Color::Red) text->setDefaultTextColor(Qt::red);
        else if (t.color == Color::Orange) text->setDefaultTextColor(QColor(255, 165, 0));
      }
    }
};

void MainWindow::drawBoard() {
  if (aiStatusText && aiStatusText->scene() == scene) {
      scene->removeItem(aiStatusText);
  }
  scene->clear();
  if (aiStatusText) {
      scene->addItem(aiStatusText);
      aiStatusText->setPos(800, 50);
      aiStatusText->setZValue(100);
  }
  int yOffset = 20;

  auto drawSets = [&](const std::vector<Set>& sets, const QString& title) {
    QGraphicsTextItem* titleItem = scene->addText(title);
    titleItem->setPos(10, yOffset);
    yOffset += 30;

    for (const Set& s : sets) {
      int xOffset = 20;
      for (const Tile& t : s.tiles) {
        TileItem* item = new TileItem(t);
        item->setPos(xOffset, yOffset);
        scene->addItem(item);
        xOffset += 45;
      }
      yOffset += 70;
    }
  };

  drawSets(board.runs, "Runs:");
  drawSets(board.groups, "Groups:");
  scene->setSceneRect(0, 0, 1000, std::max(800, yOffset + 100));
}

void MainWindow::onResetClicked() {
  board = initialBoard;
  if (currentPlayerIndex >= 0 && currentPlayerIndex < players.size()) {
      players[currentPlayerIndex]->set_hand(initialHand);
  }
  drawBoard();
}

void MainWindow::onDoneClicked() {
  if (dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
      QMessageBox::information(this, "AI Turn", "AI manages its own turns.");
      return;
  }

  std::vector<TileItem*> items;
  for (QGraphicsItem* item : scene->items()) {
    if (TileItem* ti = dynamic_cast<TileItem*>(item)) {
      items.push_back(ti);
    }
  }

  std::sort(items.begin(), items.end(), [](TileItem* a, TileItem* b){
      if (std::abs(a->y() - b->y()) > 30) return a->y() < b->y();
      return a->x() < b->x();
  });

  Board newBoard;
  std::vector<Tile> currentSet;
  bool valid = true;
  
  auto addSet = [&]() {
    if (!currentSet.empty()) {
      // Find non-jokers to infer the set type
      std::vector<std::pair<int, Tile*>> nonJokers;
      for (size_t i = 0; i < currentSet.size(); ++i) {
          if (!currentSet[i].isJoker) {
              nonJokers.push_back({i, &currentSet[i]});
          }
      }

      bool isGroup = true;
      if (nonJokers.size() >= 2) {
          if (nonJokers[0].second->value != nonJokers[1].second->value) {
              isGroup = false;
          }
      } else if (nonJokers.size() == 1) {
          // Only 1 regular tile, rest are jokers. Could be either.
          // Let's assume group if it's placed like a group? Hard to know.
          // We will just try Group first, if it fails logically, fallback to Run.
          // Actually, if we just look at the board, we can assume Run if we can build it.
          // Let's assume Run if we can mathematically make it a Run (values fit in 1-13).
          int refIdx = nonJokers[0].first;
          int refVal = nonJokers[0].second->value;
          int startVal = refVal - refIdx;
          int endVal = startVal + currentSet.size() - 1;
          if (startVal >= 1 && endVal <= 13) {
              isGroup = false; // It fits as a run!
          } else {
              isGroup = true;
          }
      } else {
          // All Jokers?!
          isGroup = true;
      }

      // Infer Joker values/colors
      if (isGroup) {
          int groupVal = nonJokers.empty() ? 1 : nonJokers[0].second->value;
          std::vector<Color> available = {Color::Black, Color::Blue, Color::Red, Color::Orange};
          for (auto nj : nonJokers) {
              auto it = std::find(available.begin(), available.end(), nj.second->color);
              if (it != available.end()) available.erase(it);
          }
          for (size_t i = 0; i < currentSet.size(); ++i) {
              if (currentSet[i].isJoker) {
                  currentSet[i].value = groupVal;
                  if (!available.empty()) {
                      currentSet[i].color = available.back();
                      available.pop_back();
                  } else {
                      currentSet[i].color = Color::None; // Invalid anyway
                  }
              }
          }
      } else {
          // Run
          Color runColor = nonJokers.empty() ? Color::Black : nonJokers[0].second->color;
          int startVal = nonJokers.empty() ? 1 : nonJokers[0].second->value - nonJokers[0].first;
          for (size_t i = 0; i < currentSet.size(); ++i) {
              if (currentSet[i].isJoker) {
                  currentSet[i].value = startVal + i;
                  currentSet[i].color = runColor;
              }
          }
      }

      Set s(isGroup ? SetType::Group : SetType::Run, currentSet);
      // Push directly to not lose tiles if invalid
      if (isGroup) newBoard.groups.push_back(s);
      else newBoard.runs.push_back(s);
    }
    currentSet.clear();
  };

  TileItem* prev = nullptr;
  for (TileItem* item : items) {
    if (prev) {
      if (std::abs(item->y() - prev->y()) > 30 || item->x() - prev->x() > 60) {
        addSet();
      }
    }
    currentSet.push_back(item->tile);
    prev = item;
  }
  addSet();

  for (Set& s : newBoard.runs) if (!s.valid()) valid = false;
  for (Set& s : newBoard.groups) if (!s.valid()) valid = false;

  std::vector<Tile> oldTiles = initialBoard.tiles_on_board();
  std::vector<Tile> newTiles = newBoard.tiles_on_board();
  normalize_jokers(oldTiles);
  normalize_jokers(newTiles);
  std::sort(oldTiles.begin(), oldTiles.end());
  std::sort(newTiles.begin(), newTiles.end());

  if (!std::includes(newTiles.begin(), newTiles.end(), oldTiles.begin(), oldTiles.end())) {
    QMessageBox::warning(this, "Invalid", "Board is missing tiles from previous state. Try again.");
    onResetClicked();
    return;
  }

  if (!valid) {
    QMessageBox::warning(this, "Invalid", "Board has invalid sets. Try again.");
    onResetClicked();
    return;
  }

  std::vector<Tile> placed = get_newly_placed_tiles(initialBoard, newBoard);
  if (placed.empty()) {
      QMessageBox::warning(this, "Invalid", "You must place at least one tile or draw a tile to end your turn.");
      return;
  }

  std::vector<Tile> currentHand = players[currentPlayerIndex]->get_hand();
  bool handValid = true;
  for (const Tile& t : placed) {
      auto it = std::find(currentHand.begin(), currentHand.end(), t);
      if (it != currentHand.end()) {
          currentHand.erase(it);
      } else {
          handValid = false;
          break;
      }
  }

  if (!handValid) {
      QMessageBox::warning(this, "Invalid", "You placed tiles that were not in your hand! Try again.");
      onResetClicked();
      return;
  }

  players[currentPlayerIndex]->set_hand(currentHand);
  board = newBoard;
  
  if (currentHand.empty()) {
      QMessageBox::information(this, "Winner", QString::fromStdString(players[currentPlayerIndex]->get_name()) + " won!");
      std::exit(0);
  }

  // Ensure first move logic is handled for Humans if they had such logic, 
  // but if the board is valid, we just accept it.
  QMessageBox::information(this, "Valid", "Move accepted!");
  startNextTurn();
}

void MainWindow::onDrawTileClicked() {
  if (dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
    bool ok;
    QString tileStr = QInputDialog::getText(this, "AI Draw", "Enter drawn tile (e.g. '13 Red' or 'Joker'):", QLineEdit::Normal, "", &ok);
    if (ok && !tileStr.trimmed().isEmpty()) {
      std::string s = tileStr.trimmed().toStdString();
      Tile t(0, Color::None);
      if (s == "Joker" || s == "joker" || s == "JOKER") {
        t.isJoker = true;
      } else {
        std::stringstream ss(s);
        int val;
        std::string c;
        ss >> val >> c;
        Color col = str_to_color(c);
        t = Tile(val, col);
      }
      static_cast<AIPlayer*>(players[currentPlayerIndex].get())->add_to_hand(t);
      QMessageBox::information(this, "AI Drew", "AI registered drawing!");
      startNextTurn();
    }
  } else {
    if (!bag.is_empty()) {
      bag.draw();
      QMessageBox::information(this, "Draw", "Human drew a tile.");
      startNextTurn();
    }
  }
}

void MainWindow::startNextTurn() {
  currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
  initialBoard = board;
  initialHand = players[currentPlayerIndex]->get_hand();
  drawBoard();

  if (dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
    QMessageBox::information(this, "Turn", "AI's turn. Processing...");
    processAITurn();
  } 
  else {
    QMessageBox::information(this, "Turn", QString("%1's turn!").arg(QString::fromStdString(players[currentPlayerIndex]->get_name())));
  }
}

void MainWindow::processAITurn() {
  AIPlayer* ai = static_cast<AIPlayer*>(players[currentPlayerIndex].get());

  // Position the text item on the right side
  aiStatusText->setPos(800, 50); 
  aiStatusText->setPlainText("AI is thinking...");
  if (!aiStatusText->scene()) scene->addItem(aiStatusText);
  
  ai->set_progress_callback([this](std::string msg) {
      if (msg == "MUST_DRAW_TILE") {
          aiStatusText->setPlainText("AI could not move.\nPlease press 'Draw Tile' for AI.");
      } else {
          aiStatusText->setPlainText(QString::fromStdString(msg));
      }
      QApplication::processEvents();
  });
  
  ai->play_turn(board, bag);
  
  if (aiStatusText->toPlainText().contains("AI could not move.")) {
      // Just leave the text there for the user to press 'Draw Tile'
  } else {
      startNextTurn();
  }
  
  drawBoard();
  
  if (!aiStatusText->scene()) scene->addItem(aiStatusText);
  aiStatusText->setZValue(100);
}

void MainWindow::onSpawnTileClicked() {
    if (dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
        QMessageBox::information(this, "AI Turn", "Spawning tiles during AI's turn is not permitted.");
        return;
    }
    bool ok;
    QString tileStr = QInputDialog::getText(this, "Spawn Tile", "Enter tiles to spawn separated by commas (e.g. '13 Red, Joker, 5 Blue'):", QLineEdit::Normal, "", &ok);
    if (ok && !tileStr.trimmed().isEmpty()) {
        QStringList tileStrings = tileStr.split(",");
        int spawnX = 50;
        
        for (const QString& tsRaw : tileStrings) {
            std::string s = tsRaw.trimmed().toStdString();
            if (s.empty()) continue;
            
            Tile t(0, Color::None);
            if (s == "Joker" || s == "joker" || s == "JOKER") {
                t.isJoker = true;
            } else {
                std::stringstream ss(s);
                int val;
                std::string c;
                if (!(ss >> val >> c)) {
                    QMessageBox::warning(this, "Invalid", QString("Could not parse tile: '%1'. Ignoring remaining.").arg(QString::fromStdString(s)));
                    break;
                }
                std::string lower_c = c;
                for (auto& ch : lower_c) ch = std::tolower(ch);
                
                Color col;
                if (lower_c == "black") col = Color::Black;
                else if (lower_c == "blue") col = Color::Blue;
                else if (lower_c == "red") col = Color::Red;
                else if (lower_c == "orange") col = Color::Orange;
                else {
                    QMessageBox::warning(this, "Invalid", QString("Invalid color in tile: '%1'. Ignoring remaining.").arg(QString::fromStdString(s)));
                    break;
                }
                if (val < 1 || val > 13) {
                    QMessageBox::warning(this, "Invalid", QString("Invalid value in tile: '%1'. Ignoring remaining.").arg(QString::fromStdString(s)));
                    break;
                }
                t = Tile(val, col);
            }
            
            if (!dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
                players[currentPlayerIndex]->add_to_hand(t);
            }

            TileItem* visualItem = new TileItem(t);
            visualItem->setPos(spawnX, 700); 
            scene->addItem(visualItem);
            spawnX += 50; // offset each spawned tile so they don't overlap perfectly
        }
    }
}
