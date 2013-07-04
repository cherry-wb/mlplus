#include <fstream>
#include <iostream>
#include <algorithm> 
#include <vector>
#include "pattern_scan.h"
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#define STRIP_FLAG_HELP 1
#include <gflags/gflags.h>
using namespace std;
using namespace boost;
DEFINE_string(feature, "", "feature file name");
DEFINE_string(input, "", "input file name");
struct WordInfo
{
  float weight;
  int id;
  friend bool operator < (const WordInfo& l, const WordInfo& r)
  {
      return l.id < r.id;
  }
  friend bool operator > (const WordInfo& l, const WordInfo& r)
  {
      return l.id > r.id;
  }
};
bool load(const string& filename, vector<string>& features,
    unordered_map<string, WordInfo>& featureWeight)
{
    ifstream ifs(filename.c_str());
    string line;
    vector<string> keys;
    int wordid = 1;
    while(getline(ifs, line))
    {
        boost::split(keys, line, boost::is_any_of("\t "));
        string& word = keys[0]; 
        float weight = 1;
        if (keys.size() > 1)
        {
            weight = boost::lexical_cast<float>(keys[1]);
        }
        WordInfo info;
        info.weight = weight;
        info.id = wordid++;
        features.push_back(word); 
        featureWeight[word] = info;
    }
    return true;
}
int main(int argc, char** argv)
{
    //FLAGS_feature;
    //FLAGS_input;
    google::SetVersionString("1.0.0");
    string usage("This program allows bulk text comparison.  Sample usage:\n"
        "program --input input --feature feature\n");
    google::SetUsageMessage(usage);
    google::ParseCommandLineFlags(&argc, &argv, true);
    if (FLAGS_feature.empty() || FLAGS_input.empty())
    {
        cerr << usage << endl;
        exit(0);
    }
    //FLAGS_input;
    vector<string> features;
    unordered_map<string, WordInfo> weighted;
    load(FLAGS_feature, features, weighted);
    PatternScan scanner;
    scanner.Init(features);

    ifstream is(FLAGS_input.c_str());
    string line;
    int linen = 0;
    std::vector<WordInfo> wfeature;
    wfeature.reserve(10000);
    while(getline(is, line))
    {
        wfeature.clear();
        std::map<std::string, uint32_t> matches;
        scanner.Scan(line, matches);
		pair<string,size_t> record;
        cout << ++linen << "\t"; 
        BOOST_FOREACH(record, matches)
        {
            WordInfo info = weighted[record.first];
            info.weight = record.second * info.weight;
            wfeature.push_back(info);
        }
        std::sort (wfeature.begin(), wfeature.end()); 
        WordInfo info;
        BOOST_FOREACH(info, wfeature)
        {
            cout << info.id << ":" << info.weight << " ";
        }
        cout << endl;
    }
    return 0;
}
