import os.path
import struct
import sys

def main():
    # for arg in sys.argv[1:]:
    #     print(arg)

    if len(sys.argv) < 2:
        return

    name = sys.argv[1].lower()
    output_path = sys.argv[2]

    shaders = []

    for arg in sys.argv[3:]:
        shader_path = arg
        virtual_path = os.path.basename(arg)
        open_params = "r"
        if shader_path.endswith(".spv"):
            open_params += "b"
        with open(shader_path, open_params) as shader_file:
            shader = shader_file.read()
            if shader_path.endswith(".spv"):
                shader_binary = struct.unpack("i" * int((len(shader) / 4)), shader)
                shader = ""
                for op in shader_binary:
                    shader += str(op) + ", "
                shader = shader[:-2]
            shaders.append([virtual_path, shader_path, shader])

    header_guard = output_path.replace(".", "_").replace("/", "_").replace("-", "_")
    output = "#ifndef {}_H\n#define {}_H\n\n".format(header_guard, header_guard)
    output += "#include <Canta/util/KernelHelper.h>\n\n"
    output += "namespace {} {{\n\n".format(name)
    for shader in shaders:
        var_name = "{}".format(shader[0].replace(".", "_").replace("/", "_"))
        if shader[0].endswith(".spv"):
            count = len(shader[2].split(","))
            output += "constexpr std::array<const u32, {}> {}_embedded = {{{}}};\n".format(count, var_name, shader[2])
            output += "inline canta::util::kernel_helper {}(const u32 x = 1, const u32 y = 1, const u32 z = 1) {{\n".format(var_name.replace("_spv", ""))
            output += "return canta::util::kernel_helper(\"{}\", {}_embedded)(x, y, z);\n".format(var_name, var_name)
            output += "}\n"
        else:
            output += "constexpr const char* {}_embedded = R\"({})\";\n".format(var_name, shader[2])

    output += "\n}\n"
    output += "\n"
    output += "#ifndef EMBEDDED_SHADERS_NO_REGISTER\n"
    output += "static inline void registerEmbededShaders{}(canta::PipelineManager& manager) ".format(name.capitalize()) + "{\n"
    for shader in shaders:
        var_name = "{}".format(shader[0].replace(".", "_").replace("/", "_"))
        if shader[0].endswith(".spv"):
            continue
        output += "manager.addVirtualFile(R\"({})\", {}::{}_embedded);\n".format(shader[0], name, var_name)

    output += "}\n#endif\n"
    output += "#endif"

    with open(output_path, "w") as output_file:
        output_file.write(output)

if __name__ == "__main__":
    main()