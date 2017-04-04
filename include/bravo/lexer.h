#ifndef BRAVO_LEXER_H
#define BRAVO_LEXER_H

//
//  lexer.h
//
//  Created by Mark Elrod on 4/22/2016.

//  This lexer template class scans text for tokens using state transition tables defined in
//  in the tables class. The tables class is specified with a template parameter. For example ...
//
//  lexer<cpp_types> lex("a = b + c");
//
//  int id = 0;
//
//  while ((id = lex.next()) != 0)
//  {
//      std::cout << id << ", " << lex.text << "\n";
//  }
//
//  An important goal for this effort was to keep the lexer code separate from the table definitions so that
//  the same code could be used with different tables. The lexer class is a template and
//  the code is found in lexer.h and lexer.cpp. The table definitions are stored in separte classes in 
//  their own files.
//
//  The class "lexer" uses a number of the techniques found in code generated by Flex. For example
//  negative values are used to indicate that we should exit from the loop in the
//  match() method.
//
//  Comparison of code:
//
//  Flex Lexer generated code               Bravo Lexer code 
//  About 1500 lines of code (w/o tables)   About 500 lines of code (without tables) 
//  Uses goto's                             Uses switch case statements
//  Uses negative states                    Same functionality
//  Reads input into buffer as needed       Same functionality
//  Temporarily terminates text with null   Same functionality
//  Has separate state for null chars       Same functionality
//  get_previous_state() starts at text     previous_state() starts at accepting_state (a little faster)
//  Match loop saves all accepting states   Match loop does not save accepting state for null states (a little faster)
//  yy_get_next_buffer()                    prep_next_buffer() (equivalent method)
//  yy_try_NUL_trans()                      null_trans() (equivalent method)
//  yy_get_previous_state()                 previous_state() (equivalent method)                                        
//
//  Much of the functionality for the code generated by Flex was not needed for our purposes thus
//  the Bravo lexer class has fewer lines of code.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

#include <bravo/lex_buffer.h>
#include <bravo/range_value.h>


#ifdef BRAVO_LIB_BUILD
#define LIB_PUBLIC __declspec(dllexport)
#else
#define LIB_PUBLIC
#endif

#define E_CONTINUE_SCAN   0
#define E_END_OF_FILE     1
#define E_LAST_MATCH      2

#define S_DO_ACTION         1
#define S_STAR_COMMENT      2 // To be removed
#define S_MATCH             3
#define S_FIND_ACTION       4
#define S_INIT              5
#define S_DONE              6

#define MAP_MIN     0
#define MAP_MATCH   1
#define MAP_MAX     2

#define RV_EQ       1
#define RV_LT       2


template <class T>
struct ext_maps_builder
{
    ext_maps_builder()
    {
        ext_maps.resize(T::state_count);
        const typename T::range_point *rp = &T::next_ext[0];
        for (; rp->table; ++rp)
        {
            if (rp->table >= T::state_count)
                break;
            ext_maps[rp->table][rp->key] = { rp->valid, rp->lt_value, rp->eq_value };
        }
    }

    std::vector<std::map<int32_t, range_value>> ext_maps;
};

#define DU_SUCCESS          0
#define DU_INVALID          1
#define DU_NULL_CHAR        2

inline int32_t decode_utf8(char *&c, int &error)
{
    int byte_count = 0;
    int count_mask = 0x80;
    int32_t result = 0;
    
    while(count_mask & *c)
    {
        ++byte_count;
        count_mask >>= 1;
    }
    
    if (byte_count < 2 || byte_count > 4)
    {
        ++c;
        error = DU_INVALID;
        return -1;
    }
    
    switch(byte_count)
    {
        case 2:
            result = (*c & 0x1F) << 6;
            ++c;
            if(!*c)
            {
                ++c;
                error = DU_NULL_CHAR;
                return -1;
            }
            
            result |= (*c & 0x3F);
            ++c;
            
            if (result <= 0x7F)
            {
                error = DU_INVALID;
                return -1;
            }
            
            break;
            
        case 3:
            result = (*c & 0x0F) << 12;
            ++c;
            if(!*c)
            {
                ++c;
                error = DU_NULL_CHAR;
                return -1;
            }
            
            result |= (*c & 0x3F) << 6;
            ++c;
            
            if(!*c)
            {
                ++c;
                error = DU_NULL_CHAR;
                return -1;
            }
            
            result |= (*c & 0x3F);
            ++c;
            
            if (result <= 0x07FF)
            {
                error = DU_INVALID;
                return -1;
            }
            
            break;
            
        case 4:
            result = (*c & 0x07) << 18;
            ++c;
            if(!*c)
            {
                ++c;
                error = DU_NULL_CHAR;
                return -1;
            }
            
            result |= (*c & 0x3F) << 12;
            ++c;
            
            if(!*c)
            {
                ++c;
                error = DU_NULL_CHAR;
                return -1;
            }
            
            result |= (*c & 0x3F) << 6;
            ++c;
            
            if(!*c)
            {
                ++c;
                error = DU_NULL_CHAR;
                return -1;
            }
            
            result |= (*c & 0x3F);
            ++c;
            
            if (result <= 0xFFFF)
            {
                error = DU_INVALID;
                return -1;
            }
            break;
    }
    
    
    error = DU_SUCCESS;
    return result;
}

