#pragma once

#include "StatsAggregator.h"

#include <pcap/pcap.h>

#include <filesystem>

namespace flowtraders
{

class PcapFileReader
{
public:
    explicit PcapFileReader(StatsAggregator& aggregator);

    void ProcessFile(const std::filesystem::path& filePath);

private:
    struct PacketMetadata
    {
        Side side;
        uint64_t sequenceNumber;
        uint64_t timestampNs;
    };

    bool ExtractPacket(const pcap_pkthdr* header, const u_char* data, PacketMetadata& output) const;

    StatsAggregator& aggregator_;
};

} // namespace flowtraders
