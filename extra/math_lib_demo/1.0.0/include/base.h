#ifndef BASE_H
#define BASE_H

#ifdef _WIN32
#if defined(B_BUILD_LIB)
#define B_EXPORT B_DECL_EXPORT
#else
#define B_EXPORT B_DECL_IMPORT
#endif

#define B_DECL_EXPORT __declspec(dllexport)
#define B_DECL_IMPORT __declspec(dllimport)
#else
#define B_EXPORT B_DECL_EXPORT
#define B_DECL_EXPORT __attribute__((visibility("default")))
#define B_DECL_IMPORT
#endif 

class base_p;

class B_EXPORT base
{
public:
    base();
    base(base_p &);
    ~base();
    
    base_p *p_;
};

#endif // BASE_H