template<class T>
std::vector<std::map<int32_t, range_value>> &get_ext_maps()
{
    static ext_maps_builder<T> builder;

    return builder.ext_maps;
}

// Lexer template class
template<class T>
class LIB_PUBLIC lexer
{
public:
    enum input_type {txt, filename};
    typedef         T tables;               // Tables class
    std::istream   *stream;                 // Pointer to input stream
    int             accepting_state;        // Last accepting state
    char           *accepting_ptr;          // buf.ptr for last accepting state
    lex_buffer      buf;                    // Buffer for text data to be processed
    char            swap_char;              // Holds the char that is replaced with a null terminator for text
    int             state;                  // Current table state
    int             action;                 // Action to be performed, on table state
    int             loop_state;             // State of loop in next() method.
    int             return_val;             // Return value for next() method.
    bool            in_star_comment;        // To be removed.
    char            prev_comment[3];        // To be removed.
    bool            eof_pending;            // If true, we hit EOF for the input stream. It is possible that there is still
                                            // some remaining text in the buffer which hasn't yet been processed.
    char           *text;
    bool            fill_buffer;
    int             null_state;
    int             null_rule;
    bool            use_other_rule;
    int             max_user_rule;
    int             error;
    std::ifstream   file;
    std::string     fname;
    std::vector<std::map<int32_t, range_value>> &ext_maps;

    // Constructor with a string as input
    lexer(const std::string &s, input_type type = txt) : stream(nullptr), ext_maps(get_ext_maps<T>())
    {
        init();
        if (type == txt)
        {
            set_string(s);
        }
        else
        {
            open(s);
        }
    }

    // Constructor with a stream as input
    lexer(std::istream &in) : stream(&in), buf(BUF_SIZE), ext_maps(get_ext_maps<T>())
    {
        init();
    }

    lexer() : stream(nullptr), buf(BUF_SIZE), ext_maps(get_ext_maps<T>())
    {
        init();
    }
    
    ~lexer()
    {
    }

    int open(const std::string &fname_)
    {
        buf.resize(BUF_SIZE);
        text = buf.text;
        fname = fname_;
        file.open(fname);
        stream = &file;
        
        return 0;
    }

    int set_string(const std::string &str)
    {
        buf.set_string(str);
        text = buf.text;
        fill_buffer = false;
        
        // Init hold_char
        if (str.size())
        {
            swap_char = str[0];
        }
        else
        {
            swap_char = 0;
        }
        
        return 0;
    }

    void init()
    {
        error = 0;
        use_other_rule = tables::null_trans[tables::start_state] != 0;
        null_state = tables::next[tables::start_state][0];
        null_rule = tables::accept[null_state];
        max_user_rule = null_rule - ((use_other_rule)?2:1);
        
        // Init members
        accepting_state     = 0;
        accepting_ptr       = nullptr;
        state               = tables::start_state;
        action              = 0;
        loop_state          = 0;
        return_val          = 0;
        eof_pending         = false;
        swap_char           = 0;
        text                = buf.text;
        fill_buffer         = true;
        // To be removed
        memset(prev_comment, 0, 3);
        in_star_comment     = false;
    }
    
