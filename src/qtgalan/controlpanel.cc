#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "controlpanel.h"
#include "mainwin.h"

#include "galan/global.h"

#include <qlayout.h>
#include <qpushbutton.h>

GALAN_USE_NAMESPACE
using namespace std;

ControlPanel::ControlPanel(QWidget *parent)
  : QWidget(parent),
    scrollView(0)
{
  QVBoxLayout *vbl = new QVBoxLayout(this);

  QHBoxLayout *hbl = new QHBoxLayout(vbl);

  scrollView = new QScrollView(this);
  vbl->addWidget(scrollView);

  scrollView->resizeContents(2048, 2048);
  scrollView->enableClipper(true);
  scrollView->addChild(new QPushButton("Hello", scrollView->viewport()), 10, 10);
  scrollView->addChild(new QPushButton("Aloha", scrollView->viewport()), 1000, 50);

  hbl->addWidget(new QPushButton("A", this));
  hbl->addWidget(new QPushButton("B", this));
  hbl->addWidget(new QPushButton("C", this));
  hbl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
}

ControlPanel::~ControlPanel() {
}
