#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Calculators/Core/CalculatorBase.h"
#include "Calculators/Core/CalculatorPage.h"
#include "Calculators/Core/JsonCalculator.h"

#include "T42-MathJaxQt6/include/MathJaxQt6.h"
#include "T42-Utils/utils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <QButtonGroup>
#include <QResizeEvent>
#include <QRegularExpression>
#include <QFileDialog>
#include <QProgressDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTimer>

#include <set>
#include <libloaderapi.h>
#include "T42-Utils/FileUtils.h"

static QString toString( EFormulaType formulaType )
{
    if ( formulaType == EFormulaType::eBaseFormula )
        return QObject::tr( "BaseFormula" );
    else if ( formulaType == EFormulaType::eCurrentFormula )
        return QObject::tr( "CurrentFormula" );
    else if ( formulaType == EFormulaType::eCurrentValueFormula )
        return QObject::tr( "CurrentValueFormula" );
    else if ( formulaType == EFormulaType::eJSFormula )
        return QObject::tr( "JSFormula" );
    return QObject::tr( "Unknown" );
}

CMainWindow::CMainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    fImpl( new Ui::CMainWindow )
{
    setWindowIcon( QIcon( ":/resources/scubacalc.png" ) );
    setAttribute( Qt::WA_DeleteOnClose );

    fImpl->setupUi( this );
    initMathJaxWidgets();

    fBlankPage = new QWidget;
    fImpl->stackedWidget->addWidget( fBlankPage );
    fImpl->stackedWidget->installEventFilter( this );

    connect(
        fImpl->formulaGroupBox, &NTowel42::CMathJaxQt6GroupBox::sigErrorMessage,
        [ = ]( const QString &msg )
        {
            fImpl->formulaGroupBox->setMathJaxVisible( false );
            QMessageBox::critical( this, tr( "Error in MathJax Engine" ), msg );
        } );

    connect( fImpl->reset, &QPushButton::clicked, this, &CMainWindow::slotResetCurrentPage );
    loadSettings();

    connect( fImpl->whichCalculator, &QTreeWidget::currentItemChanged, this, &CMainWindow::slotSelectCalculator );
    connect( fImpl->imperial, &QRadioButton::toggled, this, &CMainWindow::slotUnitsChanged );
    connect( fImpl->metric, &QRadioButton::toggled, this, &CMainWindow::slotUnitsChanged );
    connect( fImpl->seaWater, &QRadioButton::toggled, this, &CMainWindow::slotWaterChanged );
    connect( fImpl->freshWater, &QRadioButton::toggled, this, &CMainWindow::slotWaterChanged );

    connect(
        fImpl->menuFile, &QMenu::aboutToShow,   //
        [ this ]()   //
        {
            fImpl->actionGenerateAllFormulas->setVisible( false );
            fImpl->actionGenerateUpdatedFormulas->setVisible( false );
        } );

    QTimer::singleShot( 100, [ = ] { loadCalculators(); } );
}

void CMainWindow::initMathJaxWidgets()
{
    fRenderingEngine = fImpl->formulaGroupBox->engine();
    fImpl->formulaGroupBox->setTitle( {} );

    fImpl->formulaGroupBox->slotSetAutoUpdateMinimumParentHeight( true );
    fImpl->formulaGroupBox->slotHideEmptyOrInvalid( true );
    fImpl->formulaGroupBox->slotSetAutoSizeToParentWidth( true );
    fImpl->formulaGroupBox->updateMathJaxWidgetName();
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::loadSettings()
{
    QSettings settings;
    if ( settings.value( "ImperialUnits", true ).toBool() )
        fImpl->imperial->setChecked( true );
    else
        fImpl->metric->setChecked( true );
    if ( settings.value( "SeaWater", true ).toBool() )
        fImpl->seaWater->setChecked( true );
    else
        fImpl->freshWater->setChecked( true );
}

void CMainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue( "ImperialUnits", fImpl->imperial->isChecked() );
    settings.setValue( "SeaWater", fImpl->seaWater->isChecked() );
}

