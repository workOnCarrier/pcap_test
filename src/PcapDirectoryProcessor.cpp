#include "PcapDirectoryProcessor.h"

#include "PcapFileReader.h"

#include <cctype>
#include <filesystem>
#include <stdexcept>

namespace flowtraders
{

PcapDirectoryProcessor::PcapDirectoryProcessor(PcapFileReader& reader)
: reader_(reader)
{
}

void PcapDirectoryProcessor::ProcessDirectory(const std::string& directoryPath)
{
    namespace fs = std::filesystem;
    fs::path root(directoryPath);
    if (!fs::exists(root))
    {
        throw std::runtime_error("Input directory does not exist: " + directoryPath);
    }
    if (!fs::is_directory(root))
    {
        throw std::runtime_error("Input path is not a directory: " + directoryPath);
    }

    bool processedAny = false;
    for (const auto& entry : fs::directory_iterator(root))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        if (!ShouldParseFile(entry.path()))
        {
            continue;
        }
        processedAny = true;
        reader_.ProcessFile(entry.path());
    }

    if (!processedAny)
    {
        throw std::runtime_error("No pcap files found in directory: " + directoryPath);
    }
}

bool PcapDirectoryProcessor::ShouldParseFile(const std::filesystem::path& filePath)
{
    auto extension = filePath.extension().string();
    for (auto& ch : extension)
    {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return extension == ".pcap";
}

} // namespace flowtraders