    void hit_null()
    {
        // Handle the null and backup one char
        // buf.ptr points to 1 past the first null
        // Amount of text matched 
        int matched_count = (int)(buf.ptr - (buf.text)) - 1;

        if (buf.ptr <= &buf.buf[buf.count])  
        {
            // This was a real null char. 
            // If this were EOB (End Of Buffer) then buf.ptr would equal &buf.buf[buf.count + 1]
            // Notice we dont call get_next_buffer() when we get a real null in the text stream, 
            // only when we reach an EOB eob

            int next_state;

            // Restore buf.ptr and back up one char
            *buf.ptr = swap_char;
            buf.ptr = buf.text + matched_count; // buf.ptr now points to the first null terminator

            // For speed in the match() method we loop and set current_state directly, we don't keep history
            // such as the previous state. Thus we need to recalculate it here. This is fine for performance because
            // hit_null() is called rarely compared to the number of iterations inside match().
            // Also, to increase the speed, get_previous_state() starts with the accepting_state rather than 
            // the begining of the text pointed to by buf.text. 
            state = previous_state(); 

            // Because we received a real null, we check to see if there is a defined transition for null with the
            // current_state
            next_state = null_trans(state); 

            if (next_state)
            {
                // There was a transition for null, thus move to the next char and continue matching.
                ++buf.ptr;
                state = next_state;
                loop_state = S_MATCH; // To continue trying to match after hitting a null terminator seems like odd behavior  
                return;
            }
            else
            {
                // Now that we have backed up to the previous state before the null, let's reevaluate the action to be performed.
                // find_action() will look up the appropriate action tablesd on the current state.
                loop_state = S_FIND_ACTION;  
            }
        }
        else switch (prep_next_buffer()) // The null we hit was an EOB, thus we need to read more data into the buffer if possible
        {
        case E_END_OF_FILE:
            // Nothing left to do, no pending text, return 0 to stop the main loop calling next()
            return_val = 0;
            loop_state = S_DONE;
            return;

        case E_CONTINUE_SCAN:
            // We've loaded more data into the buffer so continue with match()
            buf.ptr = buf.text + matched_count;
            if (in_star_comment)
            {
                loop_state = S_STAR_COMMENT;
            }
            else
            {
                state = previous_state();
                loop_state = S_MATCH;
            }
            return;

        case E_LAST_MATCH:
            // There was no new data to load into the buffer. Backup to the previous state and call find_action()
            buf.ptr = buf.text + matched_count;
            if (in_star_comment)
            {
                loop_state = S_STAR_COMMENT;
            }
            else
            {
                state = previous_state();
                loop_state = S_FIND_ACTION;
            }
            return;
        }
    }

    void do_action(int act)	
    {
        // Do one of the following:
        // 1) Set return_val to the rule id for the action
        // 2) Read more data into the buffer
        // 3) Handle real null char
        // 3) Return an error

        if (act <= max_user_rule)
        {
            loop_state = S_DONE;
            return_val = tables::rule_to_id[act];
        }
        else if (act == null_rule)
        {
            hit_null();
        }
        else
        {
            return_val = 0;
        }
    }

    inline void find_action()
    {
        // Set action table on the current state. If the current state is not an accepting state,
        // then backup and set the action basted on the last accepting state.
        action = tables::accept[state];

        if (action == 0)
        {            
            if(accepting_ptr && accepting_state != null_state)
            {
                // Need to back up to the last accepting state
                buf.ptr = accepting_ptr + 1;
                state = accepting_state;
                action = tables::accept[state];
            }
            else
            {
                buf.ptr++;
                error = 1;
            }
        }

        // Set hold_char and null terminate the matching text.
        swap_char = *buf.ptr;
        *buf.ptr = '\0';
    }

    inline void set_next_pointers()
    {
        // Set pointers so we can pick up where we left off with each call to next()
        *buf.ptr = swap_char;
        buf.text = buf.ptr;
        text = buf.text;
        state = tables::start_state;
        accepting_ptr = nullptr;
    }

    inline void read_star_comment()
    {
        // Will remove this method later
        const std::string end_comment = "*/";

        while (*buf.ptr)
        {
            prev_comment[1] = *buf.ptr;
            if (end_comment == prev_comment)
            {

                accepting_ptr = buf.ptr;
                ++buf.ptr;
                break;
            }

            prev_comment[0] = prev_comment[1];
            ++buf.ptr;
        }

        if (!*buf.ptr)
        {
            state = null_state; // -tables::next[current_state][0];
            loop_state = S_MATCH;
        }
        else
        {
            prev_comment[0] = 0;
            prev_comment[1] = 0;
            in_star_comment = false;
            swap_char = *buf.ptr;
            *buf.ptr = 0;
            loop_state = S_DONE;
        }
    }
    
