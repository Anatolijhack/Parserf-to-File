#pragma once
#include "LogPipeline.h"
#include <set>
class TxtParser : public LogParser
{
public:
	void ParseFile(const std::string& filenamem, std::function <void(const std::string&)> online) override;
	
};
class LineParser : public LogSearch
{
public:
	LogEntry ParseLine(const std::string& line) override;
	std::unique_ptr<LogSearch> Clone()  const override;
};
class LevelFilter : public LogFilter
{
private:
	std::string startTime;
	std::string endTime;
	std::string level;
public:
	LevelFilter(std::string level,std::string startTime,std::string endTime);
	std::unique_ptr<LogFilter> Clone() const override;
	bool FilterByLevel(const LogEntry& entry) override;

	

};
class ConsoleOutput : public IOutput
{
public:
	void write(const AnalysisResult& result) override;
};
class Analyzer : public IAnalyzer
{
private:
	AnalysisResult result;
	std::set<std::string> keyword;
public:
	Analyzer(const std::set<std::string>& keyword);
	void Proces(const LogEntry& entry) override;
	AnalysisResult GetResukt() override;
	std::unique_ptr<IAnalyzer> Clone() const override;
	void Merge(const IAnalyzer& other) override;
};