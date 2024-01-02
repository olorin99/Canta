#include <Canta/Device.h>

int main() {

    auto device = canta::Device::create({
        .applicationName = "hello_triangle"
    });

    return 0;
}