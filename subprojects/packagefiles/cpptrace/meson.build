project('cpptrace', 'c', 'cpp', version: '0.7.3')

# めんどくさいのでオプションなしでautoconfig.cmakeだけを移植している

cmake_dir = include_directories('cmake')
cxx = meson.get_compiler('cpp')
has_cxxabi = cxx.compiles('#include "has_cxxabi.cpp"', name: 'cxxabi support check', include_directories: cmake_dir)
has_unwind = cxx.compiles('#include "has_unwind.cpp"', name: 'unwind support check', include_directories: cmake_dir)
has_execinfo = cxx.compiles('#include "has_execinfo.cpp"', name: 'execinfo support check', include_directories: cmake_dir)
dbghelp_dep = cxx.find_library('dbghelp', required: false, disabler: true)
has_stackwalk = cxx.compiles('#include "has_stackwalk.cpp"', name: 'stackwalk support check', include_directories: cmake_dir, dependencies: dbghelp_dep)
has_cxx_exception_type = cxx.compiles('#include "has_cxx_exception_type.cpp"', name: 'cxx exception type support check', include_directories: cmake_dir)
dl_dep = cxx.find_library('dl', required: false, disabler: true)
has_dl_find_object = cxx.compiles('#include "has_dl_find_object.cpp"', name: 'dl find object support check', include_directories: cmake_dir, dependencies: dl_dep)
has_dladdr1 = has_dl_find_object and cxx.compiles('#include "has_dladdr1.cpp"', name: 'dladdr1 support check', include_directories: cmake_dir, dependencies: dl_dep)
has_mach_vm = cxx.compiles('#include "has_mach_vm.cpp"', name: 'mach vm support check', include_directories: cmake_dir)

cpptrace_unwind_with_execinfo = false
cpptrace_unwind_with_unwind = false
cpptrace_unwind_with_dbghelp = false
cpptrace_unwind_with_winapi = false
if host_machine.system() == 'darwin' and cxx.get_id() == 'clang'
  if has_execinfo
    cpptrace_unwind_with_execinfo = true
    summary('unwind', 'execinfo')
  else
    error('No unwinding back-end seems to be supported, stack tracing will not work.')
  endif
elif host_machine.system() != 'windows'
  if has_unwind
    cpptrace_unwind_with_unwind = true
    summary('unwind', 'unwind')
  elif has_execinfo
    cpptrace_unwind_with_execinfo = true
    summary('unwind', 'execinfo')
  else
    error('No unwinding back-end seems to be supported, stack tracing will not work.')
  endif
else # host_machine.system() == 'windows'
  if has_stackwalk
    cpptrace_unwind_with_dbghelp = true
    summary('unwind', 'dbghelp')
  else
    cpptrace_unwind_with_winapi = true
    summary('unwind', 'winapi')
  endif
endif

cpptrace_get_symbols_with_libdwarf = false
cpptrace_get_symbols_with_dbghelp = false
if host_machine.system() != 'windows'
  cpptrace_get_symbols_with_libdwarf = true
  summary('symbols', 'libdwarf')
elif cxx.get_argument_syntax() == 'gcc'
  # Use both dbghelp and libdwarf under mingw: Some files may use pdb symbols, e.g. system dlls like KERNEL32.dll and
  # ntdll.dll at the very least, but also other libraries linked with may have pdb symbols.
  cpptrace_get_symbols_with_libdwarf = true
  cpptrace_get_symbols_with_dbghelp = true
  summary('symbols', 'libdwarf + dbghelp')
else
  cpptrace_get_symbols_with_dbghelp = true
  summary('symbols', 'dbghelp')
endif

cpptrace_demangle_with_cxxabi = false
cpptrace_demangle_with_winapi = false
cpptrace_demangle_with_nothing = false
if has_cxxabi
  cpptrace_demangle_with_cxxabi = true
  summary('demangle', 'cxxabi')
elif cxx.get_argument_syntax() == 'msvc'
  cpptrace_demangle_with_winapi = true
  summary('demangle', 'winapi')
else
  cpptrace_demangle_with_nothing = true
  summary('demangle', 'nothing')
endif

