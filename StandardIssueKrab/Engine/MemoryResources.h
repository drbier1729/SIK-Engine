#pragma once

/*
* IMPORTANT: None of these are thread-safe!!!
*/

/*
* This class can be constructed by passing another memory resource to its
* constructor in order to log calls to do_allocate and do_deallocate.
*/
class DebugMemoryResource final : public std::pmr::memory_resource
{
public:
    enum class Flag {
        TrackOnly = 0,
        LogAndTrack = 1
    };

public:
    explicit DebugMemoryResource(const char* name, Flag flag, 
        std::pmr::memory_resource* upstream = std::pmr::null_memory_resource());

    ~DebugMemoryResource() noexcept;

    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override;
    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override;
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

private:
    std::pmr::memory_resource* upstream_;
    const char* name_;
    SizeT currently_allocated_bytes_;
    Flag flag_;
};


/*
* This class manages, but does not own, a chunk of memory such that allocations
* are done linearly (one after the other). There are two differences between 
* this and a monotonic_buffer: 
*   1) this provides a Clear method which frees the entire buffer and prepares 
*       it for reuse.
*   2) if the buffer is full, this will just throw std::bad_alloc
* 
* Note that Clear does NOT call destructors for objects allocated in the
* buffer -- if you want this you must call delete_object from the allocator used
* to create it for each object you want to destroy.
*/
class LinearMemoryResource final : public std::pmr::memory_resource
{
public:
    LinearMemoryResource() = default;
    
    explicit LinearMemoryResource(void* data, 
        std::size_t capacity);
    
    ~LinearMemoryResource() override { Clear(); }

    // No destructors called on allocated objects. If this is necessary, it is
    // left up to the caller to ensure all objects are properly destroyed.
    inline void Clear(void) noexcept {
        std::memset(data_, 0, capacity_);
        top_ = data_;
    }

    // Returns: total available space in bytes for the buffer
    inline std::size_t Capacity() const noexcept {
        return capacity_;
    }

    // Returns: current number of bytes used
    inline std::size_t Size() const noexcept {
        return static_cast<std::size_t>(top_ - data_);
    }

    // No destructors called on allocated objects. If this is necessary, it is
    // left up to the caller to ensure all objects are properly destroyed.
    inline void Reset(void* new_data, std::size_t new_capacity) noexcept {
        if (Size() > 0) { Clear(); }
        data_ = static_cast<std::byte*>(new_data);
        top_ = data_;
        capacity_ = new_capacity;
    }

private:
    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override;
    
    void do_deallocate(void*, std::size_t, std::size_t) override {}
    
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
    
private:
    std::byte* data_ = nullptr;
    std::byte* top_ = nullptr;
    std::size_t capacity_ = 0ull;
};


/*
* A resource which will allocate from a collection of "pools" which are each 
* contiguous buffers of fixed-size memory blocks. An object allocated using 
* this resource will be placed in the pool which best fits its size.
* 
* Has a default max allocation ("largest block") size of 1024 bytes, although 
* this can be tuned by providing Opts to the constructor. Opts is a typedef for
* std::pmr::pool_options which is defined as:
* 
*   struct pool_options
*   {
*       std::size_t max_blocks_per_chunk = 0;
*       std::size_t largest_required_pool_block = 0;
*   }
* 
* PoolMemoryResource must be provided an upstream resource -- good candidates 
* for this are a monotonic_buffer, LinearMemoryResource, or ChunkMemoryResource.
*/
class PoolMemoryResource final : public std::pmr::memory_resource
{
public:
    using Opts = std::pmr::pool_options;

public:
    explicit PoolMemoryResource(std::pmr::memory_resource* upstream)
        : pool_{ 
                Opts{.largest_required_pool_block = 1024ull},
                std::forward<std::pmr::memory_resource*>(upstream) 
        }
    {}

    explicit PoolMemoryResource(const Opts& opts, std::pmr::memory_resource* upstream)
        : pool_{
                std::forward<const Opts&>(opts),
                std::forward<std::pmr::memory_resource*>(upstream)
        }
    {}

private:
    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        
        return pool_.allocate(std::forward<std::size_t>(bytes),
            std::forward<std::size_t>(alignment));
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override {
        
        pool_.deallocate(std::forward<void*>(ptr),
            std::forward<std::size_t>(bytes), 
            std::forward<std::size_t>(alignment));
    }
    
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return pool_.is_equal(other);
    }

private:
    std::pmr::unsynchronized_pool_resource pool_;
};

/*
* Manages, but does not own, a block of memory by dividing it into NumBuffers 
* equally sized chunks which are each managed by a LinearMemoryResource. Objects
* allocated into a buffer will be valid for num_buffer calls of swap_buffer, at 
* which point they will be freed by a call to LinearMemoryResource::Clear().
*
* Primary use case is for storing objects for NumBuffer frames.
* 
* Note: if capacity_ is not divisible by NumBuffers, there will wasted space
* equal to capacity_ % num_buffers bytes.
*/
template<std::size_t num_buffers>
class MultiBufferMemoryResource final : public std::pmr::memory_resource
{
public:
    /*
    * Note : if capacity_ is not divisible by NumBuffers, there will wasted space
    * equal to capacity_% NumBuffers bytes.
    */
    explicit MultiBufferMemoryResource(void* data, std::size_t capacity)
        : current_idx_{ 0ull }
    {
        std::byte* const mem = reinterpret_cast<std::byte*>(data);
        std::size_t const buf_cap = capacity / num_buffers;

        for (std::size_t i = 0ull; i < num_buffers; ++i) {
            resources_[i] = LinearMemoryResource{ mem + (i*buf_cap), buf_cap };
        }
    }

    void SwapBuffers() noexcept {
        current_idx_ = (current_idx_ + 1ull) % num_buffers;
        resources_[current_idx_].Clear();
    }

    std::size_t BufferCapacity() const noexcept {
        return resources_[0].Capacity();
    }

    std::size_t CurrentBufferSize() const noexcept {
        return resources_[current_idx_].Size();
    }

    static constexpr std::size_t BufferCount() noexcept {
        return num_buffers;
    }

private:
    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        return resources_[current_idx_].allocate(bytes, alignment);
    }
    
    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override {}
    
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }


private:
    std::array<LinearMemoryResource, num_buffers> resources_;
    std::size_t current_idx_;
};

/*
* A resource which allocates chunks from its upstream resource. Chunk size grows
* geometrically, starting with initial_capacity. Note that allocations within
* each chunk occur linearly and deallocate is a no-op. Therefore, you should use
* this as an upstream for another resource (e.g. a PoolMemoryResource) if you 
* would like to manage memory within each chunk more carefully.
*/
class ChunkMemoryResource final : public std::pmr::memory_resource
{
public:
    explicit ChunkMemoryResource(std::size_t initial_capacity, std::pmr::memory_resource* upstream)
        : mono_buf_{ 
                std::forward<std::size_t>(initial_capacity), 
                std::forward<std::pmr::memory_resource*>(upstream) 
        }
    {}

private:
    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        return mono_buf_.allocate(std::forward<std::size_t>(bytes),
            std::forward<std::size_t>(alignment));
    }

    void do_deallocate(void*, std::size_t, std::size_t) override {}

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return mono_buf_.is_equal(other);
    }

private:
    std::pmr::monotonic_buffer_resource mono_buf_;
};