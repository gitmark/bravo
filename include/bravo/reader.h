#ifndef READER_H
#define READER_H

#include <string>
#include <iostream>

class reader
{
public:
    reader() {}
    virtual ~reader() {}
    virtual char next_char() = 0;
    virtual void putback(char ch) = 0;
    virtual bool eof() = 0;
};

class file_reader : public reader
{
public:
    file_reader(std::istream *is_) : is(is_), bom_read(false)  
    {
        read_bom();
    }

    virtual void putback(char ch)
    {
        is->putback(ch);
    }

    void read_bom()
    {
        if(bom_read)
            return;

        if (!is)
            return;

        bom_read = true;
        char c;

        if (is->eof())
            return;
        
        is->read(&c, 1);
        if (is->gcount() != 1)
            return;

        if (c == (char)0xEF)
        {
            if (is->eof())
                return;
            
            is->read(&c, 1);
            if (is->gcount() != 1)
                return;
            
            if (c == (char)0xBB)
            {
                is->read(&c, 1);
                if (is->gcount() != 1)
                    return;
                
                if (c == (char)0xBF)
                {
                    is->read(&c, 1);
                    if (is->gcount() != 1)
                        return;                        
                }
                else 
                    return;
            }
            else
                return;
        }
        else
            is->putback(c);        
    }
    
    virtual char next_char()
    {        
        if (!is)
            return 0;

        if (is->eof())
            return 0;


        char c;
        is->read(&c, 1);
        if (is->gcount() != 1)
            return 0;

        
        return c;
    }

    virtual bool eof()
    {
        if (!is)
            return true;

        return is->eof();
    }

    bool bom_read;
    std::istream *is;
};

class string_reader : public reader
{
public:
    string_reader(std::string *s)
        : str(s), index(0)
    {
        if (!str)
            eof_ = true;

        if (index >= str->size())
            eof_ = true;

        eof_ = false;
    }


    virtual char next_char()
    {
        if (!str)
        {
            eof_ = true;
            return 0;
        }

        if (index >= str->size())
        {
            eof_ = true;
            return 0;
        }

        char c = str->at(index++);

        if (index >= str->size())
        {
            eof_ = true;
        }

        return c;
    }

    virtual bool eof()
    {
        return eof_;
    }

    bool eof_;
    size_t index;
    std::string *str;
};


#endif // READER_H
