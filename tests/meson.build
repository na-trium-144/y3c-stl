test_cases = [
  'array',
  'iterator',
  'shared_ptr',
  'terminate',
  'typename',
  'wrap',
]
test_args = cxx.get_supported_arguments(
  '-Wno-unused-value',
  '-Wno-pessimizing-move',
)
foreach case : test_cases
  test(case,
    executable('y3c-test-' + case,
      case + '.cc',
      dependencies: [
        y3c_dep,
        doctest_dep,
      ],
      cpp_args: [
        '-DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN',
        test_args,
      ],
    ),
  )
endforeach