void CMainWindow::loadCalculators()
{
    loadDllCalculators();
    loadJsonCalculators();
    fImpl->whichCalculator->expandAll();
    fImpl->whichCalculator->sortByColumn( 0, Qt::SortOrder::AscendingOrder );
    fImpl->whichCalculator->resizeColumnToContents( 0 );
    auto colWidth = fImpl->whichCalculator->columnWidth( 0 );
    fImpl->whichCalculator->setMinimumWidth( colWidth + 20 );

    slotSelectCalculator( nullptr );
}

void CMainWindow::loadJsonCalculators()
{
    auto calcDir = QApplication::applicationDirPath() + "/Calculators";

    auto ii = QDirIterator( calcDir, QStringList() << "*.json" );
    while ( ii.hasNext() )
    {
        auto jsonFile = ii.next();
        if ( !QFileInfo( jsonFile ).isFile() )
            continue;

        std::optional< QString > errorMsg;
        auto calculators = CJsonCalculator::create( jsonFile, this, errorMsg );
        if ( !calculators.has_value() )
        {
            auto msg = errorMsg.has_value() ? errorMsg.value() : tr( "Unknown error in loading" );
            QMessageBox::critical(
                this, tr( "Error loading Json file" ),   //
                tr( "Error loading file '%1'\n%2" ).arg( jsonFile ).arg( msg ) );
            continue;
        }

        auto instantiator = [ this ]( CCalculatorBase *sourceCalc ) -> CJsonCalculator *
        {
            auto jsonCalc = dynamic_cast< CJsonCalculator * >( sourceCalc );
            if ( !jsonCalc )
                return nullptr;

            auto jsonObj = jsonCalc->jsonObject();
            auto retVal = CJsonCalculator::create( jsonCalc->calculatorProjectName() + "-reversed", jsonObj, this );
            if ( retVal && retVal->hasError() )
            {
                delete retVal;
                retVal = nullptr;
            }
            return retVal;
        };

        for ( auto &&ii : calculators.value() )
        {
            addCalculator( ii, instantiator );
        }
    }
}

void CMainWindow::loadDllCalculators()
{
    auto calcDir = QApplication::applicationDirPath() + "/Calculators";

    auto ii = QDirIterator( calcDir, QStringList() << "*.dll" );
    while ( ii.hasNext() )
    {
        auto dllName = ii.next();
        auto fi = QFileInfo( dllName );
        auto baseName = fi.baseName();
        bool isDebugDLL = baseName.endsWith( "d" );
#ifdef _DEBUG
        if ( !isDebugDLL )
#else
        if ( isDebugDLL )
#endif
            continue;

        if ( !QFileInfo( dllName ).isFile() )
            continue;
        auto hLib = ::LoadLibrary( (LPCWSTR)dllName.utf16() );
        if ( !hLib )
        {
            auto lastError = NTowel42Utils::getLastError();   // windows only
            QMessageBox::critical( this, tr( "Could not load Calculator" ), tr( "Loading calculator '%1' failed with error:<br/> %2" ).arg( dllName ).arg( lastError ) );
            continue;
        }

        auto instantiator = (TInstantiateCalcFunc)GetProcAddress( hLib, kInstantiateCalcFuncName );
        if ( !instantiator )
            continue;

        auto calculator = instantiator();
        addCalculator( calculator, [ instantiator ]( CCalculatorBase * /*sourceCalc*/ ) { return instantiator(); } );
    }
}

