#include <cctype>
#include <iostream>
#include <string>
#include <stdexcept>
#include <streambuf>
#include "attribute.h"
#include "instance.h"
#include "dataset.h"
#include "arff_loader.h"
namespace mlplus
{
using namespace std;

string ArffLoader::Token::toString()
{
    string s;
    switch(kind)
    {
    case ERR:
        s.append("<ERROR>");
        break;
    case COMMA:
        s.append("','");
        break;
    case LBRACE:
        s.append("'{'");
        break;
    case RBRACE:
        s.append("'}'");
        break;
    case LBRACK:
        s.append("'['");
        break;
    case RBRACK:
        s.append("']'");
        break;
    case LPAREN:
        s.append("'('");
        break;
    case RPAREN:
        s.append("')'");
        break;
    case IDENT:
        s.append("'");
        s.append(text);
        s.append("'");
        break;
    case NUMBER:
        s.append(text);
        break;
    case STRING:
        s.append("\"");
        s.append(text);
        s.append("\"");
        break;
    case ATTRIBUTE:
        s.append("'@attribute'");
        break;
    case DATA:
        s.append("'@data'");
        break;
    case RELATION:
        s.append("'@relation'");
        break;
    }
    return s;
}

ArffLoader::Token ArffLoader::getNextToken()
{
    ArffLoader::Token token;
    // skip whitespace
again:
    int c = mStream->sgetc();
    while(isspace(c))
    {
        if(c == '\n')
            mLine++;
        c = mStream->snextc();
    }
    // skip comments
    if(c == '%')
    {
        while((c != '\n') && (c != EOF))
            c = mStream->snextc();
        mLine++;
        goto again;
    }
    // process next token
    char buf[2];
    buf[1] = '\0';
    switch(c)
    {
    case '{':
        token.kind = ArffLoader::Token::LBRACE;
        mStream->sbumpc();
        goto out;
    case '}':
        token.kind = ArffLoader::Token::RBRACE;
        mStream->sbumpc();
        goto out;
    case '[':
        token.kind = ArffLoader::Token::LBRACK;
        mStream->sbumpc();
        goto out;
    case ']':
        token.kind = ArffLoader::Token::RBRACK;
        mStream->sbumpc();
        goto out;
    case '(':
        token.kind = ArffLoader::Token::LPAREN;
        mStream->sbumpc();
        goto out;
    case ')':
        token.kind = ArffLoader::Token::RPAREN;
        mStream->sbumpc();
        goto out;
    case ',':
        token.kind = ArffLoader::Token::COMMA;
        mStream->sbumpc();
        goto out;
    case '"':
        while(true)
        {
            c = mStream->snextc();
            if(c == '"')
            {
                mStream->sbumpc();
                token.kind = ArffLoader::Token::STRING;
                goto out;
            }
            if(c == EOF)
                goto error;
            buf[0] = c;
            token.text.append(buf);
        }
        break;
    case '\'':
        while(true)
        {
            c = mStream->snextc();
            if(c == '\'')
            {
                mStream->sbumpc();
                token.kind = ArffLoader::Token::STRING;
                goto out;
            }
            if(c == EOF)
                goto error;
            buf[0] = c;
            token.text.append(buf);
        }
        break;
    case '@':
        switch(tolower(mStream->snextc()))
        {
        case 'a':
            if((tolower(mStream->snextc()) == 't') &&
                    (tolower(mStream->snextc()) == 't') &&
                    (tolower(mStream->snextc()) == 'r') &&
                    (tolower(mStream->snextc()) == 'i') &&
                    (tolower(mStream->snextc()) == 'b') &&
                    (tolower(mStream->snextc()) == 'u') &&
                    (tolower(mStream->snextc()) == 't') &&
                    (tolower(mStream->snextc()) == 'e') &&
                    (isspace(mStream->snextc())))
            {
                token.kind = ArffLoader::Token::ATTRIBUTE;
                goto out;
            }
            break;
        case 'd':
            if((tolower(mStream->snextc()) == 'a') &&
                    (tolower(mStream->snextc()) == 't') &&
                    (tolower(mStream->snextc()) == 'a') &&
                    (isspace(mStream->snextc())))
            {
                token.kind = ArffLoader::Token::DATA;
                goto out;
            }
            break;
        case 'r':
            if((tolower(mStream->snextc()) == 'e') &&
                    (tolower(mStream->snextc()) == 'l') &&
                    (tolower(mStream->snextc()) == 'a') &&
                    (tolower(mStream->snextc()) == 't') &&
                    (tolower(mStream->snextc()) == 'i') &&
                    (tolower(mStream->snextc()) == 'o') &&
                    (tolower(mStream->snextc()) == 'n') &&
                    (isspace(mStream->snextc())))
            {
                token.kind = ArffLoader::Token::RELATION;
                goto out;
            }
            break;
        default:
            goto error;
        }
        break;

    case 'i':
    {
        int next = mStream->snextc();
        if((tolower(next) == 'n') && (tolower(mStream->snextc()) == 'f'))
        {
            token.text.append("inf");
            mStream->sbumpc();
            token.kind = ArffLoader::Token::NUMBER;
            goto out;
        }
        else
        {
            mStream->sputbackc(next);
            goto ident;
        }
    }
    break;

    case '+':
    case '-':
    {
        int next = mStream->snextc();
        if(tolower(next) == 'i')
        {
            if((tolower(mStream->snextc()) == 'n') &&
                    (tolower(mStream->snextc()) == 'f'))
            {
                buf[0] = c;
                token.text.append(buf);
                token.text.append("inf");
                mStream->sbumpc();
                token.kind = ArffLoader::Token::NUMBER;
                goto out;
            }
            else
            {
                goto error;
            }
        }
        else
        {
            mStream->sputbackc(next);
        }
    }
    // fallthrough
    case '.':
    case '0' ... '9':
        buf[0] = c;
        token.text.append(buf);
        while(true)
        {
            c = mStream->snextc();
            switch(c)
            {
            case '+':
            case '-':
            case '.':
            case '0' ... '9':
            case 'e':
            case 'E':
                buf[0] = c;
                token.text.append(buf);
                break;
            default:
                token.kind = ArffLoader::Token::NUMBER;
                goto out;
            }
        }
        break;

    case '_':
    case 'A' ... 'Z':
    case 'a' ... 'h':
    case 'j' ... 'z':
ident:
        buf[0] = c;
        token.text.append(buf);
        while(true)
        {
            c = mStream->snextc();
            switch(c)
            {
            case '_':
            case '-':
            case '0' ... '9':
            case 'A' ... 'Z':
            case 'a' ... 'z':
                buf[0] = c;
                token.text.append(buf);
                break;
            default:
                token.kind = ArffLoader::Token::IDENT;
                goto out;
            }
        }
        break;

    default:
        goto error;

    }

out:
    return token;

error:
    token.kind = ArffLoader::Token::ERR;
    return token;
}

void ArffLoader::readHeader()
{
    if(mPhase != IN_HEADER)
        return;
    Attribute* attr = NULL;
    Token token = getNextToken();
    VectorAttributeContainer* attributes = new VectorAttributeContainer();

    while(token.kind != Token::DATA)
    {
        switch(token.kind)
        {
        case Token::RELATION:
        {
            token = getNextToken();
            if(token.kind != Token::IDENT)
            {
                cerr << __FUNCTION__ << "; unexpected " << token.toString()
                     << " after @RELATION\n";
                mPhase = IN_ERR;
                return;
            }
            if(mRelationName.size() > 0)
            {
                cerr << __FUNCTION__ << ": @RELATION already seen\n";
                mPhase = IN_ERR;
                return;
            }
            mRelationName = token.text;
        }
        break;

        case Token::ATTRIBUTE:
        {
            string name;

            attr = NULL;

            token = getNextToken();
            if(token.kind != Token::IDENT)
                goto attr_unexpected;
            name = token.text;
            token = getNextToken();

            switch(token.kind)
            {

            case Token::IDENT:
            {
                for(int i = 0; i < (int)token.text.size(); i++)
                    token.text[i] = tolower(token.text[i]);
                if(token.text == "numeric" || token.text == "real" ||
                        token.text == "integer")
                {
                    attr = new Attribute(name, Attribute::NUMERIC);
                    attr->setIndex(attributes.size());
                    attributes.push_back(attr);
                }
                else if(token.text == "string")
                {
                    attr = new Attribute(name, Attribute::STRING);
                    attr->setIndex(attributes.size());
                    attributes.push_back(attr);
                }
                else
                {
                    cerr << __FUNCTION__ << ": unknown data type '"
                         << token.text << "' in @ATTRIBUTE\n";
                    mPhase = IN_ERR;
                    return;
                }

                token = getNextToken();
                if(token.kind == Token::LBRACK ||
                        token.kind == Token::LPAREN)
                {
                    attr->getRange().mLowerBoundIsOpen = (token.kind == Token::LPAREN);
                    token = getNextToken();
                    if(token.kind != Token::NUMBER)
                    {
bad_range:
                        cerr << __FUNCTION__
                             << ": bad range for numeric attribute\n";
                        mPhase = IN_ERR;
                        return;
                    }
                    if(token.text == "-inf")
                    {
                        attr->getRange().mLowerBound = -INFINITY;
                    }
                    else
                    {
                        attr->getRange().mLowerBound =
                            strtod(token.text.c_str(), NULL);
                    }
                    token = getNextToken();
                    if(token.kind != Token::COMMA)
                        goto bad_range;
                    token = getNextToken();
                    if(token.kind != Token::NUMBER)
                        goto bad_range;
                    if(token.text == "inf")
                    {
                        attr->getRange().mUpperBound = INFINITY;
                    }
                    else
                    {
                        attr->getRange().mUpperBound =
                            strtod(token.text.c_str(), NULL);
                    }
                    token = getNextToken();
                    if((token.kind != Token::RBRACK) &&
                            (token.kind != Token::RPAREN))
                        goto bad_range;
                    attr->getRange().mUpperBoundIsOpen =
                        (token.kind == Token::RPAREN);
                }
                else
                {
                    continue;
                }
            }
            break;

            case Token::LBRACE:
            {
                attr = new Attribute(name);
                attr->setIndex(attributes.size());
                attributes.push_back(attr);
                token = getNextToken();
                while(token.kind != Token::RBRACE)
                {
                    switch(token.kind)
                    {
                    case Token::IDENT:
                    case Token::NUMBER:
                    case Token::STRING:
                        attr->addValue(token.text);
                        break;
                    case Token::COMMA:
                        break;
                    default:
                        goto attr_unexpected;
                    }
                    token = getNextToken();
                }
            }
            break;

            default:
attr_unexpected:
                cerr << __FUNCTION__ << ": unexpected " << token.toString()
                     << " after @ATTRIBUTE\n";
                mPhase = IN_ERR;
                return;
            }
        }
        break;

        case Token::DATA:
            break;

        default:
            cerr << __FUNCTION__ << ": unexpected " << token.toString()
                 << "\n";
            mPhase = IN_ERR;
            return;
        }

        token = getNextToken();
    }

    mPhase = IN_DATA;
    mbHeaderRead = true;
    DenseInstanceContainer* instances = new DenseInstanceContainer();
    mHeader = new DataSet(mRelationName, attributes, instances);
    mHeader->setClassIndex(attributes.size() - 1);
}

ArffLoader::ArffLoader(streambuf * s)
{
    mStream = s;
    mbHeaderRead = false;
    mPhase = IN_HEADER;
    mHeader = NULL;
    mLine = 1;
}

ArffLoader::~ArffLoader()
{
    // delete header, if any
    if(mHeader != NULL)
        delete mHeader;
}

DataSet* ArffLoader::getStructure()
{
    if(!mbHeaderRead)
        readHeader();
    return mHeader;
}

DataSet* ArffLoader::getDataSet()
{
    if(!mbHeaderRead)
        readHeader();
    IInstanceContainer* instances = mHeader->getInstanceContainer();
    IInstance* instance;
    while((instance = getNextInstance()) != NULL)
        instances->add(instance);
    return mHeader;
}

IInstance * ArffLoader::getNextInstance()
{
    if(!mbHeaderRead)
        readHeader();

    if(mPhase != IN_DATA)
        return NULL;

    vector<float> values(mHeader->numAttributes());
    int i = 0;
    while(i < mHeader->numAttributes())
    {
        Token token = getNextToken();

        if(token.kind == Token::ERR)
            return NULL;

        if(token.kind == Token::COMMA)
            continue;

        Attribute * attr = mHeader->attribute(i);

        switch(attr->type())
        {

        case Attribute::NUMERIC:
        {
            switch(token.kind)
            {
            case Token::NUMBER:
            {
                float v;
                sscanf(token.text.c_str(), "%f", &v);
                values[i] = v;
            }
            break;
            default:
                cerr << __FUNCTION__ << ": attribute at index " << i
                     << " is not numeric\n";
                mPhase = IN_ERR;
                return NULL;
            }
        }
        break;

        case Attribute::NOMINAL:
        {
            switch(token.kind)
            {
            case Token::IDENT:
            case Token::STRING:
                values[i] = attr->indexOfValue(token.text);
                break;
            default:
                cerr << __FUNCTION__ << ": attribute at index " << i
                     << " is not nominal\n";
                mPhase = IN_ERR;
                return NULL;
            }
        }
        break;

        case Attribute::STRING:
        {
            switch(token.kind)
            {
            case Token::STRING:
                values[i] = attr->indexOfValue(token.text);
                break;
            default:
                cerr << __FUNCTION__ << ": attribute at index " << i
                     << " is not a string\n";
                mPhase = IN_ERR;
                return NULL;
            }
        }
        break;

        }

        i++;
    }
    IInstance* instance = new  DenseInstance(values);
    instance->setDataset(mHeader);
    return instance;
}

} // namespace mlplus
