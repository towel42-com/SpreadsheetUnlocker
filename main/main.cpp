#include "MainWindow/MainWindow.h"
#include "T42-MathJaxQt6/include/MathJaxQt6.h"
#include <QApplication>
#include <QMessageBox>

#include "Version.h"

int main( int argc, char **argv )
{
    QApplication appl( argc, argv );
    Q_INIT_RESOURCE( MainWindow );
    NTowel42::CMathJaxQt6::initResources();

    appl.setApplicationName( NVersion::APP_NAME );
    appl.setApplicationVersion( NVersion::getVersionString( true, false ) );
    appl.setOrganizationName( NVersion::VENDOR );
    appl.setOrganizationDomain( NVersion::HOMEPAGE );

    appl.setWindowIcon( QPixmap( ":/resources/finddupe.png" ) );

    CMainWindow *wnd = new CMainWindow;
    wnd->show();
    wnd->setWindowTitle( QString( "%1 v%2 - http://%3" ).arg( NVersion::APP_NAME ).arg( NVersion::getVersionString( true, false ) ).arg( NVersion::HOMEPAGE ) );
    return appl.exec();
}
