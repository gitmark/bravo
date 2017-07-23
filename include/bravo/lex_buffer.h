#ifndef BRAVO_LEX_BUFFER_H
#define BRAVO_LEX_BUFFER_H

#include <vector>

#define BUF_SIZE 2
#define LEX_BUF_INCREMENT 2

struct lex_buffer
{
    lex_buffer(const std::string &str)
        : text(nullptr),
        ptr(nullptr),
        count(0)
    {
        set_string(str);
    }

    lex_buffer(size_t s = 0)
        : text(nullptr),
        ptr(nullptr),
        count(0)
    {
        if (s)
            resize(s);
    }

    ~lex_buffer()
    {
        clear();
    }

    std::vector<char> buf;	// Character buffer
    char    *text;          // Pointer to the beginning of the text match
    char    *ptr;		    // Current pointer in the buffer, end of match
    size_t  count;          // Number of chars read into buffer, doesnt include 2 EOB null chars

    void set_string(const std::string &str)
    {
        size_t s = str.size();
        
        if (s)
        {
            resize(s);                      // Allocating buffer size to equal string size
            memcpy(&buf[0], str.c_str(), s);    // Reading data into buffer
            count = s;
            buf[count] = 0;
            buf[count + 1] = 0;
            // The entire string is read into the buffer. There is nothing remaining.
        }
    }

    void clear()
    {
        buf.clear();
        text    = nullptr;
        ptr     = nullptr;
        count   = 0;
    }

    void flush()
    {
        if (!buf.size())
        {
            return;
        }

        // Not we aren't setting size here, we are leaving the existing mem allocation as it is.
        ptr     = &buf[0];
        text    = &buf[0];
        count   = 0;
        buf[0]  = 0;
        buf[1]  = 0;
    }

    void resize(size_t n)
    {
        if (n + 2 <= buf.size())
        {
            return;
        }

        size_t prev_count   = count;

        int ptr_index = 0;
        int text_index = 0;

        if (buf.size())
        {
            ptr_index = (int)(ptr - &buf[0]);
            text_index = (int)(text - &buf[0]);
        }

        buf.resize(n + 2);

        ptr     = &buf[0] + ptr_index;
        text    = &buf[0] + text_index;
        count   = prev_count;
        buf[count]      = 0;
        buf[count + 1]  = 0;
    }
};


#endif

