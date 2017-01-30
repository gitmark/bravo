#ifndef BASE_H
#define BASE_H

#  if defined(C_BUILD_LIB)
#    define C_EXPORT C_DECL_EXPORT
#  else
#    define C_EXPORT C_DECL_IMPORT
#  define C_DECL_EXPORT __declspec(dllexport)
#  define C_DECL_IMPORT __declspec(dllimport)


class base_p;

class C_EXPORT base
{
public:
    base();
    base(base_p &);
    
    std::unique_ptr<base_p> p_;
};

#endif // BASE_H
