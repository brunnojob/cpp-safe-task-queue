# cpp-safe-task-queue

C++20 bounded worker queue for local batch jobs. Producers block when capacity is full; shutdown drains accepted jobs before workers exit.

Build: `c++ -std=c++20 -pthread -Wall -Wextra -Werror src/main.cpp -o queue`. Run `./queue`.

Project by [Brunno Dev](https://brunnodev.store).