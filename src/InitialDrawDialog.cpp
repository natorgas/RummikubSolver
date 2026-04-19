#include "InitialDrawDialog.hpp"
#include "Utils.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFont>

InitialDrawDialog::InitialDrawDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("AI Initial Draw");
  setModal(true);

  // Set a generous default size and a reasonable minimum
  resize(1200, 600); 
  setMinimumSize(800, 600);

  // Increase the global font for this dialog so text scales visually
  QFont defaultFont = this->font();
  defaultFont.setPointSize(12);
  defaultFont.setBold(true);
  this->setFont(defaultFont);

  setupUi();
}

void InitialDrawDialog::setupUi() {
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(20, 20, 20, 20);
  mainLayout->setSpacing(15);

  QGridLayout* gridLayout = new QGridLayout();
  // Set spacing between tiles
  gridLayout->setSpacing(8);

  int row = 0;
  for (const Color& col : ALL_COLORS) {
    if (col == Color::None) continue; 

    QLabel* colorLabel = new QLabel(QString::fromStdString(color_to_str(col)));
    colorLabel->setStyleSheet("font-size: 16px; color: #b7b7b7;");
    gridLayout->addWidget(colorLabel, row, 0);

    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
      Tile t(val, col);
      int idx = selectionCounts.index(t);

      QPushButton* btn = new QPushButton(QString::number(val), this);

      btn->setMinimumSize(45, 60);
      btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      btn->setProperty("tileIndex", idx);
      tileButtons[idx] = btn;
      selectionCounts[t] = 0; 

      connect(btn, &QPushButton::clicked, this, &InitialDrawDialog::onTileClicked);
      gridLayout->addWidget(btn, row, val);

      // Tell the grid that this column should grow
      gridLayout->setColumnStretch(val, 1);
    }
    // Tell the grid that this row should grow
    gridLayout->setRowStretch(row, 1);
    row++;
  }

  // 2. Create Joker button
  Tile joker(0, Color::None, true);
  int jIdx = selectionCounts.index(joker);

  QPushButton* jBtn = new QPushButton("JOKER", this);
  jBtn->setMinimumSize(100, 60);
  jBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
  jBtn->setProperty("tileIndex", jIdx);

  tileButtons[jIdx] = jBtn;
  selectionCounts[joker] = 0;
  connect(jBtn, &QPushButton::clicked, this, &InitialDrawDialog::onTileClicked);

  // 3. Layout assembly
  mainLayout->addLayout(gridLayout, 10); // Stretch factor 10 (takes most space)

  QHBoxLayout* bottomLayout = new QHBoxLayout();
  bottomLayout->addWidget(new QLabel("Special:"));
  bottomLayout->addWidget(jBtn);
  bottomLayout->addStretch();

  finishButton = new QPushButton("Finish (0/14)", this);
  finishButton->setMinimumSize(200, 60);
  finishButton->setEnabled(false);

  // Make the finish button look distinct
  finishButton->setStyleSheet("font-size: 18px; font-weight: bold;");

  connect(finishButton, &QPushButton::clicked, this, &InitialDrawDialog::onFinishClicked);
  bottomLayout->addWidget(finishButton);

  mainLayout->addLayout(bottomLayout, 1); // Stretch factor 1
}

void InitialDrawDialog::onTileClicked() {
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if (!btn) return;

  int idx = btn->property("tileIndex").toInt();
  int& count = *(selectionCounts.begin() + idx);

  if (count == 0) {
    if (totalSelected < 14) {
      count = 1;
      totalSelected++;
    }
  } 
  else if (count == 1) {
    if (totalSelected < 14) {
      count = 2;
      totalSelected++;
    } 
    else {
      // Can't add more, so deselect
      totalSelected -= 1;
      count = 0;
    }
  } 
  else {
    // count == 2
    totalSelected -= 2;
    count = 0;
  }

  updateButtonVisuals(btn, count);
  finishButton->setText(QString("Finish (%1/14)").arg(totalSelected));
  finishButton->setEnabled(totalSelected == 14);
}

void InitialDrawDialog::updateButtonVisuals(QPushButton* btn, int count) {
  if (count == 0) {
    btn->setStyleSheet("");
  } else if (count == 1) {
    btn->setStyleSheet("background-color: #90EE90; border: 2px solid green; color: black;"); 
  } else if (count == 2) {
    btn->setStyleSheet("background-color: #32CD32; border: 3px solid #006400; color: white; font-weight: bold;"); 
  }
}

std::vector<Tile> InitialDrawDialog::getDrawnTiles() const {
  std::vector<Tile> result;
  for (const Color& col : ALL_COLORS) {
    for (int val = MIN_TILE_VALUE; val <= MAX_TILE_VALUE; ++val) {
      if (col == Color::None && val != 0) continue; 
      Tile t(val, col, (col == Color::None));
      int count = selectionCounts[t];
      for (int i = 0; i < count; ++i) result.push_back(t);
    }
  }
  Tile joker(0, Color::None, true);
  int jCount = selectionCounts[joker];
  for(int i=0; i<jCount; ++i) result.push_back(joker);

  return result;
}

void InitialDrawDialog::onFinishClicked() {
  accept();
}
