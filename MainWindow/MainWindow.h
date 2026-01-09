#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include <unordered_map>
#include <memory>
#include <optional>

class QDir;
class QProgressDialog;

#include "Calculators/Core/CalculatorFwd.h"
namespace Ui
{
    class CMainWindow;
}

namespace NTowel42
{
    class CMathJaxQt6;
    class CMathJaxQt6GroupBox;
}

class CCalculatorBase;
class QTreeWidgetItem;
class CMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    CMainWindow( QWidget *parent = 0 );
    ~CMainWindow();

public:
    void loadCalculators();

private:
Q_SIGNALS:

public Q_SLOTS:
    void slotSelectCalculator( QTreeWidgetItem *item );
    void slotUnitsChanged();
    void slotWaterChanged();
    void slotResetCurrentPage();

private:
    CCalculatorBase *currentCalculator() const;
    CCalculatorPage *currentCalculatorPage() const;

    void setCurrentPage( QTreeWidgetItem *item, CCalculatorPage *page, bool initPage );
    void loadSettings();
    void saveSettings();

    void showUnits( bool show );
    void showWaterType( bool show );
    void addCalculator( CCalculatorBase *calculator, const std::function< CCalculatorBase *( CCalculatorBase *sourceCalc ) > &instantiator );

    void loadFormulasForPage( CCalculatorPage *page );

    bool renderSVG( const QString &formula );
    std::optional< QString > formulaForPage( QWidget *page );

    void initMathJaxWidgets();
    NTowel42::CMathJaxQt6GroupBox *mathJaxGoupBox() const;
    void setMathJaxWidgetsVisible( bool visible );

private:
    void loadJsonCalculators();
    void loadDllCalculators();
    void setFormulaForPage( CCalculatorPage *page, const QString &formula, bool finished );

    CCalculatorBase *getCalculator( QTreeWidgetItem *leaf ) const;
    CCalculatorBase *getCalculator( QWidget *page ) const;
    QTreeWidgetItem *getItemForPage( QWidget *page ) const;

    QTreeWidgetItem *findItem( QTreeWidgetItem *parent, const QStringList &path, bool createIfNecessary );
    std::unique_ptr< Ui::CMainWindow > fImpl;

    QWidget *fBlankPage{ nullptr };

    std::unordered_map< QTreeWidgetItem *, CCalculatorBase * > fCalculators;
    std::unordered_map< QWidget *, QTreeWidgetItem * > fPageToItem;
    std::unordered_map< QWidget *, std::optional< QString > > fPageToFormulasMap;

    std::shared_ptr< NTowel42::CMathJaxQt6 > fRenderingEngine;
};

#endif
