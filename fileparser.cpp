#include "fileparser.h"

FileParser::FileParser(QSharedPointer<Plotter>& _plotter, QString& file, QObject *parent) : QObject(parent), plotter(_plotter) {
    filePath = file.toStdWString();
}

//Reading data from selected file and preparing for drawing
//Possible result values:
//0 - success; -1 - there is header in the middle of the data; -2 - wrong elements count in data line; -4 - can't open file
int FileParser::processFile(){
    std::ifstream file;
    file.open(filePath);
    if (file.good())
    {
        QMutexLocker locker(&plotter->mutex);
        plotter->clearPlotPoints();
        std::string line;
        bool isHeader = true;
        while (std::getline(file, line))
        {
            std::vector<std::string_view> terms;
            terms.reserve(2);
            //Header could be any length and there could be space filled lines between header lines
            //but when we reach data, there can't be any headers further
            if(getTerms(line, terms)){
                if(!isHeader)
                    return FileParserState::HEADER_ERROR;
                header << line << '\n';
            }
            else{
                if(terms.empty())
                    continue;
                isHeader = false;
                //Now we get data
                if(terms.size() != 2)		//If elements coutn != 2 -> data corrupted
                    return FileParserState::ELEMENTS_COUNT_ERROR;
                float timestamp = 0.0f;
                float measurement = 0.0f;
                timestamp = atof(terms[0].data());
                measurement = atof(terms[1].data());
                plotter->appendPlotPoint(timestamp, measurement);
            }
       }
       file.close();
    }
    else
        return FileParserState::FILE_CANT_OPEN;

    return FileParserState::SUCCESS;
}

void FileParser::slotRun()
{
    int result = processFile();
    std::string str = header.str();
    QString qstr = QString::fromUtf8(str.c_str());
    emit sigProcessComplete(result, qstr);
}

//Function for single line parsing, returns true, if current line is header, false otherwise
//using vector, cause it's destructor is more efficient then list destructor
//it is better to reserve 2 elements before passing it here
bool FileParser::getTerms(std::string& line, std::vector<std::string_view>& outTerms){
    char* begin = nullptr;
    size_t size = line.size();
    bool isFirst = true;
    for(size_t i = 0; i < size; i++){
        if(begin == nullptr && line[i] != ' ' && line[i] != '\t'){
            if(isFirst){			//'#' could be a part of header line
                if(line[i] == '#')
                    return true;	//now we know, this line is header
                else
                    isFirst = false;
            }
            begin = &line[i];
        }
        else if(begin != nullptr && (line[i] == ' ' || line[i] == '\t')){
            outTerms.push_back(std::string_view(begin, &line[i] - begin));
            begin = nullptr;
        }
    }
    if(begin != nullptr)	//add last element
        outTerms.push_back(std::string_view(begin, &line[size-1] - begin));
    return false;
}
