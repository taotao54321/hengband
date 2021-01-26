#include "autoconf.h"
#ifdef USE_SDL2

#include <cerrno>
#include <optional>
#include <string>

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

    [[nodiscard]] std::optional<std::string> convert(const std::string &src) const
    {
        constexpr std::size_t BUF_SIZE = 1024;

        const auto *inbuf = src.data();
        auto inbytesleft = std::size(src);
        std::string dst;
        dst.reserve(inbytesleft); // 入力と同程度の Byte 数は必要

        char buf[BUF_SIZE];
        while (inbytesleft > 0) {
            auto *outbuf = buf;
            auto outbytesleft = std::size(buf);

            // iconv() は inbuf の内容を変更しないので const_cast してよい
            const auto n = iconv(conv_, const_cast<char **>(&inbuf), &inbytesleft, &outbuf, &outbytesleft);
            if (n == std::size_t(-1)) {
                switch (errno) {
                case E2BIG:
                    break; // ignore
                case EILSEQ:
                case EINVAL:
                default:
                    return std::nullopt;
                }
            }

            dst.append(buf, std::size(buf) - outbytesleft);
        }

        return dst;
    }
};

} // anonymous namespace

std::optional<std::string> euc_to_utf8(const std::string &euc)
{
    const static EncodingConverter conv("EUC-JP", "UTF-8");
    return conv.convert(euc);
}

std::optional<std::string> utf8_to_euc(const std::string &utf8)
{
    const static EncodingConverter conv("UTF-8", "EUC-JP");
    return conv.convert(utf8);
}

#endif // USE_SDL2
