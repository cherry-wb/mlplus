#include "data_file_reader.h"
#include "string_utility.h"
#include <fstream>
#include <iostream>
using namespace mlplus;
using namespace std;

void  DataFileReader::read(const std::string& filename)
{
    ifstream ifs(filename.c_str());
    read(ifs);
}
void DataFileReader::read(istream& istr)
{
    string line;
    while(std::getline(istr, line))
    {
        parseLine(line);
    }
}
void DataFileReader::parseLine(const string& line)
{
}
