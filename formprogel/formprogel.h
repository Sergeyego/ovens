#ifndef FORMPROGEL_H
#define FORMPROGEL_H

#include <QWidget>
#include "rest/resttablemodel.h"
#include "rest/relmodels.h"

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
    RestRelModel *modelElDim;
    void loadSettings();
    void saveSettings();
    QString currentId;

private slots:
    void updFinished();
};

#endif // FORMPROGEL_H
