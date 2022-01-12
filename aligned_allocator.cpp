#include <iostream>
#include <limits>
#include <memory>

// standard in C++23
template< class Pointer >
struct allocation_result {
    Pointer ptr;
    std::size_t count;
};

template <typename T>
struct aligned_allocator {
    using value_type = T;
    std::size_t alignment;

    aligned_allocator(): alignment(1024) {};
    aligned_allocator(std::size_t alignment): alignment(alignment) {};

    template<typename U>
    aligned_allocator(const aligned_allocator<U>& other): alignment(other.alignment){ };

    [[nodiscard]] constexpr value_type* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }

        if (n > std::numeric_limits<size_t>::max() / sizeof(value_type)){
            throw std::bad_array_new_length();
        }

        const std::size_t aligned_size = alignment * ((n * sizeof(value_type) + alignment - 1) / alignment);
        void* const pointer = aligned_alloc(alignment, aligned_size);

        if (pointer == NULL) {
            throw std::bad_alloc();
        }

        return static_cast<value_type*>(pointer);
    }

    [[nodiscard]] constexpr allocation_result<value_type*> allocate_at_least(std::size_t n)
    {
        const std::size_t aligned_size = alignment * ((n * sizeof(value_type) + alignment - 1) / alignment);
        return allocation_result<value_type*>{allocate(aligned_size), aligned_size / sizeof(value_type)};
    }

    constexpr void deallocate(value_type* p, std::size_t n [[maybe_unused]]) { free(p); }
};

template <typename T1, typename T2>
constexpr bool operator==(const aligned_allocator<T1>& lhs, const aligned_allocator<T2>& rhs) noexcept
{
    return lhs.alignment == rhs.alignment;
}

int main()
{
    constexpr int len = 2;

    aligned_allocator<int> alloc;

    allocation_result<int*> result = alloc.allocate_at_least(len);

    alloc.deallocate(result.ptr, 0);

    std::cout << "Count " << result.count << std::endl;

    auto shared_ptr = std::allocate_shared<int>(alloc);

    using traits_t = std::allocator_traits<decltype(alloc)>;
}

