# Safe Task Queue

Executor concorrente com capacidade limitada, prioridades, prazo de execução, futures e encerramento com drenagem ou cancelamento.

## Executar

Requisitos: C++20 e CMake.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
build/task_queue > resultado.json
```

## Funcionamento

A implementação reutilizável está em `include/task_queue.hpp`. Os testes exercitam mil tarefas concorrentes, exceções, prazo e rejeição após encerramento.

## Persistência de resultados

O arquivo de operações está em [vercel-home-telemetry-api.vercel.app](https://vercel-home-telemetry-api.vercel.app/laboratory.html?project=cpp-safe-task-queue). As migrações Supabase estão no [repositório da API](https://github.com/brunnojob/vercel-home-telemetry-api/tree/main/supabase/migrations).

```sh
python cloud/sync.py enqueue resultado.json --project cpp-safe-task-queue
python cloud/sync.py sync
```

Defina `BRUNNODEV_ACCESS_TOKEN` com sua sessão. A fila SQLite conserva os relatórios até confirmação do servidor; o mesmo conteúdo não gera registros duplicados. Tokens não são gravados no código.
