project('y3c-stl', 'cpp',
  meson_version: '>=1.1',
  default_options: [
    'cpp_std=c++11',
    'libdwarf:default_library=static',
    'cpptrace:default_library=static',
  ],
)

cpptrace_dep = dependency('cpptrace')
rang_dep = dependency('rang')
doctest_dep = dependency('doctest')

y3c_inc = include_directories('include')
y3c_src = [
  'src/exception.cc',
  'src/msg.cc',
  'src/trace.cc',
]
y3c_lib = library('y3c',
  y3c_src,
  include_directories: y3c_inc,
  dependencies: [
    cpptrace_dep,
    rang_dep,
  ],
)
y3c_dep = declare_dependency(
  include_directories: y3c_inc,
  link_with: y3c_lib,
)

subdir('tests')
subdir('examples')

