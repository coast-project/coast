#ifndef BOOST_OR_STD_MEMORY_H
#define BOOST_OR_STD_MEMORY_H

#if defined(USE_TR1)
#include <tr1/memory>
namespace boost_or_std {
	using std::auto_ptr;
	using std::tr1::shared_ptr;
}  // namespace boost_or_std
#elif __cplusplus >= 201103L
#include <memory>
namespace boost_or_std {
	template <class T, class Deleter = std::default_delete<T> >
	using auto_ptr = std::unique_ptr<T, Deleter>;

	using std::shared_ptr;
}  // namespace boost_or_std
#else  // USE_STD03 || USE_STD98
#include <boost/shared_ptr.hpp>
#include <memory>
namespace boost_or_std {
	using boost::shared_ptr;
	using std::auto_ptr;
}  // namespace boost_or_std
#endif

#endif	// BOOST_OR_STD_MEMORY_H
