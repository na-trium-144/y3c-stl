examples = [
  'array-at',
  'shared_ptr-ref',
]
foreach case : examples
  executable('y3c-example-' + case,
    case + '.cc',
    dependencies: [
      y3c_dep,
    ],
  )
endforeach
