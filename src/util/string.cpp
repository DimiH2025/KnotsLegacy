// Copyright (c) 2019-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <util/string.h>

#include <string>

namespace util {
void ReplaceAll(std::string& in_out, const std::string& search, const std::string& substitute)
{
    if (search.empty()) return;
    // Literal, non-recursive replacement. `substitute` is plain text and is never
    // interpreted as a format string (a std::regex_replace-based implementation
    // would expand sequences like $', $&, $` and $1 inside it, which allowed a
    // crafted wallet name to alter the -walletnotify command). Scanning resumes
    // after the inserted text, so a substitute containing `search` is not itself
    // re-replaced.
    std::string::size_type pos = 0;
    while ((pos = in_out.find(search, pos)) != std::string::npos) {
        in_out.replace(pos, search.length(), substitute);
        pos += substitute.length();
    }
}
} // namespace util
