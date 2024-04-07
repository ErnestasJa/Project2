#ifndef THEPROJECTMAIN_TYPEUTILS_H
#define THEPROJECTMAIN_TYPEUTILS_H

#define NONCOPYABLE(Type)                                                                          \
  Type(const Type&)            = delete;                                                           \
  Type& operator=(const Type&) = delete

#endif // THEPROJECTMAIN_TYPEUTILS_H
