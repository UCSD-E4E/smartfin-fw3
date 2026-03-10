#ifndef __SPSC_QUEUE_HPP__
#define __SPSC_QUEUE_HPP__

/**
 * @file spsc_queue.hpp
 * @brief Simple single-producer single-consumer ring buffer.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include <atomic>
#include <cstddef>
#include <utility>

namespace sf::util
{

    /**
     * @brief Lock-free SPSC ring buffer with fixed capacity and atomic
     * indices.
     *
     * Capacity is compile-time and one slot is reserved to distinguish full vs
     * empty. No locking; safe only for one producer thread and one consumer
     * thread.
     *
     * @tparam T Element type stored in the queue.
     * @tparam Capacity Total slots in the ring (must be > 1).
     */
    template <typename T, std::size_t Capacity> class SpscQueue
    {
        static_assert(Capacity > 1, "SpscQueue capacity must be > 1");

    public:
        /** @brief Construct an empty queue (head/tail zeroed). */
        SpscQueue() : head_(0), tail_(0)
        {
        }

        /**
         * @brief Enqueue by copy.
         * @param item Element to copy in.
         * @return true if enqueued, false if full.
         */
        bool push(const T &item)
        {
            const std::size_t next = increment(head_);
            if (next == tail_.load(std::memory_order_acquire))
            {
                return false; // full
            }

            buffer_[head_] = item;
            head_.store(next, std::memory_order_release);
            return true;
        }

        /**
         * @brief Enqueue by move.
         * @param item Element to move in.
         * @return true if enqueued, false if full.
         */
        bool push(T &&item)
        {
            const std::size_t next = increment(head_);
            if (next == tail_.load(std::memory_order_acquire))
            {
                return false; // full
            }

            buffer_[head_] = std::move(item);
            head_.store(next, std::memory_order_release);
            return true;
        }

        /**
         * @brief Dequeue into `out`.
         * @param out Destination for the popped element.
         * @return true if an element was popped, false if empty.
         */
        bool pop(T &out)
        {
            if (empty())
            {
                return false;
            }

            out = std::move(buffer_[tail_]);
            tail_.store(increment(tail_), std::memory_order_release);
            return true;
        }

        /** @brief Check if queue is empty. */
        bool empty() const
        {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
        }

        /** @brief Check if queue is full. */
        bool full() const
        {
            return increment(head_) == tail_.load(std::memory_order_acquire);
        }

        /** @brief Maximum number of storable elements (Capacity - 1). */
        std::size_t capacity() const
        {
            return Capacity - 1;
        }

        /** @brief Current element count. */
        std::size_t size() const
        {
            const std::size_t h = head_.load(std::memory_order_acquire);
            const std::size_t t = tail_.load(std::memory_order_acquire);
            if (h >= t)
            {
                return h - t;
            }
            return Capacity - (t - h);
        }

    private:
        /**
         * @brief Advance an index with wraparound.
         * @param idx Current index.
         * @return Next index modulo Capacity.
         */
        std::size_t increment(std::size_t idx) const
        {
            return (idx + 1) % Capacity;
        }

        T buffer_[Capacity];            //!< Ring storage.
        std::atomic<std::size_t> head_; //!< Producer index (atomic).
        std::atomic<std::size_t> tail_; //!< Consumer index (atomic).
    };

} // namespace sf::util

#endif // __SPSC_QUEUE_HPP__
