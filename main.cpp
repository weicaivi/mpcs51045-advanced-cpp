#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <execution>
#include <chrono>
#include <iomanip>

// Function to measure sorting time with different execution policies
template <typename ExecutionPolicy>
double measure_sort_time(ExecutionPolicy policy, std::vector<double>& data) {
    std::vector<double> data_copy = data;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::sort(policy, data_copy.begin(), data_copy.end());
    
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

int main() {
    const size_t vector_size = 10'000'000;
    const int num_runs = 5;
        
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-1000000.0, 1000000.0);
    
    std::vector<double> data(vector_size);
    for (auto& val : data) {
        val = dist(gen);
    }
        
    std::vector<double> seq_times, par_times, par_unseq_times;
    
    for (int i = 0; i < num_runs; ++i) {
        std::cout << "Run " << (i + 1) << ":" << std::endl;
        
        // Sequential (default)
        double seq_time = measure_sort_time(std::execution::seq, data);
        seq_times.push_back(seq_time);
        std::cout << "  std::execution::seq      : " << std::fixed << std::setprecision(2) 
                  << seq_time << " ms" << std::endl;
        
        // Parallel
        double par_time = measure_sort_time(std::execution::par, data);
        par_times.push_back(par_time);
        std::cout << "  std::execution::par      : " << std::fixed << std::setprecision(2) 
                  << par_time << " ms" << std::endl;
        
        // Parallel unsequenced
        double par_unseq_time = measure_sort_time(std::execution::par_unseq, data);
        par_unseq_times.push_back(par_unseq_time);
        std::cout << "  std::execution::par_unseq: " << std::fixed << std::setprecision(2) 
                  << par_unseq_time << " ms" << std::endl;
        
        std::cout << std::endl;
    }
    
    auto average = [](const std::vector<double>& v) {
        return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };
    
    double avg_seq = average(seq_times);
    double avg_par = average(par_times);
    double avg_par_unseq = average(par_unseq_times);
    
    std::cout << "=== SUMMARY ===" << std::endl;
    std::cout << "Average times over " << num_runs << " runs:" << std::endl;
    std::cout << "  std::execution::seq      : " << std::fixed << std::setprecision(2) 
              << avg_seq << " ms" << std::endl;
    std::cout << "  std::execution::par      : " << std::fixed << std::setprecision(2) 
              << avg_par << " ms" << std::endl;
    std::cout << "  std::execution::par_unseq: " << std::fixed << std::setprecision(2) 
              << avg_par_unseq << " ms" << std::endl;
    
    double speedup_par = avg_seq / avg_par;
    double speedup_par_unseq = avg_seq / avg_par_unseq;
    
    std::cout << "\nSpeedups compared to sequential:" << std::endl;
    std::cout << "  std::execution::par      : " << std::fixed << std::setprecision(2) 
              << speedup_par << "x" << std::endl;
    std::cout << "  std::execution::par_unseq: " << std::fixed << std::setprecision(2) 
              << speedup_par_unseq << "x" << std::endl;
    
    return 0;
}