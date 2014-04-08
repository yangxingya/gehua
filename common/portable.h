/*
 *
 */

#if !defined gehua_portable_h_
#define gehua_portable_h_


#if defined _MSC_VER
# include <memory>
#else  // _MSC_VER
# if defined __GNUC__
#  include <tr1/memory>
# endif // __GNUC__
#endif

#if (defined _MSC_VER) && (_MSC_VER == 1500)
using ::std::tr1::weak_ptr;
using ::std::tr1::shared_ptr;
#else
using ::std::weak_ptr;
using ::std::shared_ptr;
#endif


#endif // ! gehua_portable_h_