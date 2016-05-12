#ifndef BOOST_OR_STD_TYPE_TRAITS_H
#define BOOST_OR_STD_TYPE_TRAITS_H

#if defined(USE_TR1)
#include <tr1/type_traits>
namespace boost_or_std {
	using namespace std::tr1;
}
#elif defined(USE_STD0X) || defined(USE_STD11) || defined(USE_STD14) ||\
defined(USE_STD17) || defined(USE_STD1y) || defined(USE_STD1z)
#include <type_traits>
namespace boost_or_std {
	using namespace std;
}
#else // USE_STD03
#include <boost/type_traits.hpp>
namespace boost_or_std {
	using namespace boost;
}
#endif

#endif // BOOST_OR_STD_TYPE_TRAITS_H
