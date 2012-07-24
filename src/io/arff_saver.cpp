#include "attribute.h"
#include "Instance.h"
#include "dataset.h"
#include "arff_saver.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <streambuf>

namespace mlplus
{
using namespace std;

void ArffSaver::writeHeader(Instances * header)
{
    mStream->sputn("@relation ", 10);
    string relation = header->relationName();
    mStream->sputn(relation.c_str(), relation.size());
    mStream->sputn("\n\n", 2);
    for(int i = 0; i < header->numAttributes(); i++)
    {
        mStream->sputn("@attribute ", 11);

        Attribute * attr = header->attribute(i);

        switch(attr->type())
        {

        case Attribute::NUMERIC:
        {
            string name = attr->name();
            mStream->sputn(name.c_str(), name.size());
            mStream->sputn(" numeric", 8);
            if((((NumericAttribute *)attr)->getLowerBound() != NAN &&
                    ((NumericAttribute *)attr)->getLowerBound() != -INFINITY) &&
                    (((NumericAttribute *)attr)->getUpperBound() != NAN &&
                     ((NumericAttribute *)attr)->getUpperBound() != INFINITY))
            {
                char buf[40];
                mStream->sputc(' ');
                if(((NumericAttribute *)attr)->lowerBoundIsOpen())
                    mStream->sputc('(');
                else
                    mStream->sputc('[');
                sprintf(buf, "%lf",
                        ((NumericAttribute *)attr)->getLowerBound());
                mStream->sputn(buf, strlen(buf));
                mStream->sputc(',');
                sprintf(buf, "%lf",
                        ((NumericAttribute *)attr)->getUpperBound());
                mStream->sputn(buf, strlen(buf));
                if(((NumericAttribute *)attr)->upperBoundIsOpen())
                    mStream->sputc(')');
                else
                    mStream->sputc(']');
            }
        }
        break;

        case Attribute::NOMINAL:
        {
            string name = attr->name();
            bool first = true;
            mStream->sputn(name.c_str(), name.size());
            mStream->sputn(" {", 2);
            for(int j = 0; j < attr->numValues(); j++)
            {
                if(!first)
                {
                    mStream->sputc(',');
                }
                else
                {
                    first = false;
                }
                string val = attr->value(j);
                mStream->sputn(val.c_str(), val.size());
            }
            mStream->sputn("}", 1);
        }
        break;

        case Attribute::STRING:
        {
            string name = attr->name();
            mStream->sputn(name.c_str(), name.size());
            mStream->sputn(" string", 7);
        }
        break;

        default:
            cerr << __FUNCTION__ << ": unhandled attribute (" << i << ")\n";
            return;
        }

        mStream->sputc('\n');
    }
    mStream->sputn("\n@data\n\n", 8);
    mbHeaderWritten = true;
}

ArffSaver::ArffSaver(streambuf * s)
{
    mStream = s;
}

ArffSaver::~ArffSaver()
{
}

void
ArffSaver::writeInstance(Instance * instance)
{
    for(int i = 0; i < instance->numAttributes(); i++)
    {
        Attribute * attr = instance->attribute(i);
        if(i != 0)
            mStream->sputc(',');

        switch(attr->type())
        {

        case Attribute::NUMERIC:
        {
            char buf[40];
            sprintf(buf, "%lf", instance->value(i));
            mStream->sputn(buf, strlen(buf));
        }
        break;

        case Attribute::NOMINAL:
        {
            string val = attr->value((int)instance->value(i));
            mStream->sputn(val.c_str(), val.size());
        }
        break;

        case Attribute::STRING:
        {
            string val = attr->value((int)instance->value(i));
            mStream->sputc('"');
            mStream->sputn(val.c_str(), val.size());
            mStream->sputc('"');
        }
        break;

        }
    }

    mStream->sputc('\n');
}

void ArffSaver::writeIncremental(Instance * instance)
{
    if(!mbHeaderWritten)
        writeHeader(instance->dataset());
    writeInstance(instance);
}

void ArffSaver::writeBatch(Instances * instances)
{
    if(!mbHeaderWritten)
        writeHeader(instances);
    for(int i = 0; i < instances->numInstances(); i++)
        writeInstance(instances->instance(i));
}

} // namespace mlplus
