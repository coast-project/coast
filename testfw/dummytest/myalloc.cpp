#include <cstddef>
#include <iostream>

template <typename T, typename U = long double>
struct AlignedSize {
	struct X {
		T a;
		U b;
	};
	static size_t const offset = (sizeof(T) < offsetof(X, b)) ? offsetof(X, b) : sizeof(T);
	static const size_t value = sizeof(T) + ((sizeof(T) % sizeof(U)) != 0U ? (sizeof(U) - sizeof(T) % sizeof(U)) : 0);
};
struct AllocLD {
	char *ptr;
	long double ld;
};

int main() {
	using namespace std;
	cout << "offset of char* to long double: " << AlignedSize<char *>::offset << endl;
	cout << "value of char* to long double: " << AlignedSize<char *>::value << endl;
	cout << "sizeof(char*): " << sizeof(char *) << endl;
	cout << "sizeof(long double): " << sizeof(long double) << endl;
	cout << "sizeof(AllocLD): " << sizeof(AllocLD) << endl;
}
