#include "bayes_message_passing.h"
#include "dataset.h"
#include "attribute_container.h"
#include "instance_container.h"
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <iterator>
using namespace std;
using namespace boost;
namespace po = boost::program_options;
using namespace mlplus;
using namespace mlplus::estimators;

int main(int argn, char** args)
{
    string input_data;
    string model_file;
    bool istrain = false;
    po::options_description desc("Allowed options for [bayes_message_passing]");
    desc.add_options()("help,h", "message:")
        ("train,t", "train or classify")
        ("input_data,i", po::value<string>(&input_data), "trainning or classify data")
        ("model_file,m", po::value<string>(&model_file), "model file name");
    po::variables_map vm;
    po::store(po::parse_command_line(argn, args, desc), vm);
    po::notify(vm); 
    if (vm.count("help"))
    {
        cout << desc << "\n";
        return 1;
    }
    istrain =  (bool)vm.count("train");
    if (!istrain && (input_data.empty() || model_file.empty()))
    {
        cout << "for classify\n";
        cout << desc << "\n";
        return 1;
    }
    if (istrain && input_data.empty())//for trainning
    {
        cout << "for train\n";
        cout << desc << "\n";
        return 1;
    }
    string str;
    ifstream ifs(input_data.c_str());
    IAttributeContainer* attributes = new MapAttributeContainer();
    IInstanceContainer* instances = new SparseInstanceContainer();
    DataSet dataset("sparse_classify", attributes, instances);
    vector<string> attributeVector;
    IInstance* instance = NULL;
    while(getline(ifs, str))
    {
        split(attributeVector, str, is_any_of("\t "),token_compress_on);
        vector<ValueType> values;
        vector<int> indices;
        for(unsigned i = 0; i < attributeVector.size(); ++i)
        {
            if (attributeVector[i].empty()) continue;
            vector<string> kvs;
            split(kvs, attributeVector[i], is_any_of(":"));
            Attribute* target = new Attribute(kvs[0], Attribute::BINARY);//name of attribute
            int idcs = 0;
            ValueType value = 0;
            if (i != 0)
            {
                idcs = boost::lexical_cast<int, string>(kvs[0]) - 1;
                value = boost::lexical_cast<ValueType, string>(kvs.back());
            }
            else
            {
                value = boost::lexical_cast<ValueType, string>(kvs.back()) - 1;
            }
            target->setIndex(idcs);
            attributes->add(target);
            indices.push_back(idcs);
            values.push_back(value);
        }
        instance = new SparseInstance(values, indices, 1);
        instance->setDataset(&dataset);
        instances->add(instance);
    }
    dataset.setTargetIndex(0);
    BayesMsgPassing bayes("sparse_classify", 6);

    if (istrain) //trainning
    {
        bayes.train(&dataset);
        bayes.save(cout);
    }
    else
    {
        ifstream model(model_file.c_str());
        bayes.load(model);
        AutoInstanceIteratorPtr instanceIt(dataset.newInstanceIterator());
        float all = 0;
        float right = 0;
        while(instanceIt->hasMore())
        {
            IInstance* ins = instanceIt->next();
            pair<int, double> v = bayes.predict(ins);
            if (v.first == ins->targetValue())
            {
                ++right;
            }
            ++all;
            cout << v.first << endl;
        }
        cerr << "Accurate:" << right/all << endl;
    }
}
