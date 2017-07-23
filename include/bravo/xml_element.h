#ifndef XML_ELEMENT_H
#define XML_ELEMENT_H

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <memory>

class xml_element;

class xml_element
{
public:
    xml_element *parent;
    std::string tag;
    std::string text;
    std::map<std::string, std::string> attributes;
    bool closing;
    bool prolog;
    bool cdata;
    bool comment;
    bool dtd;
    std::vector<std::unique_ptr<xml_element>> children_;
    
    xml_element()
    {
        parent = nullptr;
        closing = false;
        prolog = false;
        cdata = false;
        comment = false;
        dtd = false;
    }

    std::string attrib(const std::string &name)
    {
        if(!attributes.count(name))
            return "";
        
        return attributes.at(name);
    }
    
    xml_element* child(const std::string &tag, int i = 0)
    {
        std::vector<xml_element*> vec;
        int j = 0;
        for(auto &e : children_)
        {
            if (e->tag == tag)
            {
                if (j == i)
                    return e.get();
                ++j;
            }
        }
        
        return nullptr;
    }
    
    std::vector<unique_ptr<xml_element>> &children()
    {
        return children_;
    }

    std::vector<xml_element*> children(const std::string &tag)
    {
        std::vector<xml_element*> vec;
        for(auto &e : children_)
        {
            if (e->tag == tag)
            {
                vec.push_back(e.get());
            }
        }
        
        return vec;
    }
    
    std::vector<std::unique_ptr<xml_element>>::iterator first_tag(const std::string &tag)  
    {
        for(auto it = children_.begin(); it != children_.end(); ++it)
        {
            if((*it)->tag == tag)
                return it;
        }
        
        return children_.end();
    }
    
    std::vector<std::unique_ptr<xml_element>>::iterator next_tag(std::vector<std::unique_ptr<xml_element>>::iterator &it_, const std::string &tag)  
    {
        auto it = it_;
        ++it;
        for(; it != children_.end(); ++it)
        {
            if((*it)->tag == tag)
                return it;
        }
        
        return children_.end();
    }
     
    void clear()
    {
        tag = "";
        text = "";
        attributes.clear();
        closing = false;
        prolog = false;
        cdata = false;
        comment = false;
        dtd = false;
    }

    void to_stream(std::ostream &os, int margin = 0)
    {
        std::string m = std::string(margin, ' ');
        std::string m2 = std::string(margin + 3, ' ');
        os << m << "tag: " << tag << "\n";
        os << m << "attributes:\n";
        for (auto &p : attributes)
        {
            os << m2 << p.first << ": " << p.second << "\n";
        }

        os << m << "closing: " << ((closing) ? 1 : 0) << "\n";
        os << m << "prolog: " << ((prolog) ? 1 : 0) << "\n";
    }

    friend std::ostream &operator<<(std::ostream &os, xml_element &e);

};

std::ostream &operator<<(std::ostream &os, xml_element &e)
{
    e.to_stream(os);
    return os;
}


#endif // XML_ELEMENT

