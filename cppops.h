#ifndef _CPPOPS_H
#define _CPPOPS_H

#define DefCPPOps_Mask(datatype)\
\
static inline datatype operator|=(datatype &lhs, datatype rhs)\
{\
  lhs = (datatype)(((int)lhs) | ((int)rhs));\
  return lhs;\
}\
\
static inline datatype operator&=(datatype &lhs, datatype rhs)\
{\
  lhs = (datatype)(((int)lhs) & ((int)rhs));\
  return lhs;\
}\
\
static inline datatype operator|(datatype lhs, datatype rhs)\
{\
  return (datatype)(((int)lhs) | ((int)rhs));\
}\
\
static inline datatype operator&(datatype lhs, datatype rhs)\
{\
  return (datatype)(((int)lhs) & ((int)rhs));\
}\
\
static inline datatype operator~(datatype rhs)\
{\
  return (datatype)(~((int)rhs));\
}\

#define DefCPPOps_Enum(datatype)\
\
static inline datatype operator++(datatype &rhs, int)\
{\
  datatype old = rhs;\
  rhs = (datatype)((int)rhs + 1);\
  return old;\
}\
\
static inline datatype operator--(datatype &rhs, int)\
{\
  datatype old = rhs;\
  rhs = (datatype)((int)rhs - 1);\
  return old;\
}\
\
static inline datatype operator+(datatype lhs, int rhs)\
{\
  return (datatype)((int)lhs + rhs);\
}\
\
static inline datatype operator-(datatype lhs, int rhs)\
{\
  return (datatype)((int)lhs - rhs);\
}\

#endif /* _CPPOPS_H */
