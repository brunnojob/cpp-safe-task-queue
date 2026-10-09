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

## Result synchronization

The [operations archive](https://vercel-home-telemetry-api.vercel.app/laboratory.html?project=cpp-safe-task-queue) stores execution results. Supabase migrations are in the [API repository](https://github.com/brunnojob/vercel-home-telemetry-api/tree/main/supabase/migrations).

```sh
python cloud/sync.py enqueue result.json --project cpp-safe-task-queue
python cloud/sync.py sync
```

Set `BRUNNODEV_ACCESS_TOKEN` to your session token. The SQLite outbox retains reports until the server confirms persistence; identical content does not create duplicate records. Tokens are not stored in source code. To run the synchronization tests:

```sh
python -m unittest discover -s cloud
```
