#include <cassert>
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
#include "Player.hpp"
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

  turnStatusText = new QGraphicsTextItem();
  turnStatusText->setDefaultTextColor(Qt::white);
  QFont turnFont = turnStatusText->font();
  turnFont.setPointSize(20);
  turnFont.setBold(true);
  turnStatusText->setFont(turnFont);
  scene->addItem(turnStatusText);
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
  QInputDialog dialog(this);
  dialog.setWindowTitle("Setup");
  dialog.setLabelText("How many players are playing?");
  dialog.setInputMode(QInputDialog::IntInput);
  dialog.setIntRange(2, 4);
  dialog.setIntValue(2);
  dialog.setIntStep(1);
  dialog.resize(400, 300);
  
  ok = dialog.exec() == QDialog::Accepted;
  if (!ok) {
    std::exit(0);
    return;
  }
  int nPlayers = dialog.intValue();

  // Get player names
  for (int i = 0; i < nPlayers; ++i) {
    std::string rawString = "Enter name for player " + std::to_string(i) + "\n(Player 0 is AI):";
    QString prompt = QString::fromStdString(rawString);
    QInputDialog nameDialog(this);
    nameDialog.setWindowTitle("Player Name");
    nameDialog.setLabelText(prompt);
    nameDialog.setInputMode(QInputDialog::TextInput);
    nameDialog.setTextValue("");
    nameDialog.resize(400, 300);
    ok = nameDialog.exec() == QDialog::Accepted;
    QString qName = nameDialog.textValue();

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
  QInputDialog timeDialog(this);
  timeDialog.setWindowTitle("Time Limit");
  timeDialog.setLabelText("How long are you willing to wait? (in seconds):");
  timeDialog.setInputMode(QInputDialog::IntInput);
  timeDialog.setIntRange(1, 180);
  timeDialog.setIntValue(30);
  timeDialog.setIntStep(1);
  timeDialog.resize(400, 300);
  ok = timeDialog.exec() == QDialog::Accepted;
  timeLimit = ok ? timeDialog.intValue() : 20; // Default fallback

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
      font.setPointSize(14);
      text->setFont(font);
      text->setPos(5, 5);
      if (t.isJoker) {
        text->setPlainText("J");
        text->setDefaultTextColor(Qt::black);
      }
      else {
        if (t.color == Color::Black) text->setDefaultTextColor(Qt::black);
        else if (t.color == Color::Blue) text->setDefaultTextColor(Qt::blue);
        else if (t.color == Color::Red) text->setDefaultTextColor(Qt::red);
        else if (t.color == Color::Orange) text->setDefaultTextColor(QColor(255, 165, 0));
      }
    }
};

void MainWindow::drawBoard() {
  if (aiStatusText && aiStatusText->scene() == scene) scene->removeItem(aiStatusText);
  if (turnStatusText && turnStatusText->scene() == scene) scene->removeItem(turnStatusText);

  scene->clear();

  if (aiStatusText) {
    scene->addItem(aiStatusText);
    aiStatusText->setPos(800, 50);
    aiStatusText->setZValue(100);
  }
  if (turnStatusText) {
    scene->addItem(turnStatusText);
    turnStatusText->setPos(400, 10);
    turnStatusText->setZValue(100);
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
    if (players[currentPlayerIndex]->n_owned_tiles() == 0) {
      QMessageBox::information(this, "Winner", QString::fromStdString(players[currentPlayerIndex]->get_name()) + " won!");
      std::exit(0);
    }
    startNextTurn();
    return;
  }

  Board newBoard = parseBoardFromScene();

  if (!validateBoardRules(newBoard)) {
    onResetClicked();
    return;
  }

  std::vector<Tile> currentHand;
  int nTilesPlaced = 0;
  if (!validatePlayerHand(newBoard, currentHand, nTilesPlaced)) {
    onResetClicked();
    return;
  }

  if (!validateFirstMove(newBoard)) {
    onResetClicked();
    return;
  }

  // Update Game State
  players[currentPlayerIndex]->set_hand(currentHand);
  dynamic_cast<HumanPlayer*>(players[currentPlayerIndex].get())->decrease_tiles(nTilesPlaced);
  board = newBoard;

  assert(players[currentPlayerIndex]->n_owned_tiles() >= 0 && "Can't have less than 0 tiles");
  if (players[currentPlayerIndex]->n_owned_tiles() == 0) {
    QMessageBox::information(this, "Winner", QString::fromStdString(players[currentPlayerIndex]->get_name()) + " won!");
    std::exit(0);
  }

  QMessageBox::information(this, "Valid", "Move accepted!");
  startNextTurn();
}

