#pragma once
// Single-producer / single-consumer lock-based queue.
// Used to pass commands from the stdin thread to the serial-writer.

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

class ThreadSafeQueue {
public:
    void push(std::string item) {
        {
            std::lock_guard lock{mutex_};
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    // Blocks until an item is available or stop() is called.
    // Returns nullopt when stopped and queue is empty.
    std::optional<std::string> pop() {
        std::unique_lock lock{mutex_};
        cv_.wait(lock, [this]{ return !queue_.empty() || stopped_; });
        if (queue_.empty()) return std::nullopt;
        auto item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    void stop() {
        {
            std::lock_guard lock{mutex_};
            stopped_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<std::string>  queue_;
    std::mutex               mutex_;
    std::condition_variable  cv_;
    bool                     stopped_{false};
};
