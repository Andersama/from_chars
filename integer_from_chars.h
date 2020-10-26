#pragma once
#include <system_error>
// Slight variation on the microsoft from_chars header for integers, templated to ignore particular characters
namespace from_chs {
    struct from_chars_result {
        const char *ptr;
        std::errc   ec;
    };

    const constexpr std::array<unsigned char, 256> _integer_digit_table =
        []() -> std::array<unsigned char, 256> {
        std::array<unsigned char, 256> ret{};

        for (size_t i = 0; i < ret.size(); i++)
            ret[i] = 255;

        for (char ch = 'a'; ch <= 'z'; ch++)
            ret[ch] = (ch - 'a') + 10;
        for (char ch = 'A'; ch <= 'Z'; ch++)
            ret[ch] = (ch - 'A') + 10;
        for (char ch = '0'; ch <= '9'; ch++)
            ret[ch] = ch - '0';

        return ret;
    }();

    template <class ty, char... ignored_chars>
    [[nodiscard]] from_chars_result integer_from_chars(const char *const first, const char *const last,
                                                       ty &raw_value) noexcept {
        bool        minus_sign = false;
        const char *next       = first;

        if constexpr (std::is_signed_v<ty>) {
            if (next != last && *next == '-') {
                minus_sign = true;
                ++next;
            }
        }

        using unsigned_int_type = std::make_unsigned_t<ty>;

        constexpr unsigned_int_type uint_max    = static_cast<unsigned_int_type>(-1);
        constexpr unsigned_int_type int_max     = static_cast<unsigned_int_type>(uint_max >> 1);
        constexpr unsigned_int_type abs_int_min = static_cast<unsigned_int_type>(int_max + 1);

        unsigned_int_type risky_value;
        unsigned_int_type max_digit;

        int base = 10;
        // detect 0x and 0b pattern for hex and binary digits and 04474 for octal
        if (next != last && *next == '0') {
            ++next;
            if (next != last && (*next == 'x' || *next == 'X')) {
                base = 16;
                ++next;
            } else if (next != last && (*next == 'b' || *next == 'B')) {
                base = 2;
                ++next;
            } else if (next != last) {
                base = 8;
            }
        }

        if constexpr (std::is_signed_v<ty>) {
            if (minus_sign) {
                risky_value = static_cast<unsigned_int_type>(abs_int_min / base);
                max_digit   = static_cast<unsigned_int_type>(abs_int_min % base);
            } else {
                risky_value = static_cast<unsigned_int_type>(int_max / base);
                max_digit   = static_cast<unsigned_int_type>(int_max % base);
            }
        } else {
            risky_value = static_cast<unsigned_int_type>(uint_max / base);
            max_digit   = static_cast<unsigned_int_type>(uint_max % base);
        }

        unsigned_int_type value = 0;

        bool overflowed_int = false;

        for (; next != last; ++next) {
            char ch = *next;
            if (((ignored_chars == ch) || ...)) //skip if an ignored character... codegen here is...somewhat meh
                continue;
            const unsigned char digit = _integer_digit_table[ch];

            if (digit >= base) {
                break;
            }

            if (value < risky_value                                // never overflows
                || (value == risky_value && digit <= max_digit)) { // overflows for certain digits
                value = static_cast<unsigned_int_type>(value * base + digit);
            } else { // _Value > _Risky_val always overflows
                overflowed_int =
                    true; // keep going, _Next still needs to be updated, _Value is now irrelevant
            }
        }

        if (next - first == static_cast<ptrdiff_t>(minus_sign)) {
            return {first, std::errc::invalid_argument};
        }

        if (overflowed_int) {
            return {next, std::errc::result_out_of_range};
        }

        if constexpr (std::is_signed_v<ty>) {
            if (minus_sign) {
                value = static_cast<unsigned_int_type>(0 - value);
            }
        }

        raw_value =
            static_cast<ty>(value); // implementation-defined for negative, N4713 7.8 [conv.integral]/3

        return {next, std::errc{}};
    }

