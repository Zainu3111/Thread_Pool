#include <cmath>
bool is_prime(size_t x){
	size_t check_upto = static_cast<size_t>(std::sqrt(x));
	//std::cout << check_upto << std::endl;
	for (size_t i{2}; i <= check_upto; ++i){
		if(x % i == 0){
			return false;
		}
	}
	return true;
}
