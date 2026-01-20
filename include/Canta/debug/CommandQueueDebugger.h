#ifndef CANTA_COMMANDQUEUEDEBUGGER_H
#define CANTA_COMMANDQUEUEDEBUGGER_H

namespace canta {

    class Device;
    class CommandQueueDebugger {
    public:

        struct CreateInfo {
            Device* device;
        };

        static auto create(const CreateInfo& info) -> CommandQueueDebugger;

        void render();

    private:

        Device* _device;

    };

}

#endif //CANTA_COMMANDQUEUEDEBUGGER_H