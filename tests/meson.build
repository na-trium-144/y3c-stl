test_cases = [
  'array',
  'shared_ptr',
  'typename',
  'wrap',
]
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
      ],
    ),
  )
endforeach
