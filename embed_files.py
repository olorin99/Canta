
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
        with open(shader_path, "r") as shader_file:
            shaders.append([virtual_path, shader_path, shader_file.read()])

    output = "static inline void registerEmbededShaders{}(canta::PipelineManager& manager) ".format(name) + "{\n"
    for shader in shaders:
        output += "manager.addVirtualFile(R\"({})\", R\"({})\");\n".format(shader[0], shader[2])

    output += "}"

    with open(output_path, "w") as output_file:
        output_file.write(output)

if __name__ == "__main__":
    main()