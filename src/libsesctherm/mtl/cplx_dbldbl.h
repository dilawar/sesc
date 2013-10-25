#ifndef MTL_CPLX_DLBDLB_H
#define MTL_CPLX_DLBDLB_H

#include "mtl_complex.h"
#include "doubledouble.h"
#include "mtl_config.h"

#include <iosfwd>

namespace std {

// stupid g++
inline complex<doubledouble> sqrt(complex<doubledouble> const& cx)
{
  doubledouble re, im;
  if( cx.imag()== doubledouble(0) ) {
    re = sqrt( abs( cx.real() ) );
    im = 0;
  } else {
    re = sqrt ((abs(cx.real()) + abs(cx))*(doubledouble)0.5);
    im = cx.imag()/(2*re);
  }
  if( cx.real()< doubledouble(0) ) {
    doubledouble temp;
    if( cx.imag()>= doubledouble(0) ) {
      temp=re; re=im;
    } else {
      temp=-re; re=-im;
    }
    im = temp;
  }
  return complex<doubledouble>(re,im);
}

inline std::ostream&
operator<<(std::ostream& os, complex<doubledouble> const& x)
{
  cout << "(" << x.real() << "," << x.imag() << ")" << endl;
  return os;
}

inline complex<doubledouble>
operator/(complex<doubledouble> const & c1, complex<doubledouble> const & c2)
{
  // JGS should check div by zero?
  doubledouble value = norm(c2);
  return complex<doubledouble> (
      (c1.real ()*c2.real ()+c1.imag ()*c2.imag ())/value,
      (c2.real ()*c1.imag ()-c1.real ()*c2.imag ())/value);
}

} /* namespace std */

#endif