    template <class ty, char... ignored_chars>
    [[nodiscard]] from_chars_result integer_from_chars_table(const char *const first, const char *const last,
                                                             ty &raw_value) noexcept {
        bool        minus_sign = false;
        const char *next       = first;

        if constexpr (std::is_signed_v<ty>) {
            if (next != last && *next == '-') {
                minus_sign = true;
                ++next;
            }
        }
        // using basic_type = std::remove_cvref<ty>;
        // using bool_shunt = std::conditional_t<std::is_same<basic_type,bool>::value,uint32_t,basic_type>;
        // because make unsigned complains about bool
        using unsigned_int_type = std::make_unsigned_t<ty>;

        constexpr unsigned_int_type uint_max    = static_cast<unsigned_int_type>(-1);
        constexpr unsigned_int_type int_max     = static_cast<unsigned_int_type>(uint_max >> 1);
        constexpr unsigned_int_type abs_int_min = static_cast<unsigned_int_type>(int_max + 1);

        unsigned_int_type risky_value;
        unsigned_int_type max_digit;

        int base = 10;
        // detect 0x and 0b pattern for hex and binary digits and 04474 for octal
        if (next != last && *next == '0') {
            ++next;
            if (next != last && (*next == 'x' || *next == 'X')) {
                base = 16;
                ++next;
            } else if (next != last && (*next == 'b' || *next == 'B')) {
                base = 2;
                ++next;
            } else if (next != last) {
                base = 8;
            }
        }

        if constexpr (std::is_signed_v<ty>) {
            if (minus_sign) {
                risky_value = static_cast<unsigned_int_type>(abs_int_min / base);
                max_digit   = static_cast<unsigned_int_type>(abs_int_min % base);
            } else {
                risky_value = static_cast<unsigned_int_type>(int_max / base);
                max_digit   = static_cast<unsigned_int_type>(int_max % base);
            }
        } else {
            risky_value = static_cast<unsigned_int_type>(uint_max / base);
            max_digit   = static_cast<unsigned_int_type>(uint_max % base);
        }

        unsigned_int_type value          = 0;
        bool              overflowed_int = false;
        // static changes where this array is stored*, good idea?
        static const constexpr std::array<unsigned char, 256> tbl = []() -> std::array<unsigned char, 256> {
            std::array<unsigned char, 256> ret{};

            for (size_t i = 0; i < ret.size(); i++)
                ret[i] = 254;

            for (char ch = 'a'; ch <= 'z'; ch++)
                ret[ch] = (ch - 'a') + 10;
            for (char ch = 'A'; ch <= 'Z'; ch++)
                ret[ch] = (ch - 'A') + 10;
            for (char ch = '0'; ch <= '9'; ch++)
                ret[ch] = ch - '0';
            //here we be cheeky, we encode a sentinel
            ((ret[ignored_chars] = 255), ...);

            return ret;
        }();
        for (; next != last; ++next) {
            char                ch    = *next;
            const unsigned char digit = tbl[ch];
            if (digit == 255) // if our sentinel we loop
                continue;
            if (digit >= base)
                break;
            if (value < risky_value                                // never overflows
                || (value == risky_value && digit <= max_digit)) { // overflows for certain digits
                value = static_cast<unsigned_int_type>(value * base + digit);
            } else {
                overflowed_int =
                    true; // keep going, _Next still needs to be updated, _Value is now irrelevant
            }
        }

        if (next - first == static_cast<ptrdiff_t>(minus_sign)) {
            return {first, std::errc::invalid_argument};
        }

        if (overflowed_int) {
            return {next, std::errc::result_out_of_range};
        }

        if constexpr (std::is_signed_v<ty>) {
            if (minus_sign) {
                value = static_cast<unsigned_int_type>(0 - value);
            }
        }

        raw_value =
            static_cast<ty>(value); // implementation-defined for negative, N4713 7.8 [conv.integral]/3

        return {next, std::errc{}};
    }
} // namespace from_chs