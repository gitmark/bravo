#ifndef BASE_H
#define BASE_H
#include <memory>

#ifdef _WIN32
#if defined(B_BUILD_LIB)
#define B_EXPORT B_DECL_EXPORT
#else
#define B_EXPORT B_DECL_IMPORT
#endif

#define B_DECL_EXPORT __declspec(dllexport)
#ifdef B_STATIC_BUILD
#define B_DECL_IMPORT
#else
#define B_DECL_IMPORT __declspec(dllimport)
#endif

#else
#define B_EXPORT B_DECL_EXPORT
#define B_DECL_EXPORT __attribute__((visibility("default")))
#define B_DECL_IMPORT
#endif 

//https://support.microsoft.com/en-us/help/168958/how-to-export-an-instantiation-of-a-standard-template-library-stl-class-and-a-class-that-contains-a-data-member-that-is-an-stl-object

// google "needs to have dll-interface to be used by clients of class"

class base_p;

class B_EXPORT base
{
public:
    base();
    base(base_p &);
    ~base();

    base_p *_p;
};

#endif // BASE_H