cpptrace_src = [
  'src/binary/elf.cpp',
  'src/binary/mach-o.cpp',
  'src/binary/module_base.cpp',
  'src/binary/object.cpp',
  'src/binary/pe.cpp',
  'src/binary/safe_dl.cpp',
  'src/cpptrace.cpp',
  'src/ctrace.cpp',
  'src/exceptions.cpp',
  'src/from_current.cpp',
  'src/options.cpp',
  'src/utils.cpp',
  'src/demangle/demangle_with_cxxabi.cpp',
  'src/demangle/demangle_with_nothing.cpp',
  'src/demangle/demangle_with_winapi.cpp',
  'src/snippets/snippet.cpp',
  'src/symbols/dwarf/debug_map_resolver.cpp',
  'src/symbols/dwarf/dwarf_resolver.cpp',
  'src/symbols/symbols_core.cpp',
  'src/symbols/symbols_with_addr2line.cpp',
  'src/symbols/symbols_with_dbghelp.cpp',
  'src/symbols/symbols_with_dl.cpp',
  'src/symbols/symbols_with_libbacktrace.cpp',
  'src/symbols/symbols_with_libdwarf.cpp',
  'src/symbols/symbols_with_nothing.cpp',
  'src/unwind/unwind_with_dbghelp.cpp',
  'src/unwind/unwind_with_execinfo.cpp',
  'src/unwind/unwind_with_libunwind.cpp',
  'src/unwind/unwind_with_nothing.cpp',
  'src/unwind/unwind_with_unwind.cpp',
  'src/unwind/unwind_with_winapi.cpp',
  'src/utils/microfmt.cpp',
  'src/utils/utils.cpp',
  'src/platform/dbghelp_syminit_manager.cpp',
]
cpptrace_deps = []
cpptrace_args = ['-DNOMINMAX']
if get_option('default_library') == 'static'
  cpptrace_args += ['-DCPPTRACE_STATIC_DEFINE']
endif
if has_cxx_exception_type
  cpptrace_args += ['-DCPPTRACE_HAS_CXX_EXCEPTION_TYPE']
endif
if has_dl_find_object
  cpptrace_args += ['-DCPPTRACE_HAS_DL_FIND_OBJECT']
endif
if has_dladdr1
  cpptrace_args += ['-DCPPTRACE_HAS_DLADDR1']
endif
if has_mach_vm
  cpptrace_args += ['-DCPPTRACE_HAS_MACH_VM']
endif

if cpptrace_get_symbols_with_libdwarf
  cpptrace_args += ['-DCPPTRACE_GET_SYMBOLS_WITH_LIBDWARF']
  libdwarf_dep = dependency('libdwarf', required: false)
  if not libdwarf_dep.found()
    foreach target : ['libdwarf::dwarf', 'libdwarf::dwarf-static', 'libdwarf::dwarf-shared', 'libdwarf::dwarf']
      libdwarf_dep = dependency('libdwarf', required: false, method: 'cmake', modules: [target])
      if libdwarf_dep.found()
        break
      endif
    endforeach
  endif
  if libdwarf_dep.found()
    if cxx.has_header('libdwarf/libdwarf.h', dependencies: libdwarf_dep)
      cpptrace_args += ['-DCPPTRACE_USE_NESTED_LIBDWARF_HEADER_PATH']
    elif cxx.has_header('libdwarf.h', dependencies: libdwarf_dep)
    else
      error('couldn\'t find libdwarf.h')
    endif
  else
    libdwarf_dep = dependency('libdwarf', required: true)
  endif
  cpptrace_deps += libdwarf_dep
  if dl_dep.found()
    cpptrace_deps += dl_dep
  endif
endif
if cpptrace_get_symbols_with_dbghelp
  cpptrace_args += ['-DCPPTRACE_GET_SYMBOLS_WITH_DBGHELP']
  cpptrace_deps += dbghelp_dep
endif

if cpptrace_unwind_with_unwind
  cpptrace_args += ['-DCPPTRACE_UNWIND_WITH_UNWIND']
endif
if cpptrace_unwind_with_execinfo
  cpptrace_args += ['-DCPPTRACE_UNWIND_WITH_EXECINFO']
endif
if cpptrace_unwind_with_winapi
  cpptrace_args += ['-DCPPTRACE_UNWIND_WITH_WINAPI']
endif
if cpptrace_unwind_with_dbghelp
  cpptrace_args += ['-DCPPTRACE_UNWIND_WITH_DBGHELP']
  cpptrace_deps += dbghelp_dep
endif

if cpptrace_demangle_with_cxxabi
  cpptrace_args += ['-DCPPTRACE_DEMANGLE_WITH_CXXABI']
endif
if cpptrace_demangle_with_winapi
  cpptrace_args += ['-DCPPTRACE_DEMANGLE_WITH_WINAPI']
endif
if cpptrace_demangle_with_nothing
  cpptrace_args += ['-DCPPTRACE_DEMANGLE_WITH_NOTHING']
endif

cpptrace_lib = library('cpptrace',
  cpptrace_src,
  include_directories: ['include', 'src'],
  cpp_args: cpptrace_args,
  dependencies: cpptrace_deps,
  gnu_symbol_visibility: 'inlineshidden',
  version: meson.project_version(),
  soversion: meson.project_version().split('.')[0],
)
# todo: ranlib?
# SET(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
# SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
cpptrace_dep = declare_dependency(
  link_with: cpptrace_lib,
  include_directories: ['include'],
  compile_args: cpptrace_args,
  dependencies: cpptrace_deps,
)
