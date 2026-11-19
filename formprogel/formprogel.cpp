#include "formprogel.h"
#include "ui_formprogel.h"

FormProgEl::FormProgEl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormProgEl)
{
    ui->setupUi(this);
}

FormProgEl::~FormProgEl()
{
    delete ui;
}
