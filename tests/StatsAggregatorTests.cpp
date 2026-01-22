#include "StatsAggregator.h"

#include <cassert>
#include <cmath>
#include <iostream>

using PcapLearn::Side;
using PcapLearn::StatsAggregator;

void TestBasicMatch()
{
    StatsAggregator aggregator;
    aggregator.ProcessPacket(Side::kA, 1, 100);
    aggregator.ProcessPacket(Side::kB, 1, 250);
    [[maybe_unused]] const auto summary = aggregator.BuildSummary();
    assert(summary.totalPacketsSideA == 1);
    assert(summary.totalPacketsSideB == 1);
    assert(summary.unmatchedSideA == 0);
    assert(summary.unmatchedSideB == 0);
    assert(summary.fasterCountA == 1);
    assert(summary.fasterCountB == 0);
    assert(std::abs(summary.averageAdvantageA - 150.0) < 1e-9);
    assert(summary.averageFastestAdvantage == summary.averageAdvantageA);
}

void TestUnmatchedCounts()
{
    StatsAggregator aggregator;
    aggregator.ProcessPacket(Side::kA, 5, 1'000);
    aggregator.ProcessPacket(Side::kA, 6, 2'000);
    aggregator.ProcessPacket(Side::kB, 7, 3'000);
    [[maybe_unused]] const auto summary = aggregator.BuildSummary();
    assert(summary.unmatchedSideA == 2);
    assert(summary.unmatchedSideB == 1);
}

void TestDuplicatePackets()
{
    StatsAggregator aggregator;
    aggregator.ProcessPacket(Side::kA, 10, 100);
    aggregator.ProcessPacket(Side::kA, 10, 200); // duplicate before match
    aggregator.ProcessPacket(Side::kB, 10, 50);
    aggregator.ProcessPacket(Side::kA, 10, 300); // duplicate after match
    aggregator.ProcessPacket(Side::kB, 10, 400); // duplicate after match

    [[maybe_unused]] const auto summary = aggregator.BuildSummary();
    assert(summary.totalPacketsSideA == 3);
    assert(summary.totalPacketsSideB == 2);
    assert(summary.unmatchedSideA == 0);
    assert(summary.unmatchedSideB == 0);
    assert(summary.fasterCountB == 1);
    assert(summary.fasterCountA == 0);
}

int main()
{
    TestBasicMatch();
    TestUnmatchedCounts();
    TestDuplicatePackets();
    std::cout << "All StatsAggregator tests passed" << std::endl;
    return 0;
}
