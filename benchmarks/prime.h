#include <cmath>
inline bool is_prime(size_t x){
	if (x <= 1){
		return false;
	}
	size_t check_upto = static_cast<size_t>(std::sqrt(x));
	//std::cout << check_upto << std::endl;
	for (size_t i{2}; i <= check_upto; ++i){
		if(x % i == 0){
			return false;
		}
	}
	return true;
}
