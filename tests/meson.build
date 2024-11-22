test_cases = [
  'shared_ptr',
]
foreach case : test_cases
  test(case,
    executable('y3c-test-' + case,
        case + '.cc',
        dependencies: [
          y3c_dep,
          gtest_dep,
        ],
    ),
  )
endforeach