    inline void match()
    {
        // Loop through the state transitions tablesd on the current_state and *buf.ptr until there 
        // is no acceptable transition
           
        while(1)
        {
            while ((state = tables::next[state][(unsigned int)(unsigned char)(*buf.ptr)]) > 0)
            {
                if (tables::accept[state])
                {
                    // Capture the last accepting state 
                    accepting_state = state;
                    accepting_ptr = buf.ptr;
                }

                ++buf.ptr;
            }
            
            if (((unsigned char)*buf.ptr) > 0x7F)
            {
                // Encountered a universal character
                char *c = buf.ptr;
                int error = DU_SUCCESS;
                int32_t result = decode_utf8(c,error);
                
                if (error == DU_NULL_CHAR)
                {
                    accepting_state = null_state;
                    accepting_ptr = c - 1; // point to null char
                    state = -null_state;
                    buf.ptr = c; // point to 1 past null char
                }
                else if (error == DU_INVALID)
                {
                    // Already is an error, no action needed here
                }
                else
                {
                    auto it = ext_maps[-state].lower_bound(result);
                    if (it != ext_maps[-state].end())
                    {
                        if (it->first == result)
                        {
                            if(it->second.valid & RV_EQ)
                            {
                                state = it->second.eq_value;
                                if(state > 0)
                                {
                                    if (tables::accept[state])
                                    {
                                        // Capture the last accepting state
                                        accepting_state = state;
                                        accepting_ptr = buf.ptr;
                                    }

                                    buf.ptr = c;
                                    continue;
                                }
                            }
                        }
                        else
                        if (it->second.valid & RV_LT)
                        {
                            state = it->second.lt_value;
                            if (state > 0)
                            {
                                if (tables::accept[state])
                                {
                                    // Capture the last accepting state
                                    accepting_state = state;
                                    accepting_ptr = buf.ptr;
                                }

                                buf.ptr = c;
                                continue;
                            }
                        }
                    }
                }
            }

            break;
        }

        // We use negative values to indicate when we should exit the loop above. By negating the current state here we
        // we restore the value back to the state which encountered the offending char.
        state = -state;
    }

    inline int next()
    {
        // Get the next token
        loop_state = S_INIT;

        while (loop_state != S_DONE)
        {
            switch (loop_state)
            {
            case S_INIT:
                set_next_pointers(); // Set pointers so we can pick up where we left off
                // fall through

            case S_MATCH: // Loop through the state transitions until there is no acceptable transition
                match();
                // fall through

            case S_FIND_ACTION:
                // Set the action table on the current state. If the current state is not an accepting state, then backup one character
                // and set the action tablesd on the last accepting state.
                // Set hold_char
                find_action();
                // fall through

            case S_DO_ACTION:
                // Do one of the following:
                // 1) Set return_val to the rule id that coresponds to the action
                // 2) Read more data into the buffer
                // 3) Handle a real null character
                // 4) Return an error
                do_action(action);
                break;

            case S_STAR_COMMENT:
                // Will be removed
                read_star_comment();
                break;

            default:
                break;
            }
        }

        text = buf.text;
 //       swap_char = *buf.ptr;
 //       *buf.ptr = 0;

        return return_val;
    }