void CMainWindow::addCalculator( CCalculatorBase *calculator, const std::function< CCalculatorBase *( CCalculatorBase *sourceCalc ) > &instantiator )
{
    if ( !calculator )
        return;

    auto path = calculator->calculatorPath();
    if ( path.isEmpty() )
        return;

    auto calculatorName = calculator->calculatorName();
    path.push_back( calculatorName );
    auto leaf = findItem( fImpl->whichCalculator->invisibleRootItem(), path, true );
    fCalculators[ leaf ] = calculator;

    auto page = calculator->getPage( nullptr );
    if ( !page )
    {
        //qCDebug( ScubaCalculator ).noquote().nospace() << "No widget for page :" << path;
        return;
    }

    fImpl->stackedWidget->addWidget( page );
    fPageToItem[ page ] = leaf;

    calculator->setUpdateFormulaFunc( [ = ]( CCalculatorPage *calcPage, const QString &formula, bool finished )   //
                                      {   //
                                          this->setFormulaForPage( calcPage, formula, finished );
                                      } );

    if ( instantiator && calculator->isReversible() && !calculator->isReversed() )
    {
        auto reversedCalc = instantiator( calculator );
        if ( reversedCalc )
        {
            reversedCalc->setIsReversed( calculator, true );
            addCalculator( reversedCalc, {} );
        }
    }
}

QString pathForItem( QTreeWidgetItem *rootItem, QTreeWidgetItem *item )
{
    if ( !item )
        return {};

    auto parentItem = item->parent();

    QString retVal;
    if ( parentItem && ( parentItem != rootItem ) )
        retVal = pathForItem( rootItem, parentItem );

    if ( !retVal.isEmpty() )
        retVal += ".";
    retVal += item->text( 0 );
    return retVal;
}

QTreeWidgetItem *CMainWindow::findItem( QTreeWidgetItem *parent, const QStringList &path, bool createIfNecessary )
{
    if ( !parent )
        return nullptr;
    QTreeWidgetItem *foundChild = nullptr;
    for ( int ii = 0; !foundChild && ( ii < parent->childCount() ); ++ii )
    {
        auto child = parent->child( ii );
        if ( !child )
            continue;
        if ( child->text( 0 ) == path.front() )
            foundChild = child;
    }
    if ( !foundChild )
    {
        if ( createIfNecessary )
        {
            foundChild = new QTreeWidgetItem( parent );
            foundChild->setText( 0, path.front() );
            qDebug() << "Creating Item: " << pathForItem( fImpl->whichCalculator->invisibleRootItem(), foundChild );
        }
    }
    if ( path.length() == 1 )
        return foundChild;

    return findItem( foundChild, path.mid( 1 ), createIfNecessary );
}

CCalculatorBase *CMainWindow::getCalculator( QTreeWidgetItem *leaf ) const
{
    auto pos = fCalculators.find( leaf );
    if ( pos != fCalculators.end() )
        return ( *pos ).second;
    return nullptr;
}

CCalculatorBase *CMainWindow::getCalculator( QWidget *page ) const
{
    auto pos = fPageToItem.find( page );
    if ( pos == fPageToItem.end() )
        return nullptr;
    return getCalculator( ( *pos ).second );
}

CCalculatorBase *CMainWindow::currentCalculator() const
{
    auto page = fImpl->stackedWidget->currentWidget();
    if ( page == fBlankPage )
        return nullptr;

    return getCalculator( page );
}

CCalculatorPage *CMainWindow::currentCalculatorPage() const
{
    auto page = fImpl->stackedWidget->currentWidget();
    if ( page == fBlankPage )
        return nullptr;

    return dynamic_cast< CCalculatorPage * >( page );
}

void CMainWindow::slotUnitsChanged()
{
    auto page = fImpl->stackedWidget->currentWidget();
    if ( page == fBlankPage )
        return;

    saveSettings();
    auto item = getItemForPage( page );
    if ( !item )
        return;

    auto calc = getCalculator( item );
    if ( !calc )
        return;

    calc->setImperial( fImpl->imperial->isChecked() );
}

void CMainWindow::slotWaterChanged()
{
    auto page = fImpl->stackedWidget->currentWidget();
    if ( page == fBlankPage )
        return;

    saveSettings();
    auto item = getItemForPage( page );
    if ( !item )
        return;

    auto calc = getCalculator( item );
    if ( !calc )
        return;
    calc->setSeaWater( fImpl->seaWater->isChecked() );
}

