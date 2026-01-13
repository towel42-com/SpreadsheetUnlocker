#ifndef __XLSXUNLOCKER_H
#define __XLSXUNLOCKER_H

#include <QFileInfo>
class QString;
namespace NTowel42Utils
{
    namespace NFileUtils
    {
        struct SZipFileInfo;
    }
}

class CXLSXUnlocker
{
public:
    CXLSXUnlocker( const QString &path );
    CXLSXUnlocker( const QFileInfo &path );

    bool unlock();
    QString errorText() const { return fErrorText; }

private:
    bool unprotectSheet( QByteArray & data );
    bool setError( const QString &msg );
    QFileInfo fPath;
    QString fErrorText;
};

#endif