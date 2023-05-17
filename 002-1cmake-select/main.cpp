#include <iostream>
#include <thread>
#include <functional>
using namespace std;
void ThreadFunc(int &a){
    while(a--){
        cout << "a = " << a <<std::endl;
    }
}

int main(){
    cout << "hello ,cmake" << endl;
    thread p1  = thread(bind(ThreadFunc, 10));
    p1.join();
    return 0;
}