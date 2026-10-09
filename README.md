# Safe Task Queue

A concurrent executor with bounded queue capacity, priorities, task-start deadlines, futures, and shutdown that drains or cancels queued tasks.

## Run

Requirements: C++20 and CMake.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
build/task_queue > result.json
```

## Behavior

The reusable implementation is in `include/task_queue.hpp`. Tests cover 1,000 concurrent tasks, exceptions, deadlines, and rejection after shutdown. A deadline is checked before a task starts; it does not interrupt a running task.

## Optional report archive

Export a JSON report from the command above, then run `python cloud/sync.py enqueue result.json --project cpp-safe-task-queue` and `python cloud/sync.py sync`. Synchronization requires `BRUNNODEV_ACCESS_TOKEN` and the external operations API; the local outbox retains unacknowledged reports.

## License

Original source and documentation are MIT licensed; see [LICENSE](LICENSE). Third-party dependencies and media retain their respective terms. Maintained by [Brunno Dev](https://brunnodev.store).
