#pragma once

#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace PcapLearn
{

enum class Side
{
    kA,
    kB
};

struct StatsSummary
{
    uint64_t totalPacketsSideA = 0;
    uint64_t totalPacketsSideB = 0;
    uint64_t unmatchedSideA = 0;
    uint64_t unmatchedSideB = 0;
    uint64_t fasterCountA = 0;
    uint64_t fasterCountB = 0;
    double averageAdvantageA = 0.0;
    double averageAdvantageB = 0.0;
    double averageFastestAdvantage = 0.0;
};

class StatsAggregator
{
public:
    void ProcessPacket(Side side, uint64_t sequenceNumber, uint64_t timestampNs);

    StatsSummary BuildSummary() const;

private:
    struct SequenceState
    {
        bool hasA = false;
        bool hasB = false;
        uint64_t tsA = 0;
        uint64_t tsB = 0;
    };

    void HandleMatch(uint64_t sequenceNumber, const SequenceState& state);

    std::unordered_map<uint64_t, SequenceState> pending_;
    std::unordered_set<uint64_t> completed_;
    uint64_t totalPacketsA_ = 0;
    uint64_t totalPacketsB_ = 0;
    uint64_t fasterCountA_ = 0;
    uint64_t fasterCountB_ = 0;
    uint64_t fastestOverallCount_ = 0;
    long double sumAdvantageA_ = 0.0;
    long double sumAdvantageB_ = 0.0;
    long double sumFastestAdvantage_ = 0.0;
};

} // namespace PcapLearn
