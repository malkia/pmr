#define _ENABLE_EXTENDED_ALIGNED_STORAGE 1
#include <memory_resource>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstdlib>

#ifdef _MSC_VER
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

struct A
{
    float x;
    float y;
    float z;
};

struct alignas(__STDCPP_DEFAULT_NEW_ALIGNMENT__*2) AA : public A
{
};

void* operator new(std::size_t s) noexcept
{
    auto p = malloc(s);
    std::cout << "  New " << s << " bytes -> " << p << std::endl;
    return p;
}

void* operator new(std::size_t s, const std::nothrow_t& tag) noexcept
{
    auto p = malloc(s);
    std::cout << "  New(nothrow_t) " << s << " bytes -> " << p << std::endl;
    return p;
}

void* operator new[](std::size_t s) noexcept
{
    auto p = malloc(s);
    std::cout << "  New[] " << s << " bytes -> " << p << std::endl;
    return p;
}

void* operator new(std::size_t s, std::align_val_t a) noexcept
{
#ifdef _MSC_VER
    auto p = _aligned_malloc( s, std::size_t(a) );
#else
    auto p = std::aligned_alloc( s, std::size_t(a) );
#endif
    std::cout << "  New aligned " << s << " bytes, " << "align " << std::size_t(a) << " -> " << p << std::endl;
    return p;
}

void* operator new(std::size_t s, std::align_val_t a, const std::nothrow_t& tag) noexcept
{
#ifdef _MSC_VER
    auto p = _aligned_malloc( s, std::size_t(a) );
#else
    auto p = std::aligned_alloc( s, std::size_t(a) );
#endif
    std::cout << "  New aligned (nothrow_t) " << s << " bytes, " << "align " << std::size_t(a) << " -> " << p << std::endl;
    return p;
}

void* operator new[](std::size_t s, std::align_val_t a) noexcept
{
#ifdef _MSC_VER
    auto p = _aligned_malloc( s, std::size_t(a) );
#else
    auto p = std::aligned_alloc( s, std::size_t(a) );
#endif
    std::cout << "  New[] aligned " << s << " bytes, " << "align " << std::size_t(a) << " -> " << p << std::endl;
    return p;
}

void operator delete(void* p)
{
    std::cout << "  Delete " << p << std::endl;
    free(p);
}

void operator delete(void* p, std::align_val_t a)
{
    std::cout << "  Delete aligned " << p << " align " << std::size_t(a) << std::endl;
#ifdef _MSC_VER
    _aligned_free( p );
#else
    // TODO: Double free bug?
    //free(p);
#endif
}

// Stolen from https://github.com/camio/pmr_sandbox/blob/master/simplicity.cpp
class LoggingResource : public std::pmr::memory_resource {
public:
  LoggingResource(std::pmr::memory_resource *underlyingResource)
      : d_underlyingResource(underlyingResource) {}

private:
  std::pmr::memory_resource *d_underlyingResource;

  void *do_allocate(size_t bytes, size_t align) override {
    std::cout << "Allocating " << bytes << " bytes, " << align << " alignment"<< std::endl;
    return d_underlyingResource->allocate(bytes, align);
  }
  void do_deallocate(void *p, size_t bytes, size_t align) override {
    std::cout << "Deallocating " << bytes << " bytes" << std::endl;
    return d_underlyingResource->deallocate(p, bytes, align);
  }
  bool do_is_equal(memory_resource const &other) const noexcept override {
    return d_underlyingResource->is_equal(other);
  }
};

namespace std::pmr {
template <class T, class... Args>
::std::shared_ptr<T> make_shared(Args&&... args)
{
  return ::std::allocate_shared<T, ::std::pmr::polymorphic_allocator<::std::byte>>(
    ::std::pmr::get_default_resource(), 
    ::std::forward<Args>(args)...
  );
}

template <class T, class... Args>
::std::shared_ptr<T> make_shared_from(std::pmr::memory_resource* mr, Args&&... args)
{
  return ::std::allocate_shared<T, ::std::pmr::polymorphic_allocator<::std::byte>>(
    mr,
    ::std::forward<Args>(args)...
  );
}
}

int main(int argc, const char *argv[])
{
#ifdef _MSC_VER
#ifdef _DEBUG
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
  _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  std::cout << "MSVCRT: Enabling debug features..." << std::endl;
#endif
#endif
    // Allocated with global new/delete
    auto a = std::make_shared<A>();
    auto aa = std::make_shared<AA>();

    auto mr = new LoggingResource(std::pmr::get_default_resource());
    auto b0 = std::allocate_shared<A, std::pmr::polymorphic_allocator<std::byte>>(mr);
    auto b0a = std::allocate_shared<AA, std::pmr::polymorphic_allocator<std::byte>>(mr);
    auto b1 = std::pmr::make_shared<A>();
    auto b1a = std::pmr::make_shared<AA>();
    auto b2 = std::pmr::make_shared_from<A>(mr);
    auto b2a = std::pmr::make_shared_from<AA>(mr);
    auto pr = std::pmr::set_default_resource(mr);
    // Allocated with the LoggingResource above
    auto c0 = std::allocate_shared<A, std::pmr::polymorphic_allocator<std::byte>>(mr);
    auto c1 = std::pmr::make_shared<A>();
    auto c2 = std::pmr::make_shared_from<A>(mr);
    std::pmr::set_default_resource(mr);
    auto d0 = std::allocate_shared<A, std::pmr::polymorphic_allocator<std::byte>>(std::pmr::get_default_resource());
    auto d1 = std::pmr::make_shared<A>();
    auto d2 = std::pmr::make_shared_from<A>(pr);
    auto d3 = std::pmr::make_shared_from<A>(mr);

#ifdef _MSC_VER
#ifdef _DEBUG
    _CrtCheckMemory();
#endif
#endif
    return 0;
}
