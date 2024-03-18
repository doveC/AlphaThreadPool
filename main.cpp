#include <thread>
#include <iostream>
#include "ThreadPool.h"

using namespace std;

int main()
{
    Util::ThreadPool pool;
    std::mutex ouputMutex;
    std::vector<std::future<int>> vec;
    for (int i = 1; i < 20; i++)
    {
      vec.push_back(pool.submit(
          [&](int id) -> int {
            if (id % 2 == 0)
            {
              this_thread::sleep_for(0.2s);
              lock_guard<mutex> lock(ouputMutex);
              cout << "id: " << id << endl;
              return -1;
            }
            else
            {
              return id;
            }
          },
          i));
    }

    for (auto& fut : vec)
    {
        int i = fut.get();
        cout << i << endl;
    }
}