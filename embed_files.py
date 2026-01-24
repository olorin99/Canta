import struct
import sys

def main():
    for arg in sys.argv[1:]:
        print(arg)

    if len(sys.argv) < 2:
        return

    name = sys.argv[1]
    output_path = sys.argv[2]

    shaders = []

    for arg in sys.argv[3:]:
        paths = arg.split(":")
        print(paths)
        virtual_path = paths[0]
        shader_path = paths[1]
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
                print(shader)
            shaders.append([virtual_path, shader_path, shader])

    header_guard = output_path.replace(".", "_").replace("/", "_").replace("-", "_")
    output = "#ifndef {}_H\n#define {}_H\n\n".format(header_guard, header_guard)
    for shader in shaders:
        if shader[0].endswith(".spv"):
            count = len(shader[2].split(","))
            output += "constexpr std::array<const u32, {}> {}_embedded = {{{}}};\n".format(count, shader[0].replace(".", "_").replace("/", "_"), shader[2])
        else:
            output += "constexpr const char* {}_embedded = R\"({})\";\n".format(shader[0].replace(".", "_").replace("/", "_"), shader[2])

    output += "\n"
    output += "#ifndef EMBEDDED_SHADERS_NO_REGISTER\n"
    output += "static inline void registerEmbededShaders{}(canta::PipelineManager& manager) ".format(name) + "{\n"
    for shader in shaders:
        if shader[0].endswith(".spv"):
            continue
        output += "manager.addVirtualFile(R\"({})\", {}_embedded);\n".format(shader[0], shader[0].replace(".", "_").replace("/", "_"))

    output += "}\n#endif\n"
    output += "#endif"

    with open(output_path, "w") as output_file:
        output_file.write(output)

if __name__ == "__main__":
    main()