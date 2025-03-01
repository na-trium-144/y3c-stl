project('y3c-stl', 'c', 'cpp',
  version: '0.4.0',
  meson_version: '>=1.1',
  default_options: [
    'buildtype=release',
    'cpp_std=c++11',
    'libdwarf:default_library=static',
    'cpptrace:default_library=static',
  ],
)
fs = import('fs')
cxx = meson.get_compiler('cpp')
py = find_program('python3')
cmake = import('cmake')

y3c_soversion = '0.3'

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static y3c libraries is currently not supported',
)
summary('Library', get_option('default_library'))
conf_data = configuration_data()
conf_data.set10('Y3C_SHARED', get_option('default_library') == 'shared')
y3c_system_visibility = false
y3c_system_dllexport = false
y3c_system_add_debug = false
y3c_system_version_rc = false
if host_machine.system() == 'linux'
  summary('System', 'linux')
  y3c_system_visibility = true
elif host_machine.system() == 'darwin'
  summary('System', 'darwin')
  y3c_system_visibility = true
elif host_machine.system() == 'windows'
  y3c_system_dllexport = true
  y3c_system_version_rc = true
  if cxx.get_argument_syntax() == 'msvc'
    summary('System', 'windows msvc ' + host_machine.cpu_family())
    y3c_system_add_debug = true
    add_project_arguments('/utf-8', language: 'cpp')
  else
    summary('System', 'windows mingw')
  endif
elif host_machine.system() == 'cygwin'
  summary('System', 'cygwin')
  y3c_system_dllexport = true
  y3c_system_version_rc = true
else
  warning('unsupported system: ' + host_machine.system())
endif

y3c_lib_name_release = 'y3c'
y3c_lib_name_debug = 'y3c' + (y3c_system_add_debug ? 'd' : '')
summary('Build type', get_option('buildtype'))
if y3c_system_add_debug
  debug_crt = get_option('b_vscrt') == 'mtd' or get_option('b_vscrt') == 'mdd' or \
    (get_option('b_vscrt').endswith('from_buildtype') and get_option('buildtype') == 'debug')
  if debug_crt
    y3c_lib_name = y3c_lib_name_debug
  else
    y3c_lib_name = y3c_lib_name_release
  endif
  summary('Debug CRT', debug_crt, bool_yn: true)
else
  y3c_lib_name = y3c_lib_name_release
endif

conf_data.set10('Y3C_SYSTEM_VISIBILITY', y3c_system_visibility)
conf_data.set10('Y3C_SYSTEM_DLLEXPORT', y3c_system_dllexport)
conf_data.set10('Y3C_SYSTEM_ADD_DEBUG', y3c_system_add_debug)

configure_file(
  input: 'y3c-config.h.in',
  output: 'y3c-config.h',
  configuration: conf_data,
  install: true,
  install_dir: get_option('includedir') / 'y3c',
)
y3c_config_dep = declare_dependency(
  include_directories: '.',
  compile_args: ['-DY3C_MESON'],
)

if get_option('tests')
  doctest_dep = dependency('doctest', required: false)
  if doctest_dep.found()
    if cxx.has_header('doctest/doctest.h', dependencies: doctest_dep)
      doctest_dep = declare_dependency(
        dependencies: doctest_dep,
        compile_args: ['-DY3C_DOCTEST_NESTED_HEADER'],
      )
    endif
  else
    doctest_dep = dependency('doctest', required: true)
    # subprojectのdoctestのヘッダーはネストしていない
  endif
else
  doctest_dep = disabler()
  meson.override_dependency('doctest', dependency('', required: false))
endif

cpptrace_dep = dependency('cpptrace', version: '>=0.8')
libdwarf_dep = dependency('libdwarf')
rang_dep = dependency('rang')

y3c_inc = include_directories('include')
y3c_src = [
  'src/final_message.cc',
  'src/terminate_detail.cc',
  'src/what.cc',
]
if y3c_system_version_rc
  y3c_src += import('windows').compile_resources(
    configure_file(
      input: 'version.rc.in',
      output: 'version.rc',
      configuration: configuration_data({
        'fileversion': meson.project_version().replace('.', ','),
        'strversion': meson.project_version(),
      }),
    ),
  )
endif
y3c_lib = library(y3c_lib_name,
  y3c_src,
  include_directories: y3c_inc,
  cpp_args: ['-DY3C_BUILDING'],
  dependencies: [
    y3c_config_dep,
    cpptrace_dep,
    libdwarf_dep,
    rang_dep,
  ],
  version: meson.project_version(),
  soversion: y3c_soversion,
  # darwin_versions: [
  #   y3c_abi_major + '.' + y3c_abi_minor,
  #   meson.project_version(),
  # ],
  gnu_symbol_visibility: 'inlineshidden',
  install: true,
)
y3c_dep = declare_dependency(
  include_directories: y3c_inc,
  link_with: y3c_lib,
  dependencies: [y3c_config_dep],
)

import('pkgconfig').generate(
  name: 'y3c',
  description: 'y3c-stl',
  url: 'https://github.com/na-trium-144/y3c-stl',
  filebase: 'y3c',
  libraries: [y3c_lib],
  version: meson.project_version(),
)
meson.override_dependency('y3c', y3c_dep)
if get_option('default_library') == 'shared'
  # sharedライブラリの場合のみ、y3c.pc から Requires.private の行を削除
  meson.add_install_script(py,
    meson.project_source_root() / 'remove_private_requires.py',
  )
endif

if get_option('default_library') == 'shared'
  cmake.write_basic_package_version_file(
    name: 'y3c',
    version: meson.project_version(),
    compatibility: 'SameMajorVersion',
    install_dir: get_option('libdir') / 'cmake' / 'y3c', # 'y3c-' + y3c_abi_major,
  )
  conf_data = configuration_data({
    'prefix': get_option('prefix'),
    'libdir': get_option('libdir'),
    'includedir': get_option('includedir'),
    'y3c_lib_debug': y3c_lib_name_debug,
    'y3c_lib_release': y3c_lib_name_release,
    'y3c_soversion': y3c_soversion,
    'y3c_version_str': meson.project_version(),
  })
  cmake.configure_package_config_file(
    name: 'y3c',
    input: 'y3c-config.cmake.in',
    configuration: conf_data,
    install_dir: get_option('libdir') / 'cmake' / 'y3c', # 'y3c-' + y3c_abi_major,
  )
endif

install_subdir('include/y3c', install_dir: get_option('includedir'))
install_data('README.md', install_dir: get_option('datadir') / 'doc' / 'y3c')
if fs.exists('LICENSE')
  install_data('LICENSE', install_dir: get_option('datadir') / 'doc' / 'y3c')
endif

subdir('tests')
subdir('examples')
