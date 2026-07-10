# 🚀 Log Processing Pipeline (C++)

Многопоточный пайплайн для обработки логов с поддержкой **CSV / JSON / TXT**, фильтрацией, анализом текста и агрегацией результатов.

---

## 📌 Возможности

* 📂 Поддержка форматов:

  * `.csv`
  * `.json`
  * `.log` / `.txt`

* ⚙️ Pipeline архитектура:

  * `Parser` — чтение файла
  * `Search` — парсинг строки
  * `Filter` — фильтрация по уровню и времени
  * `Analyzer` — анализ текста
  * `Output` — вывод результата

* 🧵 Многопоточность:

  * ThreadPool
  * batch processing
  * futures + reduce

* 🔍 Анализ:

  * подсчёт уровней логов
  * поиск ключевых слов
  * нормализация слов
  * bigram (словосочетания)

---

## 🏗 Архитектура

Проект реализует pipeline-подход:

```
File → Parser → LineParser → Filter → Analyzer → Output
```

Каждый этап изолирован и легко заменяется (Strategy pattern).

---

## 🧵 Параллелизм

* Файл разбивается на **батчи**
* Каждый батч обрабатывается в отдельном потоке
* Результаты объединяются через **reduce**

```
batch → thread → AnalysisResult → merge → final result
```

---

## 📁 Структура проекта

```
/core
  LogPipeline.h
  ThreadPool.h

/parsers
  Csv.h / Csv.cpp
  Json.h / Json.cpp
  Txt.h / Txt.cpp

/utils
  helpers (normalize, split, time parsing)

main.cpp
```

---

## ⚙️ Сборка

### Требования:

* C++17+
* Компилятор: MSVC / GCC / Clang

### Пример (g++):

```bash
g++ -std=c++17 -pthread *.cpp -o log_pipeline
```

---

## ▶️ Запуск

```cpp
ThreadPool pool(8);

auto pipeline = createPipeline("logs.csv");

auto result = pipeline->run("logs.csv", pool);

pipeline->getOutput()->write(result);
```

---

## 📂 Обработка директории

```cpp
for (const auto& entry : fs::recursive_directory_iterator(dir))
{
    auto pipeline = createPipeline(entry.path());

    if (!pipeline) continue;

    auto result = pipeline->run(entry.path().string(), pool);

    pipeline->getOutput()->write(result);
}
```

---

## 🔎 Фильтрация

Поддерживается:

* уровень логов:

  * `ERROR`, `INFO`, `WARNING`

* временной диапазон:

```cpp
"2025-06-01 10:00:00" → "2025-06-01 12:00:00"
```

---

## 🧠 Анализ текста

* разбиение на слова
* нормализация:

  * `running → run`
  * `errors → error`
* поиск:

  * по префиксу
  * по вхождению
* bigram:

  * `database_error`
  * `connection_failed`

---

## 📊 Пример вывода

### CSV

```
LEVEL,COUNT
"ERROR",10

WORD,COUNT
"database",5
"connection_failed",3
```

### JSON

```json
{
  "levels": {
    "ERROR": 10
  },
  "keywords": {
    "database": 5,
    "connection_failed": 3
  }
}
```

---

## ➕ Добавление нового формата

1. Создать:

   * `Parser`
   * `LineParser`
   * `Filter`
   * `Output`

2. Подключить в `createPipeline()`:

```cpp
if (ext == ".xml")
{
    return std::make_unique<LogPipeline>(...);
}
```

---

## 💡 Особенности

* Thread-safe через `Clone()`
* Нет shared mutable state между потоками
* Расширяемая архитектура
* Подходит для больших логов

---

## 🚀 Возможные улучшения

* streaming pipeline (Stage chain)
* backpressure
* top-K keywords
* метрики (latency, throughput)
* конфигурация через JSON/YAML
* улучшенный stemming (Porter)

---

## 🧑‍💻 Автор

Anatoliy — C++ разработчик

---

## 📄 Лицензия

MIT / свободное использование
