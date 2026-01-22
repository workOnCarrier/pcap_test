#include "PcapFileReader.h"

#include <arpa/inet.h>

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace PcapLearn
{
namespace
{
constexpr size_t ethernetHeaderLength = 14;
constexpr size_t metamakoTrailerLength = 20;
constexpr uint16_t etherTypeIpv4 = 0x0800;
constexpr uint8_t ipv4Version = 4;
constexpr uint8_t protocolUdp = 17;
constexpr uint16_t sideAPort = 14310;
constexpr uint16_t sideBPort = 15310;

uint16_t ReadBigEndian16(const u_char* buffer)
{
    return static_cast<uint16_t>(buffer[0] << 8 | buffer[1]);
}

uint32_t ReadBigEndian32(const u_char* buffer)
{
    return (static_cast<uint32_t>(buffer[0]) << 24) | (static_cast<uint32_t>(buffer[1]) << 16) |
           (static_cast<uint32_t>(buffer[2]) << 8) | static_cast<uint32_t>(buffer[3]);
}

uint32_t ReadLittleEndian32(const u_char* buffer)
{
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i)
    {
        value |= static_cast<uint32_t>(buffer[i]) << (i * 8);
    }
    return value;
}

std::optional<Side> SideFromPort(uint16_t port)
{
    if (port == sideAPort)
    {
        return Side::kA;
    }
    if (port == sideBPort)
    {
        return Side::kB;
    }
    return std::nullopt;
}

} // namespace

PcapFileReader::PcapFileReader(StatsAggregator& aggregator)
: aggregator_(aggregator)
{
}

void PcapFileReader::ProcessFile(const std::filesystem::path& filePath)
{
    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    std::unique_ptr<pcap_t, decltype(&pcap_close)> handle(pcap_open_offline(filePath.c_str(), errbuf), &pcap_close);
    if (!handle)
    {
        throw std::runtime_error("Failed to open pcap file " + filePath.string() + ": " + errbuf);
    }

    const int linkType = pcap_datalink(handle.get());
    if (linkType != DLT_EN10MB)
    {
        throw std::runtime_error("Unsupported datalink type in file " + filePath.string());
    }

    const u_char* packet = nullptr;
    pcap_pkthdr* header = nullptr;
    while (true)
    {
        const int status = pcap_next_ex(handle.get(), &header, &packet);
        if (status == 1)
        {
            PacketMetadata metadata;
            if (ExtractPacket(header, packet, metadata))
            {
                aggregator_.ProcessPacket(metadata.side, metadata.sequenceNumber, metadata.timestampNs);
            }
        }
        else if (status == 0)
        {
            continue;
        }
        else if (status == -2)
        {
            break;
        }
        else
        {
            std::string errorMessage = pcap_geterr(handle.get());
            throw std::runtime_error("pcap_next_ex failed for file " + filePath.string() + ": " + errorMessage);
        }
    }
}

bool PcapFileReader::ExtractPacket(const pcap_pkthdr* header, const u_char* data, PacketMetadata& output) const
{
    if (header->caplen < ethernetHeaderLength + metamakoTrailerLength)
    {
        return false;
    }

    const u_char* etherTypePtr = data + 12;
    const uint16_t etherType = ReadBigEndian16(etherTypePtr);
    if (etherType != etherTypeIpv4)
    {
        return false;
    }

    if (header->caplen < ethernetHeaderLength + 20 + metamakoTrailerLength)
    {
        return false;
    }

    const u_char* ipHeader = data + ethernetHeaderLength;
    const uint8_t version = static_cast<uint8_t>(ipHeader[0] >> 4);
    if (version != ipv4Version)
    {
        return false;
    }
    const uint8_t ihl = static_cast<uint8_t>(ipHeader[0] & 0x0F);
    const size_t ipHeaderLength = static_cast<size_t>(ihl) * 4;
    if (ipHeaderLength < 20)
    {
        return false;
    }

    const uint8_t protocol = ipHeader[9];
    if (protocol != protocolUdp)
    {
        return false;
    }

    const uint16_t ipTotalLength = ReadBigEndian16(ipHeader + 2);
    if (ipTotalLength < ipHeaderLength + 8)
    {
        return false;
    }
    const size_t minimumFrameLength = ethernetHeaderLength + static_cast<size_t>(ipTotalLength) + metamakoTrailerLength;
    if (header->caplen < minimumFrameLength)
    {
        return false;
    }

    const u_char* udpHeader = ipHeader + ipHeaderLength;
    const size_t udpHeaderOffset = udpHeader - data;
    if (header->caplen < udpHeaderOffset + 8 + metamakoTrailerLength)
    {
        return false;
    }

    const uint16_t udpLength = ReadBigEndian16(udpHeader + 4);
    if (udpLength < 8)
    {
        return false;
    }
    const size_t udpPayloadLength = static_cast<size_t>(udpLength - 8);
    const u_char* udpPayload = udpHeader + 8;
    if (udpPayloadLength < 4)
    {
        return false;
    }

    const uint16_t srcPort = ReadBigEndian16(udpHeader);
    const uint16_t dstPort = ReadBigEndian16(udpHeader + 2);

    std::optional<Side> srcSide = SideFromPort(srcPort);
    std::optional<Side> dstSide = SideFromPort(dstPort);
    if (!srcSide && !dstSide)
    {
        return false;
    }
    if (srcSide && dstSide && *srcSide != *dstSide)
    {
        output.side = *dstSide;
    }
    else if (srcSide)
    {
        output.side = *srcSide;
    }
    else
    {
        output.side = *dstSide;
    }

    const size_t payloadOffset = udpPayload - data;
    const size_t requiredBytes = payloadOffset + 4;
    if (header->caplen < requiredBytes + metamakoTrailerLength)
    {
        return false;
    }

    output.sequenceNumber = ReadLittleEndian32(udpPayload);

    const size_t trailerOffset = header->caplen - metamakoTrailerLength;
    const u_char* trailer = data + trailerOffset;
    const uint32_t seconds = ReadBigEndian32(trailer + 8);
    const uint32_t nanos = ReadBigEndian32(trailer + 12);
    output.timestampNs = static_cast<uint64_t>(seconds) * 1'000'000'000ULL + nanos;
    return true;
}

} // namespace PcapLearn
