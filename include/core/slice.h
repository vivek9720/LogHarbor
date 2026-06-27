#pragma once
#include "core/result.h"
#include <cstdint>
#include <string>
#include <vector>

namespace logharbor::core {

class ByteSlice {
    const std::uint8_t* data_{nullptr};
    std::size_t size_{0};
public:
    ByteSlice() = default;
    ByteSlice(const std::uint8_t* data, std::size_t size) : data_(data), size_(size) {}
    explicit ByteSlice(const std::string& s) : data_(reinterpret_cast<const std::uint8_t*>(s.data())), size_(s.size()) {}
    const std::uint8_t* data() const { return data_; }
    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    Result<std::uint8_t> at(std::size_t pos) const;
    Result<ByteSlice> sub(std::size_t pos, std::size_t len) const;
    std::string str() const { return std::string(reinterpret_cast<const char*>(data_), size_); }
};

class ByteReader {
    ByteSlice slice_;
    std::size_t pos_{0};
public:
    ByteReader() = default;
    explicit ByteReader(ByteSlice s) : slice_(s) {}
    std::size_t position() const { return pos_; }
    std::size_t remaining() const { return pos_ <= slice_.size() ? slice_.size() - pos_ : 0; }
    bool eof() const { return remaining() == 0; }
    Result<std::uint8_t> read_u8();
    Result<std::uint16_t> read_le16();
    Result<std::uint32_t> read_le32();
    Result<std::uint64_t> read_le64();
    Result<std::string> read_string(std::size_t n);
    Result<ByteSlice> read_slice(std::size_t n);
    Result<void> skip(std::size_t n);
    Result<std::string> read_until(char c, std::size_t limit);
};

class ByteWriter {
    std::vector<std::uint8_t> bytes_;
public:
    void write_u8(std::uint8_t v) { bytes_.push_back(v); }
    void write_le16(std::uint16_t v);
    void write_le32(std::uint32_t v);
    void write_le64(std::uint64_t v);
    void write_bytes(const std::uint8_t* data, std::size_t n);
    void write_string(const std::string& s);
    const std::vector<std::uint8_t>& bytes() const { return bytes_; }
    std::vector<std::uint8_t> take() { return std::move(bytes_); }
    void clear() { bytes_.clear(); }
};

} // namespace logharbor::core
