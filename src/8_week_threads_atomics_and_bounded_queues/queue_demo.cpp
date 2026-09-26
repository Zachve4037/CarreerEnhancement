#include "bounded_blocking_queue.h"

#include <iostream>
#include <thread>

int main() {
 BoundedBlockingQueue<int> queue(5);
 std::thread producer([&]() {
  for (int i = 0; i < 10; ++i) {
   queue.push(i);
  }
 });
 std::thread consumer([&]() {
  for (int i = 0; i < 10; ++i) {
   std::cout << queue.pop() << std::endl;
  }
 });
 producer.join();
 consumer.join();
}