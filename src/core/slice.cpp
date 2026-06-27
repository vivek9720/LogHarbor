#include "core/slice.h"

namespace logharbor::core {

Result<std::uint8_t> ByteSlice::at(std::size_t pos) const {
    if (pos >= size_) return Result<std::uint8_t>::failure(make_error(ErrorCode::truncated_input, "byte read past end", pos));
    return Result<std::uint8_t>::success(data_[pos]);
}

Result<ByteSlice> ByteSlice::sub(std::size_t pos, std::size_t len) const {
    if (pos > size_ || len > size_ - pos) return Result<ByteSlice>::failure(make_error(ErrorCode::truncated_input, "slice outside buffer", pos));
    return Result<ByteSlice>::success(ByteSlice(data_ + pos, len));
}

Result<std::uint8_t> ByteReader::read_u8() {
    if (remaining() < 1) return Result<std::uint8_t>::failure(make_error(ErrorCode::truncated_input, "expected byte", pos_));
    return Result<std::uint8_t>::success(slice_.data()[pos_++]);
}

Result<std::uint16_t> ByteReader::read_le16() {
    if (remaining() < 2) return Result<std::uint16_t>::failure(make_error(ErrorCode::truncated_input, "expected uint16", pos_));
    std::uint16_t v = static_cast<std::uint16_t>(slice_.data()[pos_]) | (static_cast<std::uint16_t>(slice_.data()[pos_ + 1]) << 8);
    pos_ += 2;
    return Result<std::uint16_t>::success(v);
}

Result<std::uint32_t> ByteReader::read_le32() {
    if (remaining() < 4) return Result<std::uint32_t>::failure(make_error(ErrorCode::truncated_input, "expected uint32", pos_));
    std::uint32_t v = 0;
    for (int i = 0; i < 4; ++i) v |= static_cast<std::uint32_t>(slice_.data()[pos_ + i]) << (8 * i);
    pos_ += 4;
    return Result<std::uint32_t>::success(v);
}

Result<std::uint64_t> ByteReader::read_le64() {
    if (remaining() < 8) return Result<std::uint64_t>::failure(make_error(ErrorCode::truncated_input, "expected uint64", pos_));
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<std::uint64_t>(slice_.data()[pos_ + i]) << (8 * i);
    pos_ += 8;
    return Result<std::uint64_t>::success(v);
}

Result<std::string> ByteReader::read_string(std::size_t n) {
    if (remaining() < n) return Result<std::string>::failure(make_error(ErrorCode::truncated_input, "expected string bytes", pos_));
    std::string out(reinterpret_cast<const char*>(slice_.data() + pos_), n);
    pos_ += n;
    return Result<std::string>::success(out);
}

Result<ByteSlice> ByteReader::read_slice(std::size_t n) {
    if (remaining() < n) return Result<ByteSlice>::failure(make_error(ErrorCode::truncated_input, "expected slice bytes", pos_));
    ByteSlice out(slice_.data() + pos_, n);
    pos_ += n;
    return Result<ByteSlice>::success(out);
}

Result<void> ByteReader::skip(std::size_t n) {
    if (remaining() < n) return Result<void>::failure(make_error(ErrorCode::truncated_input, "cannot skip past end", pos_));
    pos_ += n;
    return Result<void>::success();
}

Result<std::string> ByteReader::read_until(char c, std::size_t limit) {
    std::size_t start = pos_;
    std::size_t seen = 0;
    while (!eof() && seen <= limit) {
        if (static_cast<char>(slice_.data()[pos_]) == c) {
            std::string out(reinterpret_cast<const char*>(slice_.data() + start), pos_ - start);
            ++pos_;
            return Result<std::string>::success(out);
        }
        ++pos_;
        ++seen;
    }
    if (seen > limit) return Result<std::string>::failure(make_error(ErrorCode::invalid_format, "delimiter not found within limit", start));
    return Result<std::string>::failure(make_error(ErrorCode::truncated_input, "delimiter not found", start));
}

void ByteWriter::write_le16(std::uint16_t v) {
    bytes_.push_back(static_cast<std::uint8_t>(v & 0xff));
    bytes_.push_back(static_cast<std::uint8_t>((v >> 8) & 0xff));
}

void ByteWriter::write_le32(std::uint32_t v) {
    for (int i = 0; i < 4; ++i) bytes_.push_back(static_cast<std::uint8_t>((v >> (8 * i)) & 0xff));
}

void ByteWriter::write_le64(std::uint64_t v) {
    for (int i = 0; i < 8; ++i) bytes_.push_back(static_cast<std::uint8_t>((v >> (8 * i)) & 0xff));
}

void ByteWriter::write_bytes(const std::uint8_t* data, std::size_t n) {
    bytes_.insert(bytes_.end(), data, data + n);
}

void ByteWriter::write_string(const std::string& s) {
    write_bytes(reinterpret_cast<const std::uint8_t*>(s.data()), s.size());
}

} // namespace logharbor::core
