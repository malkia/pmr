# pmr
std::pmr explorations

# MSVC
bazel test --config=msvc -c dbg pmr

# gcc-snapshot (apt get install gcc-snapshot)
bazel test --config=gcc-snapshot -c dbg pmr
