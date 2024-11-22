project('y3c-stl', 'cpp',
  meson_version: '>=1.1',
  default_options: [
    'cpp_std=c++11',
  ],
)

# todo: find and build dependency zstd and libdwarf from meson side?
cpptrace_dep = dependency('cpptrace', required: false)
if not cpptrace_dep.found()
  cmake = import('cmake')
  opt_var = cmake.subproject_options()
  opt_var.add_cmake_defines({
    'CPPTRACE_BUILD_SHARED': false,
  })
  cpptrace_proj = cmake.subproject('cpptrace', options: opt_var)
  # https://github.com/mesonbuild/meson/issues/13101
  cpptrace_dep = []
  foreach target : cpptrace_proj.target_list()
    if target != 'uninstall' and target != 'clean-all'
      cpptrace_dep += cpptrace_proj.dependency(target)
    endif
  endforeach
endif

gtest_dep = dependency('gtest', main: true, fallback: ['gtest', 'gtest_main'])

y3c_inc = include_directories('include')
y3c_lib = library('y3c',
  [
    'src/terminate.cc',
  ],
  include_directories: y3c_inc,
  dependencies: [
    cpptrace_dep,
  ],
)
y3c_dep = declare_dependency(
  include_directories: y3c_inc,
  link_with: y3c_lib,
)

subdir('tests')
