#include "LogPipeline.h"
#include "ThrreadPool.h"
#include <optional>
#include "Txt.h"

//LogPipeline::LogPipeline(std::unique_ptr<LogParser> p, std::unique_ptr<LogSearch> s, std::unique_ptr<LogFilter> f, std::unique_ptr<IAnalyzer> a, std::unique_ptr<IOutput> o) : analyze(std::move(a)), source(std::move(p)), filter(std::move(f)), parser(std::move(s)),output(std::move(o)) {}
//
//void LogPipeline::run(const std::string& filename, ThreadPool& pool)
//{
//    const size_t BATCH_SIZE = 200;
//
//    std::vector<std::string> buffer;
//    buffer.reserve(BATCH_SIZE);
//
//    bool firstLine = true;
//
//
//
//    std::vector<std::future<void>> futures;
//
//    source->ParseFile(filename, [&](const std::string& line)
//        {
//
//            if (firstLine && parser->HasHeader())
//            {
//                parser->Init(line);
//                firstLine = false;
//                return;
//            }
//
//            firstLine = false;
//            buffer.push_back(line);
//
//            if (buffer.size() >= BATCH_SIZE)
//            {
//                auto batch = std::move(buffer);
//                buffer.clear();
//
//                futures.emplace_back(
//                    pool.submit(0, [this, batch = std::move(batch)]() mutable
//                        {
//                           
//                            auto localAnalyzer = analyze->Clone();
//
//                            for (auto& line : batch)
//                            {
//                                auto entry = parser->ParseLine(line);
//
//                                if (!filter->FilterByLevel(entry))
//                                    continue;
//
//                                localAnalyzer->Proces(entry);
//                            }
//
//                           
//                            {
//                                std::lock_guard<std::mutex> lock(mtx);
//                                analyze->Merge(*localAnalyzer);
//                            }
//                        })
//                );
//            }
//            
//        });
//
//     //🔥 хвост
//    if (!buffer.empty())
//    {
//        auto batch = std::move(buffer);
//
//        futures.emplace_back(
//            pool.submit(0, [this, batch = std::move(batch)]() mutable
//                {
//
//                    auto localAnalyzer = analyze->Clone();
//
//                    for (auto& line : batch)
//                    {
//                        auto entry = parser->ParseLine(line);
//
//                        if (!filter->FilterByLevel(entry))
//                            continue;
//
//                        localAnalyzer->Proces(entry);
//                    }
//
//
//                    {
//                        std::lock_guard<std::mutex> lock(mtx);
//                        analyze->Merge(*localAnalyzer);
//                    }
//                })
//        );
//    }
//    for (auto& f : futures)
//    {
//        f.get();
//    }
//    {
//        std::lock_guard<std::mutex> lock(cout_mtx);
//        output->write(analyze->GetResukt());
//    }
//    
//}
 



LogPipeline::LogPipeline(
    std::unique_ptr<LogParser> p,
    std::unique_ptr<LogSearch> s,
    std::unique_ptr<LogFilter> f,
    std::set<std::string> keywords_,
    std::unique_ptr<IOutput> o)
    : source(std::move(p)),
    parser(std::move(s)),
    filter(std::move(f)),
    keywords_(std::move(keywords_)),
    output(std::move(o))
{}

AnalysisResult processBatch(
    std::vector<std::string> batch,
    LogSearch* parser,
    LogFilter* filter,
    std::set<std::string> keywords)
{
    auto parserLocal = parser->Clone();
    auto filterLocal = filter->Clone();

    Analyzer analyzer(keywords);

    for (const auto& line : batch)
    {
        auto entry = parserLocal->ParseLine(line);

        if (!filterLocal->FilterByLevel(entry))
            continue;

        analyzer.Proces(entry);
    }

    return analyzer.GetResukt();
}
AnalysisResult LogPipeline::run(const std::string& filename, ThreadPool& pool)
{
    const size_t BATCH_SIZE = 200;

    std::vector<std::string> buffer;
    buffer.reserve(BATCH_SIZE);

    std::vector<std::future<AnalysisResult>> futures;

    bool firstLine = true;

    // 🔥 берём безопасные "снимки" зависимостей
    LogSearch* parserPtr = parser.get();
    LogFilter* filterPtr = filter.get();
    std::set<std::string> keywordsCopy = keywords_;

    source->ParseFile(filename, [&](const std::string& line)
        {
            if (firstLine && parserPtr->HasHeader())
            {
                parserPtr->Init(line); // выполняется в одном потоке → ок
                firstLine = false;
                return;
            }

            firstLine = false;
            buffer.push_back(line);

            if (buffer.size() == BATCH_SIZE)
            {
                auto batch = std::move(buffer);
                buffer.clear();

                auto parserClone = std::shared_ptr<LogSearch>(parser->Clone());
                auto filterClone = std::shared_ptr<LogFilter>(filter->Clone());
                auto keywordsCopy = keywords_;

                futures.emplace_back(
                    pool.submit(0,
                        [batch = std::move(batch),
                        parser = parserClone,
                        filter = filterClone,
                        keywordsCopy]() mutable
                        {
                            Analyzer analyzer(keywordsCopy);

                            for (const auto& line : batch)
                            {
                                auto entry = parser->ParseLine(line);

                                if (!filter->FilterByLevel(entry))
                                    continue;

                                analyzer.Proces(entry);
                            }

                            return analyzer.GetResukt();
                        })
                );
            }
        });

    // 🔥 tail batch
    if (!buffer.empty())
    {
        auto parserClone = std::shared_ptr<LogSearch>(parser->Clone());
        auto filterClone = std::shared_ptr<LogFilter>(filter->Clone());
        auto keywordsCopy = keywords_;

        futures.emplace_back(
            pool.submit(0,
                [batch = std::move(buffer),
                parser = parserClone,
                filter = filterClone,
                keywordsCopy]() mutable
                {
                    Analyzer analyzer(keywordsCopy);

                    for (const auto& line : batch)
                    {
                        auto entry = parser->ParseLine(line);

                        if (!filter->FilterByLevel(entry))
                            continue;

                        analyzer.Proces(entry);
                    }

                    return analyzer.GetResukt();
                })
        );
    }

    // 🔥 reduce
    AnalysisResult finalResult;

    for (auto& f : futures)
    {
        finalResult.Merge(f.get());
    }

    return finalResult;
}


