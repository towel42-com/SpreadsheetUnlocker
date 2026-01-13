#include "XLSXUnlocker.h"
#include "T42-Utils/ZIP.h"
#include "T42-Utils/BackupFile.h"

#include <QBuffer>

CXLSXUnlocker::CXLSXUnlocker( const QString &path ) :
    CXLSXUnlocker( QFileInfo( path ) )
{
}

CXLSXUnlocker::CXLSXUnlocker( const QFileInfo &path ) :
    fPath( path )
{
}

bool CXLSXUnlocker::unprotectSheet( QByteArray &data )
{
    qsizetype pos = 0;
    while ( ( pos = data.indexOf( "<sheetProtection", pos ) ) != -1 )
    {
        auto pos2 = data.indexOf( ">", pos );
        if ( pos2 == -1 )
        {
            return setError( QObject::tr( "Malformed XLSX file.", "CXLSXUnlocker::unprotectSheet()" ) );
        }
        auto endPos = pos2;
        if ( data[ pos2 - 1 ] == '/' )
        {
            endPos++;
        }
        else
        {
            const auto endBlock = QByteArray( "</sheetProtection>" );
            pos2 = data.indexOf( endBlock );
            if ( pos2 == -1 )
            {
                return setError( QObject::tr( "Malformed XLSX file.", "CXLSXUnlocker::unprotectSheet()" ) );
            }
            endPos = pos2 + endBlock.length();
        }
        data.erase( data.begin() + pos, data.begin() + endPos );
    }
    return true;
}

bool CXLSXUnlocker::unlock()
{
    if ( !fPath.exists() )
    {
        return setError( QObject::tr( "does not exist.", "CXLSXUnlocker::unlock()" ) );
    }

    auto unzipper = NTowel42Utils::NFileUtils::CZipReader( fPath );
    if ( unzipper.status() != NTowel42Utils::NFileUtils::EStatus::eNoError )
    {
        setError( unzipper.statusText() );
        return false;
    }

    auto files = unzipper.fileInfoList();
    if ( files.isEmpty() || ( unzipper.status() != NTowel42Utils::NFileUtils::EStatus::eNoError ) )
    {
        if ( unzipper.status() != NTowel42Utils::NFileUtils::EStatus::eNoError )
            setError( unzipper.statusText() );
        else
            setError( "Zip file is empty." );

        return false;
    }

    QBuffer outBuffer;
    auto zipper = NTowel42Utils::NFileUtils::CZipWriter( &outBuffer );

    for ( auto &&data : files )
    {
        QFileInfo fi( data.fFilePath );
        auto path = fi.path();

        auto fileData = unzipper.fileData( data.fFilePath );
        if ( fileData.has_value() && ( path == "xl/worksheets" ) )
        {
            if ( !unprotectSheet( fileData.value() ) )
                return false;
        }
        zipper.addItem( data, fileData );
    }
    if ( unzipper.status() != NTowel42Utils::NFileUtils::EStatus::eNoError )
    {
        setError( unzipper.statusText() );
        return false;
    }
    if ( zipper.status() != NTowel42Utils::NFileUtils::EStatus::eNoError )
    {
        setError( zipper.statusText() );
        return false;
    }

    unzipper.close();
    zipper.close();

    QString msg;
    if ( !NTowel42Utils::NFileUtils::backup( fPath, &msg ) )
    {
        setError( msg );
        return false;
    }

    QFile outFile( fPath.absoluteFilePath() );
    if ( !outFile.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        setError( "Problem writing to zip file." );
        return false;
    }
    outFile.write( outBuffer.buffer() );
    outFile.close();

    return true;
}

bool CXLSXUnlocker::setError( const QString &msg )
{
    fErrorText = "'" + fPath.absoluteFilePath() + "' - " + msg;
    return false;
}
