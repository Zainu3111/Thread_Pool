#include <fstream>
inline void record_result(const std::string& benchmark,
					size_t input,
					size_t threads,
					long long time_ms
		){
	std::ofstream out("/home/zain/Desktop/Threadpool/Documentation/benchmark_results.csv", std::ios::app);
	
	if(out.tellp() == 0){
		out << "Benchmark, InputSize, Threads, Time(ms)\n";
	}
	out << benchmark << ","
		<< input << ","
		<< threads << ","
		<< time_ms << "\n";
}
