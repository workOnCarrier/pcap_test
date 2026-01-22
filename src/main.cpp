#include "PcapDirectoryProcessor.h"
#include "PcapFileReader.h"
#include "StatsAggregator.h"

#include <iomanip>
#include <iostream>
#include <string>

namespace
{
void PrintSummary(const flowtraders::StatsSummary& summary)
{
    std::cout << "Side A total packets: " << summary.totalPacketsSideA << '\n';
    std::cout << "Side B total packets: " << summary.totalPacketsSideB << '\n';
    std::cout << "Side A unmatched packets: " << summary.unmatchedSideA << '\n';
    std::cout << "Side B unmatched packets: " << summary.unmatchedSideB << '\n';
    std::cout << "Packets where A faster: " << summary.fasterCountA << '\n';
    std::cout << "Packets where B faster: " << summary.fasterCountB << '\n';

    auto printAverage = [](const std::string& label, double value) {
        std::cout << label << std::fixed << std::setprecision(2) << value << " ns" << std::defaultfloat << '\n';
    };

    printAverage("Average advantage when A faster: ", summary.averageAdvantageA);
    printAverage("Average advantage when B faster: ", summary.averageAdvantageB);
    printAverage("Average advantage of fastest channel overall: ", summary.averageFastestAdvantage);
}
} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: feed_arbitration <pcap_directory>" << std::endl;
        return 1;
    }

    flowtraders::StatsAggregator aggregator;
    flowtraders::PcapFileReader reader(aggregator);
    flowtraders::PcapDirectoryProcessor processor(reader);
    try
    {
        processor.ProcessDirectory(argv[1]);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    const auto summary = aggregator.BuildSummary();
    PrintSummary(summary);
    return 0;
}
