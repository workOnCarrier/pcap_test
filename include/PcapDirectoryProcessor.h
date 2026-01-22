#pragma once

#include <filesystem>
#include <string>

namespace flowtraders
{

class PcapFileReader;

class PcapDirectoryProcessor
{
public:
    explicit PcapDirectoryProcessor(PcapFileReader& reader);

    void ProcessDirectory(const std::string& directoryPath);

private:
    static bool ShouldParseFile(const std::filesystem::path& filePath);

    PcapFileReader& reader_;
};

} // namespace flowtraders
