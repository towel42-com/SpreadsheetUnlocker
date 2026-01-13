#include "XLSXUnlocker.h"
#include <QCoreApplication>

#include "Version.h"

#include <QFileInfo>
#include <QCommandLineParser>
#include <iostream>

void Usage()
{
}

int main( int argc, char **argv )
{
    QCoreApplication appl( argc, argv );

    appl.setApplicationName( NVersion::APP_NAME );
    appl.setApplicationVersion( NVersion::getVersionText( true, false ) );
    appl.setOrganizationName( NVersion::VENDOR );
    appl.setOrganizationDomain( NVersion::HOMEPAGE );

    QCommandLineParser parser;
    parser.setApplicationDescription( "XLSX Unlocker" );
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument( QCoreApplication::translate( "main", "xlsx files" ), QCoreApplication::translate( "main", "XLSX files to unlock." ), QCoreApplication::translate( "main", "[files...]" ) );

    parser.process( appl );

    auto xlsxFiles = parser.positionalArguments();
    if ( xlsxFiles.isEmpty() )
    {
        parser.showHelp( 0x01 );
        return 0x01;   // actually never gets hit
    }
    int retVal = 0;
    for ( auto &&xlsxFile : xlsxFiles )
    {
        auto fi = QFileInfo( xlsxFile );
        std::cout << "Unlocking XLSX spreadsheet: '" << xlsxFile.toStdString() << "'\n";
        CXLSXUnlocker unlocker( xlsxFile );
        if ( !unlocker.unlock() )
        {
            std::cerr << "ERROR: Problem unlocking file '" << xlsxFile.toStdString() << "'\n";
            std::cerr << "    - " << unlocker.errorText().toStdString() << "\n";
            retVal |= 0x08;
        }
    }
    return retVal;
}
