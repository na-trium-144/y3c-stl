examples = [
  'array-at',
  'array-iter',
  'array-operator',
  'array-ptr-local',
  'array-ptr',
  'catch',
  'ptr-inc',
  'ptr-local',
  'ptr-null',
  'shared_ptr-null',
  'shared_ptr-ref',
  'shared_ptr-unwrap-ref',
  'terminate-direct',
  'terminate-throw-any',
  'terminate-throw-exception',
  'terminate-throw-string',
]
foreach case : examples
  executable('y3c-example-' + case,
    case + '.cc',
    dependencies: [
      y3c_dep,
    ],
  )
endforeach
