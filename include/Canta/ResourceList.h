#ifndef CANTA_RESOURCELIST_H
#define CANTA_RESOURCELIST_H

#include <Ende/platform.h>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <cassert>

namespace canta {

    template <typename T, typename List>
    class Handle {
    public:

        struct Data {
            i32 index = -1;
            std::atomic<i32> count = 0;
            std::function<void(i32)> deleter = {};
        };

        static auto create(List* list, Data* data) -> Handle {
            Handle handle = {};
            handle._list = list;
            handle._data = data;
            return handle;
        }

        Handle() = default;
        ~Handle() {
            release();
        }

        Handle(const Handle& rhs)
            : _list(rhs._list),
            _data(rhs._data)
        {
            if (_data)
                ++_data->count;
        }

        auto operator=(const Handle& rhs) -> Handle& {
            if (this == &rhs)
                return *this;

            auto newData = rhs._data;
            if (newData)
                ++newData->count;
            release();
            _list = rhs._list;
            _data = newData;
            return *this;
        }

        auto operator==(const Handle& rhs) noexcept -> bool {
            return _list == rhs._list && _data == rhs._data;
        }

        explicit operator bool() const noexcept {
            return _list && (!_data || _data->index >= 0);
        }

        auto operator*() noexcept -> T& {
            assert(_list && _data);
            return _list->_resources[_data->index]->first;
        }

        auto operator->() noexcept -> T* {
            assert(_list && _data);
            return &_list->_resources[_data->index]->first;
        }

        auto operator*() const noexcept -> const T& {
            assert(_list && _data);
            return _list->_resources[_data->index]->first;
        }

        auto operator->() const noexcept -> const T* {
            assert(_list && _data);
            return &_list->_resources[_data->index]->first;
        }

        auto index() const -> i32 {
            if (_data)
                return _data->index;
            return -1;
        }

        auto count() const -> i32 {
            if (_data)
                return _data->count;
            return 0;
        }

        auto release() -> i32 {
            if (_data) {
                --_data->count;
                if (_data->count < 1) {
                    _data->deleter(_data->index);
                }
                return _data->count;
            }
            return 0;
        }

    private:
        friend List;

        List* _list = nullptr;
        Data* _data = nullptr;

    };

    template <typename T>
    class ResourceList {
    public:
        using ResourceHandle =  Handle<T, ResourceList<T>>;
        using ResourceData = ResourceHandle::Data;

        static auto create(i32 destroyDelay) -> ResourceList<T> {
            ResourceList<T> list = {};
            list._destroyDelay = destroyDelay;
            return list;
        }

        auto allocate() -> i32 {
            i32 index = -1;
            if (!_freeResources.empty()) {
                index = _freeResources.back();
                _freeResources.pop_back();
                _resources[index] = std::make_unique<std::pair<T, ResourceData>>();
                _resources[index]->second.index = index;
                _resources[index]->second.deleter = [this](i32 index) {
                    _destroyQueue.push_back(std::make_pair(_destroyDelay, index));
                };
            } else {
                index = _resources.size();
                _resources.emplace_back(std::make_unique<std::pair<T, ResourceData>>());
                _resources.back()->second.index = index;
                _resources.back()->second.deleter = [this](i32 index) {
                    _destroyQueue.push_back(std::make_pair(_destroyDelay, index));
                };
            }
            return index;
        }

        auto reallocate(ResourceHandle handle) -> ResourceHandle {
            i32 oldIndex = handle.index();
            i32 newIndex = allocate();
            _resources[newIndex].swap(_resources[oldIndex]);
            _resources[newIndex]->second.index = newIndex;
            _resources[oldIndex]->second.index = oldIndex;
            _resources[oldIndex]->second.count = 0;
            _resources[oldIndex]->second.deleter(oldIndex);
            return getHandle(newIndex);
        }

        template <typename Func>
        void clearQueue(Func func) {
            for (auto& destroyInfo : _destroyQueue) {
                if (destroyInfo.first <= 0) {
                    func(_resources[destroyInfo.second]->first);
                    _freeResources.push_back(destroyInfo.second);
                } else
                    --destroyInfo.first;
            }
            _destroyQueue.clear();
        }

        template <typename Func>
        void clearAll(Func func) {
            for (auto& resource : _resources) {
                func(resource->first);
            }
            _resources.clear();
            _destroyQueue.clear();
            _freeResources.clear();
        }

        auto getHandle(i32 index) -> ResourceHandle {
            if (index >= _resources.size())
                return {};
            _resources[index]->second.count++;
            return ResourceHandle::create(this, &_resources[index]->second);
        }

        auto allocated() const -> u32 { return _resources.size(); }

        auto toDestroy() const -> u32 { return _destroyQueue.size(); }

        auto free() const -> u32 { return _freeResources.size(); }

        auto used() const -> u32 { return allocate() - free(); }

    private:
        friend Handle<T, ResourceList<T>>;

        std::vector<std::unique_ptr<std::pair<T, typename Handle<T, ResourceList<T>>::Data>>> _resources = {};
        std::vector<u32> _freeResources = {};
        std::vector<std::pair<i32, i32>> _destroyQueue = {};
        i32 _destroyDelay = 3;

    };

}

#endif //CANTA_RESOURCELIST_H