void CMainWindow::slotSelectCalculator( QTreeWidgetItem *item )
{
    auto calculator = getCalculator( item );
    auto page = dynamic_cast< CCalculatorPage * >( calculator ? calculator->getPage( nullptr ) : nullptr );
    bool needsInit = page ? page->needsInit() : false;

    if ( page )
    {
        fImpl->pageName->setText( calculator->calculatorName() );
    }
    else
    {
        fImpl->pageName->setText( "Please Select a Calculator" );
    }

    setCurrentPage( item, page, needsInit );
}

void CMainWindow::setCurrentPage( QTreeWidgetItem *item, CCalculatorPage *page, bool initPage )
{
    bool showUnits = page != nullptr;
    bool showWaterType = page != nullptr;
    if ( page == nullptr )
    {
        fImpl->stackedWidget->setCurrentWidget( fBlankPage );
    }
    else
    {
        if ( !initPage )
            fImpl->stackedWidget->setCurrentWidget( page );
        showUnits = page->property( "showUnits" ).toBool();
        showWaterType = page->property( "showWaterType" ).toBool();
    }

    if ( page && initPage )
    {
        auto calc = getCalculator( item );
        if ( !calc )
            return;
        fImpl->stackedWidget->setCurrentWidget( page );
        calc->init( fImpl->imperial->isChecked(), fImpl->seaWater->isChecked() );
    }

    this->showUnits( showUnits );
    this->showWaterType( showWaterType );
    fImpl->reset->setVisible( page != nullptr );
    loadFormulasForPage( page );
}

void CMainWindow::showUnits( bool show )
{
    fImpl->unitGroupBox->setVisible( show );
}

void CMainWindow::showWaterType( bool show )
{
    fImpl->waterGroupBox->setVisible( show );
}

QTreeWidgetItem *CMainWindow::getItemForPage( QWidget *page ) const
{
    auto pos = fPageToItem.find( page );
    if ( pos != fPageToItem.end() )
        return ( *pos ).second;
    return nullptr;
}

void CMainWindow::setMathJaxWidgetsVisible( bool visible )
{
    fImpl->formulaGroupBox->setMathJaxVisible( visible );
}

void CMainWindow::setFormulaForPage( CCalculatorPage *page, const QString &formula, bool finished )
{
    //Q_ASSERT( !NUtilities::hasVariable( formula ) );

    auto widget = mathJaxGoupBox();
    Q_ASSERT( widget );
    auto pos = this->fPageToFormulasMap.find( page );
    if ( pos != fPageToFormulasMap.end() )
    {
        if ( ( *pos ).second == formula )
        {
            if ( finished )
                loadFormulasForPage( page );
            return;
        }
    }
    else
    {
        pos = fPageToFormulasMap.insert( { page, std::optional< QString >() } ).first;
    }
    ( *pos ).second = formula;
    if ( finished )
        loadFormulasForPage( page );
}

void CMainWindow::loadFormulasForPage( CCalculatorPage *page )
{
    if ( !page )
    {
        setMathJaxWidgetsVisible( false );
        return;
    }

    auto formula = formulaForPage( page );
    mathJaxGoupBox()->setFormula( formula );
}

NTowel42::CMathJaxQt6GroupBox *CMainWindow::mathJaxGoupBox() const
{
    return fImpl->formulaGroupBox;
}

std::optional< QString > CMainWindow::formulaForPage( QWidget *page )
{
    auto pos = fPageToFormulasMap.find( page );

    auto calculator = getCalculator( page );

    if ( pos == fPageToFormulasMap.end() )
    {
        //qCDebug( ScubaCalculator ).noquote().nospace() << "Page: '" << calculator->calculatorName() << "' has no " << toString( formulaType ) << " formula.";
        return {};
    }
    auto retVal = ( *pos ).second;
    //qCDebug( ScubaCalculator ).noquote().nospace() << "Page: '" << calculator->calculatorName() << "' " << toString( formulaType ) << " formula is '" << retVal << "'";
    return retVal;
}

void CMainWindow::slotResetCurrentPage()
{
    auto page = fImpl->stackedWidget->currentWidget();
    if ( page == fBlankPage )
        return;

    auto calc = getCalculator( page );
    if ( !calc )
        return;
    calc->resetVariables();
}
