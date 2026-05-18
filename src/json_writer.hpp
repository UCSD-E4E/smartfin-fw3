/**
 * @file json_writer.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Lightweight JSON serializer writing into a fixed-size char buffer.
 * @date 2026-05-17
 */
#ifndef __JSON_WRITER_HPP__
#define __JSON_WRITER_HPP__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace sf
{

/**
 * @brief Streaming JSON writer backed by a caller-supplied char buffer.
 *
 * Drop-in replacement for `spark::JSONBufferWriter`. Emits ASCII JSON
 * directly into the buffer passed at construction with no heap allocation.
 * Commas between object members are inserted automatically.
 *
 * Typical usage:
 * @code
 * char buf[256];
 * sf::JSONBufferWriter w(buf, sizeof(buf));
 * w.beginObject()
 *  .name("x").value(1)
 *  .name("y").value(2.5)
 *  .endObject();
 * @endcode
 *
 * @note Output is always null-terminated. Content that would overflow the
 * buffer is silently dropped; the written portion remains valid JSON up to
 * the point where the buffer filled.
 *
 * @note Maximum object nesting depth is 16.
 */
class JSONBufferWriter
{
public:
    /**
     * @brief Construct a writer targeting @p buf.
     *
     * @param buf  Destination buffer. Must remain valid for the lifetime of
     *             this writer.
     * @param len  Total capacity of @p buf in bytes, including the null
     *             terminator.
     */
    JSONBufferWriter(char* buf, size_t len) : buf_(buf), len_(len), pos_(0), depth_(0)
    {
        memset(needs_comma_, 0, sizeof(needs_comma_));
        if (len_ > 0)
        {
            buf_[0] = '\0';
        }
    }

    /**
     * @brief Reset the writer to the beginning of the buffer.
     *
     * Clears all internal state so the same buffer can be reused for a new
     * JSON document without reallocating.
     */
    void reset()
    {
        pos_ = 0;
        depth_ = 0;
        memset(needs_comma_, 0, sizeof(needs_comma_));
        if (len_ > 0)
        {
            buf_[0] = '\0';
        }
    }

    /**
     * @brief Open a JSON object with `{`.
     *
     * Must be matched by a corresponding @c endObject().
     *
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& beginObject()
    {
        append('{');
        if (depth_ < MAX_DEPTH)
        {
            depth_++;
        }
        needs_comma_[depth_] = false;
        return *this;
    }

    /**
     * @brief Close the current JSON object with `}`.
     *
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& endObject()
    {
        append('}');
        if (depth_ > 0)
        {
            depth_--;
        }
        needs_comma_[depth_] = true;
        return *this;
    }

    /**
     * @brief Write a JSON object key.
     *
     * Automatically inserts a comma separator if this is not the first
     * member in the enclosing object. Must be followed by a @c value()
     * call or @c beginObject().
     *
     * @param key Null-terminated key string. Must not be @c nullptr.
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& name(const char* key)
    {
        if (needs_comma_[depth_])
        {
            append(',');
        }
        append('"');
        append(key ? key : "");
        append('"');
        append(':');
        needs_comma_[depth_] = false;
        return *this;
    }

    /**
     * @brief Write a JSON string value.
     *
     * @param s Null-terminated string. @c nullptr is written as an empty
     *          string.
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& value(const char* s)
    {
        append('"');
        append(s ? s : "");
        append('"');
        needs_comma_[depth_] = true;
        return *this;
    }

    /**
     * @brief Write a JSON integer value.
     *
     * @param v Integer to serialize.
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& value(int v)
    {
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%d", v);
        append(tmp);
        needs_comma_[depth_] = true;
        return *this;
    }

    /**
     * @brief Write a JSON unsigned integer value.
     *
     * @param v Unsigned integer to serialize.
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& value(unsigned int v)
    {
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%u", v);
        append(tmp);
        needs_comma_[depth_] = true;
        return *this;
    }

    /**
     * @brief Write a JSON unsigned long value.
     *
     * Resolves ambiguity on ARM targets where @c uint32_t maps to
     * @c unsigned long rather than @c unsigned int.
     *
     * @param v Unsigned long to serialize.
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& value(unsigned long v)
    {
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%lu", v);
        append(tmp);
        needs_comma_[depth_] = true;
        return *this;
    }

    /**
     * @brief Write a JSON floating-point value.
     *
     * Uses `%g` format, which suppresses trailing zeros and switches to
     * scientific notation for very large or very small values.
     *
     * @param v Value to serialize.
     * @return Reference to @c *this for method chaining.
     */
    JSONBufferWriter& value(double v)
    {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%g", v);
        append(tmp);
        needs_comma_[depth_] = true;
        return *this;
    }

private:
    static constexpr uint8_t MAX_DEPTH = 16;

    /**
     * @brief Append a null-terminated string to the buffer.
     * @param s String to append.
     */
    void append(const char* s)
    {
        if (!s || pos_ >= len_ - 1)
        {
            return;
        }
        size_t remaining = len_ - 1 - pos_;
        size_t n = strnlen(s, remaining);
        memcpy(buf_ + pos_, s, n);
        pos_ += n;
        buf_[pos_] = '\0';
    }

    /**
     * @brief Append a single character to the buffer.
     * @param c Character to append.
     */
    void append(char c)
    {
        if (pos_ >= len_ - 1)
        {
            return;
        }
        buf_[pos_++] = c;
        buf_[pos_] = '\0';
    }

    char* buf_;
    size_t len_;
    size_t pos_;
    uint8_t depth_;
    bool needs_comma_[MAX_DEPTH + 1];
};

} // namespace sf

#endif // __JSON_WRITER_HPP__
