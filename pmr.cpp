#include <memory_resource>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <cstddef> // std::byte
#include <iostream>
#include <string>

class A
{
    float x;
    float y;
    float z;
};


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

int main(int argc, const char *argv[])
{
    // Allocated with global new/delete
    auto a = std::make_shared<A>();

    auto mr = new LoggingResource(std::pmr::get_default_resource());
    // Allocated with the LoggingResource above
    auto b = std::allocate_shared<A, std::pmr::polymorphic_allocator<std::byte>>(mr);
    return 0;
}
