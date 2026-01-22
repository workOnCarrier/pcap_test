#include "StatsAggregator.h"

#include <cmath>

namespace PcapLearn
{
void StatsAggregator::ProcessPacket(Side side, uint64_t sequenceNumber, uint64_t timestampNs)
{
    if (side == Side::kA)
    {
        ++totalPacketsA_;
    }
    else
    {
        ++totalPacketsB_;
    }

    if (completed_.find(sequenceNumber) != completed_.end())
    {
        return;
    }

    SequenceState& state = pending_[sequenceNumber];
    if (side == Side::kA)
    {
        if (state.hasA)
        {
            return;
        }
        state.hasA = true;
        state.tsA = timestampNs;
    }
    else
    {
        if (state.hasB)
        {
            return;
        }
        state.hasB = true;
        state.tsB = timestampNs;
    }

    if (state.hasA && state.hasB)
    {
        HandleMatch(sequenceNumber, state);
        pending_.erase(sequenceNumber);
        completed_.insert(sequenceNumber);
    }
}

void StatsAggregator::HandleMatch(uint64_t /*sequenceNumber*/, const SequenceState& state)
{
    const int64_t delta = static_cast<int64_t>(state.tsA) - static_cast<int64_t>(state.tsB);
    if (delta < 0)
    {
        ++fasterCountA_;
        const long double advantage = static_cast<long double>(-delta);
        sumAdvantageA_ += advantage;
        sumFastestAdvantage_ += advantage;
        ++fastestOverallCount_;
    }
    else if (delta > 0)
    {
        ++fasterCountB_;
        const long double advantage = static_cast<long double>(delta);
        sumAdvantageB_ += advantage;
        sumFastestAdvantage_ += advantage;
        ++fastestOverallCount_;
    }
}

StatsSummary StatsAggregator::BuildSummary() const
{
    StatsSummary summary;
    summary.totalPacketsSideA = totalPacketsA_;
    summary.totalPacketsSideB = totalPacketsB_;

    for (const auto& entry : pending_)
    {
        const SequenceState& state = entry.second;
        if (state.hasA && !state.hasB)
        {
            ++summary.unmatchedSideA;
        }
        else if (state.hasB && !state.hasA)
        {
            ++summary.unmatchedSideB;
        }
    }

    summary.fasterCountA = fasterCountA_;
    summary.fasterCountB = fasterCountB_;
    if (fasterCountA_ > 0)
    {
        summary.averageAdvantageA = static_cast<double>(sumAdvantageA_ / fasterCountA_);
    }
    if (fasterCountB_ > 0)
    {
        summary.averageAdvantageB = static_cast<double>(sumAdvantageB_ / fasterCountB_);
    }
    if (fastestOverallCount_ > 0)
    {
        summary.averageFastestAdvantage = static_cast<double>(sumFastestAdvantage_ / fastestOverallCount_);
    }
    return summary;
}

} // namespace PcapLearn
