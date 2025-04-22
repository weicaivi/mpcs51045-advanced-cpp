#include "my_promise.h"
#include <thread>
#include <iostream>
#include <stdexcept>
#include <exception>

using namespace mpcs;
using namespace std;

int main()
{
    cout << "Starting program..." << endl;
    
    MyPromise<int> mpi;
    
    cout << "Creating consumer thread..." << endl;
    thread thr{ [&]() { 
        cout << "Consumer thread started" << endl;
        try {
            cout << "Future waiting for value..." << endl;
            cout.flush(); // Ensure output is flushed
            
            int result = mpi.get_future().get();
            
            cout << "Received value: " << result << endl;
        } catch(exception &e) {
            cout << "Exception caught: " << e.what() << endl;
        }
        cout << "Consumer thread ending" << endl;
    }};
    
    cout << "Main thread waiting a moment..." << endl;
    // Give the consumer thread time to start and print
    this_thread::sleep_for(chrono::milliseconds(500));
    
    cout << "Main thread setting value..." << endl;
    
    // Toggle between setting a value or an exception
#if 1
    cout << "Setting value to 7" << endl;
    mpi.set_value(7);
#else
    cout << "Setting exception" << endl;
    try {
        throw runtime_error("Some runtime error");
    }
    catch (exception &) {
        mpi.set_exception(current_exception());
    }
#endif

    cout << "Main thread waiting for consumer to finish..." << endl;
    thr.join();
    
    cout << "Program completed successfully" << endl;
    return 0;
}