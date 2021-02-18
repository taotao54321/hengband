#include "autoconf.h"
#ifdef USE_SDL2

#include <algorithm>
#include <cerrno>
#include <string>
#include <string_view>
#include <utility>

#include <boost/core/noncopyable.hpp>

#include <iconv.h>

#include "main-sdl2/encoding.hpp"
#include "main-sdl2/prelude.hpp"

namespace {

class EncodingConverter : private boost::noncopyable {
private:
    iconv_t conv_;

public:
    EncodingConverter(const std::string &from, const std::string &to)
        : conv_(iconv_open(to.c_str(), from.c_str()))
    {
        if (conv_ == iconv_t(-1))
            PANIC("iconv_open() failed");
    }

    ~EncodingConverter() { iconv_close(conv_); }

    // (変換後文字列, src 内で変換された Byte 数) を返す。
    [[nodiscard]] std::pair<std::string, std::size_t> convert(const std::string_view src) const
    {
        constexpr std::size_t BUF_SIZE = 1024;

        const auto *inbuf = src.data();
        auto inbytesleft = std::size(src);
        std::string dst;
        dst.reserve(inbytesleft); // 入力と同程度の Byte 数は必要
        std::size_t bytes_converted = 0;

        char buf[BUF_SIZE];
        while (inbytesleft > 0) {
            const auto *inbuf_nxt = inbuf;
            auto *outbuf = buf;
            auto outbytesleft = std::size(buf);

            // iconv() は inbuf_nxt の内容を変更しないので const_cast してよい
            const auto iconv_res = iconv(conv_, const_cast<char **>(&inbuf_nxt), &inbytesleft, &outbuf, &outbytesleft);

            // 変換に成功した分を取得
            const auto n_in = inbuf_nxt - inbuf;
            const auto n_out = std::size(buf) - outbytesleft;
            dst.append(buf, n_out);
            bytes_converted += n_in;

            // 途中で変換が失敗したらそこで打ち切る
            if (iconv_res == std::size_t(-1)) {
                switch (errno) {
                case E2BIG:
                    break; // ignore
                case EILSEQ:
                case EINVAL:
                default:
                    return { dst, bytes_converted };
                }
            }

            inbuf = inbuf_nxt;
        }

        return { dst, bytes_converted };
    }

    // 変換後文字列を返す。
    // src 内の変換できない Byte は ch_replace に置換する。
    [[nodiscard]] std::string convert_lossy(const std::string_view src, const char ch_replace) const
    {
        std::string dst;
        dst.reserve(std::size(src)); // 入力と同程度の Byte 数は必要

        for (std::size_t i = 0; i < std::size(src);) {
            const auto [s, n] = convert(src.substr(i));

            dst.append(s);
            const bool ok = n == std::size(src) - i; // 変換全体が成功したか?
            i += n;

            // 変換に失敗した Byte は ch_replace に置換した上で読み飛ばす
            if (!ok) {
                dst.push_back(ch_replace);
                i += 1;
            }
        }

        return dst;
    }
};

const EncodingConverter &conv_euc_to_utf8()
{
    const static EncodingConverter conv("EUC-JP", "UTF-8");
    return conv;
}

const EncodingConverter &conv_utf8_to_euc()
{
    const static EncodingConverter conv("UTF-8", "EUC-JP");
    return conv;
}

} // anonymous namespace

// clang-format off
std::pair<std::string, std::size_t> euc_to_utf8(const std::string_view euc) {
    return conv_euc_to_utf8().convert(euc);
}

std::pair<std::string, std::size_t> utf8_to_euc(const std::string_view utf8) {
    return conv_utf8_to_euc().convert(utf8);
}

std::string euc_to_utf8_lossy(const std::string_view euc, const char ch_replace) {
    return conv_euc_to_utf8().convert_lossy(euc, ch_replace);
}

std::string utf8_to_euc_lossy(const std::string_view utf8, const char ch_replace) {
    return conv_utf8_to_euc().convert_lossy(utf8, ch_replace);
}
// clang-format on

int utf8_char_byte_count(const std::string_view utf8)
{
    if (utf8.empty())
        return 0;

    const auto first = u8(utf8[0]);

    int n;
    if (first <= 0x7F)
        n = 1;
    else if ((first >> 5) == 0b110)
        n = 2;
    else if ((first >> 4) == 0b1110)
        n = 3;
    else if ((first >> 3) == 0b11110)
        n = 4;
    else
        return 0;

    if (n > int(std::size(utf8)))
        return 0;

    const auto valid = std::all_of(std::begin(utf8) + 1, std::begin(utf8) + n, [](const char ch) { return (u8(ch) >> 6) == 0b10; });
    if (!valid)
        return 0;

    return n;
}

#endif // USE_SDL2
