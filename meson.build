project('y3c-stl', 'cpp',
  version: '0.1.0',
  meson_version: '>=1.1',
  default_options: [
    'cpp_std=c++11',
    'libdwarf:default_library=static',
    'cpptrace:default_library=static',
  ],
)

cxx = meson.get_compiler('cpp')

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static webcface libraries is currently not supported',
)
summary('Library', get_option('default_library'))
conf_data = configuration_data({
  'Y3C_ABI_MAJOR': meson.project_version().split('.')[0],
})
conf_data.set10('Y3C_SHARED', get_option('default_library') == 'shared')
y3c_system_visibility = false
y3c_system_dllexport = false
y3c_system_exclude_libs = false
y3c_system_hidden_l = false
y3c_system_add_debug = false
y3c_system_version_rc = false
relative_rpath = ''
if host_machine.system() == 'linux'
  summary('System', 'linux')
  y3c_system_visibility = true
  y3c_system_exclude_libs = true
  relative_rpath = '$ORIGIN' / '..' / get_option('libdir')
elif host_machine.system() == 'darwin'
  summary('System', 'darwin')
  y3c_system_visibility = true
  y3c_system_hidden_l = true
  relative_rpath = '@loader_path' / '..' / get_option('libdir')
elif host_machine.system() == 'windows'
  y3c_system_dllexport = true
  y3c_system_version_rc = true
  if cxx.get_argument_syntax() == 'msvc'
    summary('System', 'windows msvc ' + host_machine.cpu_family())
    y3c_system_add_debug = true
    magick_vs = true
  else
    summary('System', 'windows mingw')
    add_global_link_arguments('-Wl,--exclude-all-symbols', language: 'cpp')
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

cpptrace_dep = dependency('cpptrace')
rang_dep = dependency('rang')
doctest_dep = dependency('doctest')

y3c_inc = include_directories('include')
y3c_src = [
  'src/exception.cc',
  'src/msg.cc',
  'src/trace.cc',
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
y3c_lib = library('y3c',
  y3c_src,
  include_directories: y3c_inc,
  cpp_args: ['-DY3C_BUILDING'],
  dependencies: [
    y3c_config_dep,
    cpptrace_dep,
    rang_dep,
  ],
  version: meson.project_version(),
  # soversion: 
  # darwin_versions: 
  gnu_symbol_visibility: 'inlineshidden',
)
y3c_dep = declare_dependency(
  include_directories: y3c_inc,
  link_with: y3c_lib,
  dependencies: [y3c_config_dep],
)

subdir('tests')
subdir('examples')