    int prep_next_buffer()
    {
        //  This method is called when there is no more text to process in the buffer and 
        //  we need to read more text from the stream into the buffer.
        //
        //  Return codes:
        //
        //  E_LAST_MATCH      We encounted the EOF of the stream, but there is still text in the buffer that
        //                    needs to be processed.
        //	E_CONTINUE_SCAN   We read new data into the buffer, continue scanning
        //	E_END_OF_FILE     We are at the end of the file, return 0 to signal the end of the main loop that calls next()
   
        // First, if the buffer is not allocated, we allocate it.
        if (!buf.buf.size())
        {
            buf.resize(BUF_SIZE);
        }

        // Init vars
        char    *dst         = &buf.buf[0];
        char    *src         = buf.text;

        // buf.ptr points to the second EOB null char so we need to subtract an extra char to get 
        // the actually count of characters that are ready.    
        int ready_count = (int)(buf.ptr - buf.text) - 1;

        // If we should not attempt to read from the stream 
        if (!fill_buffer || eof_pending)
        {
            if (ready_count)
            {
                // We encountered the stream EOF, however there is still some pending text that we should process
                return E_LAST_MATCH;
            }
            else
            {
                // We are at EOF now, processing should stop
                return E_END_OF_FILE;
            }
        }

        // If we reach this point we know that we are supposed to try to read more data into the buffer and that we 
        // haven't yet encountered EOF.

        // Move data from the back of the buffer to the front if necessary
        if (buf.count > ready_count)
        {
            int delta = (int)buf.count - ready_count;

            for (int i = 0; i < ready_count; ++i)
            {
                *(dst++) = *(src++);
            }

            buf.count = ready_count;
            buf.text -= delta;
            text = buf.text;
            buf.ptr -= delta;
            buf.buf[buf.count] = 0;
            buf.buf[buf.count + 1] = 0;
        }      
        
        // If the buffer has no room to read in new text, resize the buffer making it bigger.
        if (buf.count == buf.buf.size() - 2)
        {
            size_t new_size = buf.count + LEX_BUF_INCREMENT;
            buf.resize(new_size);
        }

        // Calculate the number of characters we should attempt to read
        size_t num_to_read = buf.buf.size() - 2 - ready_count;

        char *buf2 = &buf.buf[ready_count];

        size_t read_count = 0;

        if (stream && num_to_read)
        {
            // Read new characters into buffer
            stream->read(buf2, num_to_read);
            read_count = stream->gcount();
        }

        int rc = 0;

        // Set the return value tablesd on the counts
        if (read_count == 0)
        {
            // We read nothing thus we reached EOF
            eof_pending = true;

            if (ready_count)
            {
                rc = E_LAST_MATCH;
            }
            else
            {
                rc = E_END_OF_FILE;
            }
        }
        else
        {
            buf.count += read_count;
            rc = E_CONTINUE_SCAN;
        }

        // Terminate the buffer with EOB null characters
        buf.buf[buf.count] = 0;
        buf.buf[buf.count + 1] = 0;

        return rc;
    }

    int previous_state()
    {
        // Get the state before we hit the null
        int s = tables::start_state;
        char *cp = buf.text;

        int char_size = 1;
        for (; cp < (buf.ptr); cp += char_size)
        {
            if (*cp)
            {
                if (((unsigned char)*buf.ptr) <= 0x7F)
                {
                    s = tables::next[s][(unsigned int)(unsigned char)(*cp)];
                    char_size = 1;
                }
                else
                {
                    // Encountered a universal character
                    char *c = cp;
                    int error = DU_SUCCESS;
                    int32_t result = decode_utf8(c,error);
                    char_size = (int)(c - cp);
                    
                    if (c > buf.ptr)
                    {
                        buf.ptr = cp;
                        continue;
                    }
                    
                    if (error == DU_NULL_CHAR)
                    {
                        s = -null_state;
                    }
                    else if (error == DU_INVALID)
                    {
                        s = -s;
                    }
                    else
                    {
                        auto it = ext_maps[-state].lower_bound(result);
                        if (it != ext_maps[-state].end())
                        {
                            if (it->first == result)
                            {
                                if(it->second.valid & RV_EQ)
                                {
                                    s = it->second.eq_value;
                                }
                            }
                            else if (it->second.valid & RV_LT)
                            {
                                s = it->second.lt_value;
                            }
                        }
                    }
                }
            }
            else
            {
                s = tables::null_trans[s];
            }

            if (tables::accept[s]) // && tables::accept[state] <= tables::rule_count)
            {
                // Capture the last accepting state 
                accepting_state = s;
                accepting_ptr = cp;
            }
        }
    
        return s;
    }

    int null_trans(int s)
    {
        s = tables::null_trans[s];

        if (s)
        {
            if (tables::accept[s]) // && tables::accept[s] <= tables::rule_count)
            {
                accepting_state = s;
                accepting_ptr = buf.ptr;
            }
        }

        return s;
    }
};

/*
ext_maps[1][0xF900] = range_value(1, 0, 11);
ext_maps[1][0xFD3D] = range_value(3, 11, 11);

ext_maps[11][0xF900] = range_value(1, 0, 33);
ext_maps[11][0xFD3D] = range_value(3, 33, 33);

ext_maps[33][0xF900] = range_value(1, 0, 33);
ext_maps[33][0xFD3D] = range_value(3, 33, 33);

*/

/*
template<class T>
range_point lexer<T>::range_points[7] = {
    { 1, 0xF900, {1,  0, 11} },
    { 1, 0xFD3D, {3, 11, 11} },

    { 11, 0xF900, {1,  0, 33} },
    { 11, 0xFD3D, {3, 33, 33} },

    { 33, 0xF900, {1,  0, 33} },
    { 33, 0xFD3D, {3, 33, 33} },

    {  0, 0, {0, 0, 0} }
};
 */



#endif
