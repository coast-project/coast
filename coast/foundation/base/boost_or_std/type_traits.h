#ifndef BOOST_OR_STD_TYPE_TRAITS_H
#define BOOST_OR_STD_TYPE_TRAITS_H

#if defined(USE_TR1)
#include <tr1/type_traits>
namespace boost_or_std {
	using namespace std::tr1;
}
#elif __cplusplus >= 201103L
#include <type_traits>
namespace boost_or_std {
	using namespace std;
}
#else
#include <boost/type_traits.hpp>
namespace boost_or_std {
	using namespace boost;
}
#endif

#endif	// BOOST_OR_STD_TYPE_TRAITS_H
