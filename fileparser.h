#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

#include <fstream>
#include <string>
#include <string_view>
#include <sstream>

#include "plotter.h"

class FileParser : public QObject
{
    Q_OBJECT
public:
    FileParser(Plotter* _plotter, QString& file, QObject *parent = nullptr);

private:
    Plotter* plotter;
    std::stringstream header;
    std::wstring filePath;

    bool getTerms(std::string& line, std::vector<std::string_view>& outTerms);
    int processFile();

public slots:
    void slotRun();

signals:
    //Possible result values:
    //0 - success; -1 - there is header in the middle of the data; -2 - wrong elements count in data line; -3 - can't convert data to float
    //-4 - can't open file
    void sigProcessComplete(int result, QString header);

};

#endif // FILEPARSER_H
