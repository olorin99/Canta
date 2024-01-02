#ifndef CANTA_UTIL_H
#define CANTA_UTIL_H

#include <Ende/platform.h>
#include <expected>
#include <vector>
#include <string>
#include <string_view>
#include <span>
#include <Canta/Enums.h>

namespace canta::util {

    struct Macro {
        std::string name;
        std::string value;
    };

    auto compileGLSLToSpirv(std::string_view name, std::string_view glsl, ShaderStage stage, std::span<Macro> macros = {}) -> std::expected<std::vector<u32>, std::string>;

}

#endif //CANTA_UTIL_H
