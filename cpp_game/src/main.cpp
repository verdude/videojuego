#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
  using Clock = std::chrono::steady_clock;
  auto t = Clock::now();

  int ticks = 10;
  while (ticks-- > 0) {
    auto dt = Clock::now();
    double frameTime = (dt-t).count();
    std::cout << frameTime << "\n";
    t = dt;
  }

  return 0;
}
