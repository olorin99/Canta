#ifndef CANTA_RESOURCELIST_H
#define CANTA_RESOURCELIST_H

#include <Ende/platform.h>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <cassert>
#include <mutex>

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
            handle._hash = s_hash++;
            return handle;
        }

        Handle() = default;
        ~Handle() {
            release();
        }

        Handle(const Handle& rhs)
            : _list(rhs._list),
            _data(rhs._data),
            _hash(rhs._hash)
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

        auto operator==(const Handle& rhs) const noexcept -> bool {
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

        auto hash() const -> u32 { return _hash; }

    private:
        friend List;

        List* _list = nullptr;
        Data* _data = nullptr;

        u32 _hash = 0;
        static u32 s_hash;

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

        auto allocate() -> ResourceHandle {
            i32 index = -1;
            std::unique_lock lock(_mutex);
            if (!_freeResources.empty()) {
                index = _freeResources.back();
                _freeResources.pop_back();
                _resources[index] = std::make_unique<std::pair<T, ResourceData>>();
                _resources[index]->second.index = index;
                _resources[index]->second.deleter = [this](i32 index) {
                    std::unique_lock lock(_mutex);
                    _destroyQueue.push_back(std::make_pair(_destroyDelay, index));
                };
            } else {
                index = _resources.size();
                _resources.emplace_back(std::make_unique<std::pair<T, ResourceData>>());
                _resources.back()->second.index = index;
                _resources.back()->second.deleter = [this](i32 index) {
                    std::unique_lock lock(_mutex);
                    _destroyQueue.push_back(std::make_pair(_destroyDelay, index));
                };
            }
            return getHandle(index);
        }

        auto reallocate(ResourceHandle handle) -> ResourceHandle {
            i32 oldIndex = handle.index();
            auto newHandle = allocate();
            std::unique_lock lock(_mutex);
            auto newIndex = newHandle.index();
            _resources[newIndex].swap(_resources[oldIndex]);
            _resources[oldIndex]->first = std::move(_resources[newIndex]->first);
            _resources[newIndex]->second.index = newIndex;
            _resources[oldIndex]->second.index = oldIndex;
            return getHandle(newIndex);
        }

        void clearQueue(std::function<void(T&)> func = [](auto& resource) { resource = {}; }) {
            std::unique_lock lock(_mutex);
            for (auto it = _destroyQueue.begin(); it != _destroyQueue.end(); it++) {
                if (it->first <= 0) {
                    func(_resources[it->second]->first);
                    _freeResources.push_back(it->second);
                    _destroyQueue.erase(it--);
                } else
                    --it->first;
            }
        }

        void clearAll(std::function<void(T&)> func = [](auto& resource) { resource = {}; }) {
            std::unique_lock lock(_mutex);
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

        auto used() const -> u32 { return allocated() - free(); }

    private:
        friend Handle<T, ResourceList<T>>;

        std::vector<std::unique_ptr<std::pair<T, typename Handle<T, ResourceList<T>>::Data>>> _resources = {};
        std::vector<u32> _freeResources = {};
        std::vector<std::pair<i32, i32>> _destroyQueue = {};
        i32 _destroyDelay = 3;
        std::mutex _mutex = {};

    };

}

#endif //CANTA_RESOURCELIST_H
