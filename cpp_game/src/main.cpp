#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
  using Clock = std::chrono::steady_clock;
  auto current_time = Clock::now();

  // 60 ticks per second
  double tick_duration = 1.0/60;
  auto simulatable_time = 0.0;

  while (true) {
    auto new_time = Clock::now();
    double elapsed_time = std::chrono::duration<double>(new_time-current_time).count();
    current_time = new_time;
    simulatable_time += elapsed_time;

    while (simulatable_time >= tick_duration) {
      // simulate gameplay
      std::cout << elapsed_time << "\n";
      simulatable_time -= tick_duration;
    }

    // render
  }

  return 0;
}
