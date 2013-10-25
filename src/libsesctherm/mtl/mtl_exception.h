#ifndef MTL_EXCEPTION_H
#define MTL_EXCEPTION_H

#if MTL_EXCEPTIONS

#include <exception>
#include <string>

namespace mtl {

/**

  The assertion class defines the type of exception that MTL functions
  will throw. Typically the assertions deal with the sizes of the
  vectors and or matrices being compatible. By default the exception
  handling is turned {\em off}. To compile with the exception
  handling turned {\em on}, define the macro, -DMTL\_EXCEPTIONS=1.

  Note that you need not put a {\tt try} and {\tt catch} around
  each MTL function call. A better style is to make your whole
  function (perhaps with many calls to MTL) a {\tt try} clause.
  This way the exception handling code does not interfere with
  the readability of your algorithm. In addition, this will
  result in your having to write fewer {\tt try} and {\tt catch}
  clauses.
  
  \begin{verbatim}
  try {
    mtl::dense1D<double> x(20, 2);
    mtl::dense1D<double> y(10, 3);
    mtl::dense1D<double> z(10);
    
    vecvec::add(x, y, z);
    
    mtl::print_vector(z);
  }
  catch(mtl::assertion e) {
    cerr << e.what() << endl;
  }
  > mtl assertion: x.size() <= y.size() failed in vecvec::add()
  \end{verbatim}


  @memo Assertion Exception Class

*/
class assertion : public std::exception {
public:
  assertion(const char* assertion, const char* function) {
    desc_ = "mtl assertion: ";
    desc_ += assertion;
    desc_ += " failed in ";
    desc_ += function;
  }

  virtual ~assertion() throw() { }

  virtual const char* what() const throw() { return desc_.c_str(); }

protected:
  std::string desc_;

};

} /* namespace mtl */

#endif


#if MTL_EXCEPTIONS
#define MTL_THROW_ASSERTION    throw(mtl::assertion)
#define MTL_ASSERT(X,Y)        if (!(X)) { throw mtl::assertion(#X,Y); }
#define MTL_THROW(X)           throw(X)
#else
#define MTL_THROW_ASSERTION    /* nothing */
#define MTL_ASSERT(X,Y)        /* nothing */
#define MTL_THROW(X)           /* nothing */
#endif


#endif
