#ifndef FORMPROGWIRE_H
#define FORMPROGWIRE_H

#include <QWidget>
#include <QSettings>
#include "rest/resttablemodel.h"
#include "rest/restmapper.h"

namespace Ui {
class FormProgWire;
}

class FormProgWire : public QWidget
{
    Q_OBJECT

public:
    explicit FormProgWire(QWidget *parent = nullptr);
    ~FormProgWire();

private:
    Ui::FormProgWire *ui;
    RestTableModel *modelProg;
    RestTableModel *modelProgData;
    RestMapper *mapper;
    void loadSettings();
    void saveSettings();

private slots:
    void currentIndexChanged(int index);
    void setCurrentStep();
};

#endif // FORMPROGWIRE_H
