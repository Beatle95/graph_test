#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>

#include <fstream>
#include <string>
#include <string_view>
#include <sstream>

#include "plotter.h"

enum FileParserState{
    SUCCESS = 0,
    HEADER_ERROR = -1,
    ELEMENTS_COUNT_ERROR = -2,
    FILE_CANT_OPEN = -3
};

class FileParser : public QObject
{
    Q_OBJECT
public:
    FileParser(QSharedPointer<Plotter>& _plotter, QString& file, QObject *parent = nullptr);

private:
    QSharedPointer<Plotter> plotter;
    std::stringstream header;
    std::wstring filePath;

    bool getTerms(std::string& line, std::vector<std::string_view>& outTerms);
    int processFile();

public slots:
    void slotRun();

signals:
    //Possible result values:
    //0 - success; -1 - there is header in the middle of the data; -2 - wrong elements count in data line; -4 - can't open file
    void sigProcessComplete(int result, QString header);

};

#endif // FILEPARSER_H
