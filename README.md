# PcapLearn Feed Arbitration

## Overview

This tool performs low-latency feed arbitration between the two CME MDP 3.0 UDP multicast channels (Side A on port 14310 and Side B on port 15310). It parses raw pcap files with libpcap, extracts Metamako hardware timestamps, matches packets by exchange sequence number, and reports:

- Total packet counts per side
- Unmatched packets per side (by sequence number)
- Which side was faster for matched packets, plus average speed advantages
- Average advantage for the fastest channel overall

The implementation focuses on predictable memory usage, cache-friendly data structures, and minimal per-packet work.

## Building (host)

Requirements: CMake (>=3.16) and a compiler with C++20 support plus libpcap headers (e.g., Apple Clang/Xcode or GCC/Clang on Linux).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running

Provide the directory containing the input `.pcap` files. All `.pcap` files in that directory are processed.

```bash
./build/feed_arbitration path/to/pcap_directory
```

### Example output

Running against the supplied captures in this repository yields:

```
Side A total packets: 22627
Side B total packets: 22627
Side A unmatched packets: 0
Side B unmatched packets: 0
Packets where A faster: 20430
Packets where B faster: 2197
Average advantage when A faster: 2105.07 ns
Average advantage when B faster: 759.91 ns
Average advantage of fastest channel overall: 1974.46 ns
```

## Tests

Unit tests cover the core feed-arbitration logic (matching, duplicate handling, unmatched accounting). Build and run them via CTest:

```bash
cmake --build build --target stats_tests
ctest --test-dir build
```

## Docker workflow

A Dockerfile is provided for reproducible builds/runs. It compiles the project inside Ubuntu and exposes the analyzer as the entry point.

```bash
docker build -t pcap-learn .
docker run --rm -v "$PWD:/data" pcap-learn /app/data
```

Mount any directory containing pcaps at `/app/data` (or pass an explicit directory path after the image name).

## Design notes & assumptions

- Packets are parsed from Ethernet UDP frames with an appended 20-byte Metamako trailer. Timestamp seconds/nanoseconds are treated as big-endian as observed in the provided data.
- Exchange sequence numbers are read as 64-bit little-endian integers at the start of each UDP payload, per the CME SBE header spec.
- Only the first observed packet per side per sequence number participates in matching; subsequent duplicates on the same side are ignored for arbitration but still counted toward total packet statistics. Once both sides for a sequence are seen, further duplicates of that sequence are ignored for matching to keep memory bounded.
- Input directories are scanned non-recursively for `.pcap` files. Extend `PcapProcessor::ShouldParseFile` if additional file types must be supported.
- Future enhancements could include mmap-based pcap parsing, multi-threaded decoding, richer telemetry, and more extensive statistics (e.g., percentile advantages).
