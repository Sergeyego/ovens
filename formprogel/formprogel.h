#ifndef FORMPROGEL_H
#define FORMPROGEL_H

#include <QWidget>
#include "rest/resttablemodel.h"
#include "rest/relmodels.h"
#include "progdatamodel.h"

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
    RestTableModel *modelProg;
    RestTableModel *modelProgData;
    ProgDataModel *modelProc;
    void loadSettings();
    void saveSettings();
    QVariant currentId;
    int copyId;
    int copyGrp;

private slots:
    void upd();
    void updFinished();
    void selectElDim(QModelIndex index);
    void updProgFinished();
    void selectProg();
    void setCurrentStep();
    void copy();
    void paste();
    void updProc();
};

#endif // FORMPROGEL_H
