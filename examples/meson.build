examples = [
  'array-at',
  'array-operator',
  'array-iter',
  'shared_ptr-null',
  'ptr-local',
  'ptr-inc',
  'ptr-null',
]
foreach case : examples
  executable('y3c-example-' + case,
    case + '.cc',
    dependencies: [
      y3c_dep,
    ],
  )
endforeach