Board MainWindow::parseBoardFromScene() {
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

  auto addSet = [&]() {
    if (!currentSet.empty()) {
      std::vector<std::pair<int, Tile*>> nonJokers;
      for (size_t i = 0; i < currentSet.size(); ++i) {
        if (!currentSet[i].isJoker) {
          nonJokers.push_back({int(i), &currentSet[i]});
        }
      }

      bool isGroup = true;
      if (nonJokers.size() >= 2) {
        if (nonJokers[0].second->value != nonJokers[1].second->value) {
          isGroup = false;
        }
      } else if (nonJokers.size() == 1) {
        int refIdx = nonJokers[0].first;
        int refVal = nonJokers[0].second->value;
        int startVal = refVal - refIdx;
        int endVal = startVal + currentSet.size() - 1;
        if (startVal >= 1 && endVal <= 13) {
          isGroup = false;
        } else {
          isGroup = true;
        }
      } else {
        assert(false && "Can't be all jokers");
      }

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
            } 
            else {
              currentSet[i].color = Color::None;
            }
          }
        }
      } 
      else {
        Color runColor = nonJokers.empty() ? Color::Black : nonJokers[0].second->color;
        int startVal = nonJokers.empty() ? 1 : nonJokers[0].second->value - nonJokers[0].first;
        for (size_t i = 0; i < currentSet.size(); ++i) {
          if (currentSet[i].isJoker) {
            currentSet[i].value = startVal + int(i);
            currentSet[i].color = runColor;
          }
        }
      }

      Set s(isGroup ? SetType::Group : SetType::Run, currentSet);
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

  return newBoard;
}

bool MainWindow::validateBoardRules(const Board& newBoard) {
  bool valid = true;
  for (const Set& s : newBoard.runs) if (!s.valid()) valid = false;
  for (const Set& s : newBoard.groups) if (!s.valid()) valid = false;
  if (!valid) {
    QMessageBox::warning(this, "Invalid", "Board has invalid sets. Try again.");
    return false;
  }
  return true;
}

bool MainWindow::validatePlayerHand(const Board& newBoard, std::vector<Tile>& outCurrentHand, int& outTilesPlaced) {
  std::vector<Tile> placed = get_newly_placed_tiles(initialBoard, newBoard);
  outTilesPlaced = placed.size();
  if (placed.empty()) {
    QMessageBox::warning(this, "Invalid", "You must place at least one tile or draw a tile to end your turn.");
    return false;
  }
  outCurrentHand = players[currentPlayerIndex]->get_hand();
  bool handValid = true;
  for (const Tile& t : placed) {
    auto it = std::find(outCurrentHand.begin(), outCurrentHand.end(), t);
    if (it != outCurrentHand.end()) {
      outCurrentHand.erase(it);
    } else {
      handValid = false;
      break;
    }
  }
  if (!handValid) {
    QMessageBox::warning(this, "Invalid", "You placed tiles that were not in your hand! Try again.");
    return false;
  }
  return true;
}

bool MainWindow::validateFirstMove(const Board& newBoard) {
  if (!players[currentPlayerIndex]->made_first_move()) {
    int moveSum = val_sum_of_placed_tiles(initialBoard, newBoard);
    if (moveSum < MIN_FIRST_MOVE_SUM) {
      QMessageBox::warning(this, "First Move Invalid", 
          QString("Initial move must have a total value of at least %1 points. "
            "Current value: %2. Try again.").arg(MIN_FIRST_MOVE_SUM).arg(moveSum));
      return false;
    }
    players[currentPlayerIndex]->make_first_move();
  }
  return true;
}

void MainWindow::onDrawTileClicked() {
  if (dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
    QInputDialog drawDialog(this);
    drawDialog.setWindowTitle("AI Draw");
    drawDialog.setLabelText("Enter drawn tile (e.g. '13 Red' or 'Joker'):");
    drawDialog.setInputMode(QInputDialog::TextInput);
    drawDialog.setTextValue("");
    drawDialog.resize(400, 300);
    bool ok = drawDialog.exec() == QDialog::Accepted;
    QString tileStr = drawDialog.textValue();
    if (ok && !tileStr.trimmed().isEmpty()) {
      std::string s = tileStr.trimmed().toStdString();
      Tile t(0, Color::None, true); // Default to a safe Joker state
      bool validParse = false;
      if (s == "Joker" || s == "joker" || s == "JOKER") {
        t = Tile(0, Color::None, true);
        validParse = true;
      } 
      else {
        std::stringstream ss(s);
        int val;
        std::string c;
        if (ss >> val >> c) {
          std::string lower_c = c;
          for (auto& ch : lower_c) ch = std::tolower(ch);
          Color col = str_to_color(lower_c);
          if (col != Color::None && val >= 1 && val <= 13) {
            t = Tile(val, col, false);
            validParse = true;
          }
        }
      }
      if (validParse) {
        static_cast<AIPlayer*>(players[currentPlayerIndex].get())->add_to_hand(t);
        startNextTurn();
      } 
      else {
        QMessageBox::warning(this, "Invalid", "Invalid tile format. Please use '13 Red' or 'Joker'.");
      }
    }
  } 
  else {
    if (players[currentPlayerIndex]->draw_tile(bag)) {
      QMessageBox::information(this, "Draw", "Human drew a tile.");
      startNextTurn();
    }
  }
}

