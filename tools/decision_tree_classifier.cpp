#include <decision_tree.h>
#include <names_file_reader.h>
#include <attribute_spec.h>
#include <attribute.h>
#include <vector>
#include <string>
#include <fstream>
#include "dataset.h"
#include "scope.h"
#include "instance.h"
#include "string_utility.h"
#include "expression.h"
#include "decision_tree.h"
using namespace mlplus;
using namespace std;
void bye(int argn, char** args)
{
    cerr <<"ERROR: argument count " << argn << "\n";
    cerr << args[0] << " <examples.names> <examples.mod> < stdin\n";
    cerr << "examples:\n"
         << "\t" << args[0] << " examples.names examples.model < cases\n"
         << "\n"
         << "mail: my email.com\n"
         << "\n";
    exit(1);
}
int main(int argn, char** args)
{
    if (argn < 3) bye(argn, args);
    char* names = args[1];
    char* model = args[2];

    NamesFileReader reader(names);
    AttributeSpec spec(reader);
    ifstream ifs(model);
    BoostDecisionTree tree;
    tree.read(ifs, &spec); 
    string line;
    vector<float> decodeValue;
    decodeValue.reserve(512);
    vector<string> valuelist;
    Scope scope;
    const std::vector<Attribute*>& allAttri = spec.attributesVector();
    const std::vector<Expression*>& expressions = spec.expressionVector();

    float *confidence = new float[tree.numClasses()];
    while(getline(cin, line))
    {
        decodeValue.clear();
        valuelist.clear();
        mlplus::split(line, valuelist, ",");
        if(valuelist.size() != spec.explictAttributeCount())
        {
            cerr << "value size " << valuelist.size() 
                << " attribute count " <<  spec.explictAttributeCount() << endl;
            cout << "?" << endl;
            continue;
        }
        for(unsigned j = 0; j < spec.explictAttributeCount(); ++j)
        {
            if(allAttri[j])
            {
                int index = allAttri[j]->indexOfValue(valuelist[j]);
                if(index < 0)
                {
                    float v = atof(valuelist[j].c_str());
                    decodeValue.push_back(v);
                    scope.add(allAttri[j]->getName(), v);
                }
                else
                {
                    scope.add(allAttri[j]->getName(), valuelist[j]);
                    decodeValue.push_back(index);
                }
            }
        }
        for(unsigned j = 0; j < spec.implictAttributeCount(); ++j)
        {
            decodeValue.push_back(expressions[j]->evaluate(scope));
        }
        DenseInstance instance(decodeValue);
        tree.classify(&instance, &confidence);
        cout << confidence[1] <<"\t"<< valuelist[0] << "\t" << valuelist[1]  << endl;
    }
    delete [] confidence;
}
