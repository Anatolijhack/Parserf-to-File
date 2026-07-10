#include "Txt.h"
#include "Csv.h"
#include "Json.h"
#include <filesystem>
namespace fs = std::filesystem;

std::unique_ptr<LogPipeline> createPipeline(const std::filesystem::path& path)
{
    std::string ext = path.extension().string();

    std::set<std::string> keywords = { "error", "connection","database"};

    if (ext == ".csv")
    {
        return std::make_unique<LogPipeline>(
            std::make_unique<ParseCSV>(),
            std::make_unique<ParseLineCsv>(),
            std::make_unique<CsvFilter>(
                "ERROR",
                "2025-06-01 10:00:00",
                "2025-06-01 12:00:00"
            ),
            keywords,
            std::make_unique<CsvOutput>()   // ✔ вернули
        );
    }
    else if (ext == ".log")
    {
        return std::make_unique<LogPipeline>(
            std::make_unique<TxtParser>(),
            std::make_unique<LineParser>(),
            std::make_unique<LevelFilter>(
                "ERROR",
                "2025-06-01 10:00:00",
                "2025-06-01 12:00:00"
            ),
            keywords,
            std::make_unique<ConsoleOutput>()   // ✔ вернули
        );
    }
    else if (ext == ".json")
    {
        return std::make_unique<LogPipeline>(
            std::make_unique<JsonParse>(),
            std::make_unique<JsonLineParse>(),
            std::make_unique<JsonFilterbyLevel>(
                "ERROR",
                "2025-06-01 10:00:00",
                "2025-06-01 12:00:00"
            ),
            keywords,
            std::make_unique<JsonOutput>()   // ✔ вернули
        );
    }

    return nullptr;
}
void runOnDirectory(const std::string& dir, LogPipeline& pipeline, ThreadPool& pool)
{
    for (const auto& entry : fs::recursive_directory_iterator(dir))
    {
        if (!entry.is_regular_file())
            continue;

        const auto& path = entry.path();

        // 👉 фильтр форматов (ВАЖНО, у тебя 3 формата)
        if (path.extension() == ".csv" ||
            path.extension() == ".log" ||
            path.extension() == ".txt")
        {
            pipeline.run(path.string(), pool);
        }
    }
}
int main()
{
    //ThreadPool pool(4);
    //// ===== SOURCE (читает файл построчно)
    //std::unique_ptr<LogParser> source = std::make_unique<TxtParser>();

    //// ===== PARSER (строку → LogEntry)
    //std::unique_ptr<LogSearch> parser = std::make_unique<LineParser>();

    //// ===== FILTER (например ERROR + time можно расширить)
    //std::unique_ptr<LogFilter> filter =
    //    std::make_unique<LevelFilter>(
    //        "ERROR",
    //        "2025-06-01 10:00:00",
    //        "2025-06-01 12:00:00"
    //    );

    //// ===== ANALYZER (счёт слов + уровней)
    //std::set<std::string> keywords = { "error", "fail", "warning" };

    //std::unique_ptr<IAnalyzer> analyzer =
    //    std::make_unique<Analyzer>(keywords);

    //// ===== OUTPUT
    //std::unique_ptr<IOutput> output =
    //    std::make_unique<ConsoleOutput>();

    //// ===== PIPELINE
    //LogPipeline pipeline(
    //    std::move(source),
    //    std::move(parser),
    //    std::move(filter),
    //    std::move(analyzer),
    //    std::move(output)
    //);

    //// ===== RUN
    //pipeline.run("log.txt",pool);

    //return 0;
   // ThreadPool pool(4);
   ////===== SOURCE (читает файл)
   // //std::unique_ptr<LogParser> source =
   // //    std::make_unique<JsonParse>();

   // //// ===== PARSER (строка JSON → LogEntry)
   // //std::unique_ptr<LogSearch> parser =
   // //    std::make_unique<JsonLineParse>();

   // //// ===== FILTER
   // //std::unique_ptr<LogFilter> filter =
   // //    std::make_unique<JsonFilterbyLevel>("ERROR",
   // //               "2025-06-01 10:00:00",
   // //               "2025-06-01 12:00:00");

   // //// ===== ANALYZER
   // //std::set<std::string> keywords = { "error", "fail", "warning" };

   // //std::unique_ptr<IAnalyzer> analyzer =
   // //    std::make_unique<JsonAnalyzer>();

   // //// ===== OUTPUT
   // //std::unique_ptr<IOutput> output =
   // //    std::make_unique<JsonOutput>();

   // //// ===== PIPELINE
   // //LogPipeline pipeline(
   // //    std::move(source),
   // //    std::move(parser),
   // //    std::move(filter),
   // //    std::move(analyzer),
   // //    std::move(output)
   // //);

   // //// ===== RUN
   // //pipeline.run("log2.json",pool);
    //ThreadPool pool(4);
    ////return 0;
    //std::unique_ptr<LogParser> source =
    //    std::make_unique<ParseCSV>();

    // ===== PARSER (CSV строка → LogEntry)
    //std::unique_ptr<LogSearch> parser =
    //    std::make_unique<ParseLineCsv>();

    // ===== FILTER (например по уровню)
    //std::unique_ptr<LogFilter> filter =
    //    std::make_unique<CsvFilter>("ERROR", "2025-06-01 10:00:00","2025-06-01 12:00:00");

    // ===== KEYWORDS
    //std::set<std::string> keywords =
    //{ "error", "fail", "warning", "exception" };

    // ===== ANALYZER
    //std::unique_ptr<IAnalyzer> analyzer =
    //    std::make_unique<CsvAnalyzer>(keywords);

    // ===== OUTPUT
    //std::unique_ptr<IOutput> output =
    //    std::make_unique<CsvOutput>();

    // ===== PIPELINE
    //LogPipeline pipeline(
    //    std::move(source),
    //    std::move(parser),
    //    std::move(filter),
    //    std::move(analyzer),
    //    std::move(output)
    //);

    // ===== RUN
    //pipeline.run("log1.csv",pool);

   // return 0;
    // =========================
    // 1. ОДИН ГЛАВНЫЙ THREADPOOL
    // =========================
ThreadPool pool(8);

// =========================
// 2. ПАПКА С ЛОГАМИ
// =========================
std::string dir =
"C:\\Users\\Kirill\\source\\repos\\ParsFile by me test";

// =========================
// 3. FUTURES ДЛЯ ФАЙЛОВ
// =========================
std::vector<std::future<void>> fileFutures;

// =========================
// 4. ОБХОД ФАЙЛОВ
// =========================
for (const auto& entry : fs::recursive_directory_iterator(dir))
{
    if (!entry.is_regular_file())
        continue;

    std::filesystem::path path = entry.path();

    fileFutures.emplace_back(
        pool.submit(0, [path, &pool]()
            {
                // =========================
                // 1. создаём pipeline
                // =========================
                auto pipeline = createPipeline(path);

                if (!pipeline)
                    return;

                // =========================
                // 2. запускаем анализ
                // =========================
                AnalysisResult result =
                    pipeline->run(path.string(), pool);

                // =========================
                // 3. ВАЖНО: вывод ТОЛЬКО ЗДЕСЬ
                // =========================
                {
                    static std::mutex outMtx;
                    std::lock_guard<std::mutex> lock(cout_mtx);

                /*    if (result.levelCount.empty() &&
                        result.keywordsCount.empty())
                    {
                        std::cout << "EMPTY RESULT\n";
                    }*/

                    pipeline->getOutput()->write(result);
                }
            })
    );
}

// =========================
// 5. ЖДЁМ ВСЕ ФАЙЛЫ
// =========================
for (auto& f : fileFutures)
{
    f.get();
}

// =========================
// 6. ГРАЦИОЗНОЕ ЗАВЕРШЕНИЕ
// =========================
pool.shutdown();
}