void MainWindow::startNextTurn() {
  currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
  initialBoard = board;
  initialHand = players[currentPlayerIndex]->get_hand();

  if (turnStatusText) {
    turnStatusText->setPlainText(QString::fromStdString(players[currentPlayerIndex]->get_name()) + "'s Turn");
  }
  if (aiStatusText) {
    aiStatusText->setPlainText("");
  }

  // Re-enable all buttons at turn start
  resetBtn->setEnabled(true);
  doneBtn->setEnabled(true);
  drawTileBtn->setEnabled(true);
  spawnTileBtn->setEnabled(true);

  drawBoard();

  if (dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
    processAITurn();
  } 
}

void MainWindow::processAITurn() {
  AIPlayer* ai = static_cast<AIPlayer*>(players[currentPlayerIndex].get());
  // Disable all buttons during AI thought
  resetBtn->setEnabled(false);
  doneBtn->setEnabled(false);
  drawTileBtn->setEnabled(false);
  spawnTileBtn->setEnabled(false);
  // Position the text item on the right side
  aiStatusText->setPos(800, 50); 
  aiStatusText->setPlainText("AI is thinking...");
  if (!aiStatusText->scene()) scene->addItem(aiStatusText);
  ai->set_progress_callback([this](std::string msg) {
      if (msg == "MUST_DRAW_TILE") {
        aiStatusText->setPlainText("AI could not move.\nPlease press 'Draw Tile' for AI.");
      } 
      else {
        aiStatusText->setPlainText(QString::fromStdString(msg));
      }
      QApplication::processEvents();
      });

  ai->play_turn(board, bag);
  // Determine if AI was able to place any tiles
  std::vector<Tile> newlyPlaced = get_newly_placed_tiles(initialBoard, board);
  if (!newlyPlaced.empty()) {
    // AI placed tiles: only Done is allowed
    doneBtn->setEnabled(true);
  } 
  else {
    // AI could not move: only Draw Tile is allowed
    drawTileBtn->setEnabled(true);
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
  QInputDialog spawnDialog(this);
  spawnDialog.setWindowTitle("Spawn Tile");
  spawnDialog.setLabelText("Enter tiles to spawn separated by commas (e.g. '13 Red, Joker, 5 Blue'):");
  spawnDialog.setInputMode(QInputDialog::TextInput);
  spawnDialog.setTextValue("");
  spawnDialog.resize(400, 300);
  bool ok = spawnDialog.exec() == QDialog::Accepted;
  QString tileStr = spawnDialog.textValue();
  if (ok && !tileStr.trimmed().isEmpty()) {
    QStringList tileStrings = tileStr.split(",");
    int spawnX = 50;

    for (const QString& tsRaw : tileStrings) {
      std::string s = tsRaw.trimmed().toStdString();
      if (s.empty()) continue;
      Tile t(0, Color::None, true);
      bool validParse = false;
      if (s == "Joker" || s == "joker" || s == "JOKER") {
        t = Tile(0, Color::None, true);
        validParse = true;
      } 
      else {
        std::stringstream ss(s);
        int val;
        std::string c;
        if (ss >> val >> c) {
          std::string lower_c = c;
          for (auto& ch : lower_c) ch = std::tolower(ch);
          Color col = str_to_color(lower_c);
          if (col != Color::None && val >= 1 && val <= 13) {
            t = Tile(val, col, false);
            validParse = true;
          }
        }
      }
      if (!validParse) {
        QMessageBox::warning(this, "Invalid", QString("Could not parse tile: '%1'. Skipping this one.").arg(QString::fromStdString(s)));
        continue;
      }
      if (!dynamic_cast<AIPlayer*>(players[currentPlayerIndex].get())) {
        players[currentPlayerIndex]->add_to_hand(t);
      }
      TileItem* visualItem = new TileItem(t);
      visualItem->setPos(spawnX, 700); 
      scene->addItem(visualItem);
      spawnX += 50;
    }
  }
}
