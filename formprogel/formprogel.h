#ifndef FORMPROGEL_H
#define FORMPROGEL_H

#include <QWidget>

namespace Ui {
class FormProgEl;
}

class FormProgEl : public QWidget
{
    Q_OBJECT

public:
    explicit FormProgEl(QWidget *parent = nullptr);
    ~FormProgEl();

private:
    Ui::FormProgEl *ui;
};

#endif // FORMPROGEL_H
