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
  'vector-at',
  'vector-insert',
  'vector-iter-erase',
  'vector-iter-reallocate',
  'vector-iter',
  'vector-operator',
  'vector-ptr-local',
  'vector-ptr-reallocate',
  'vector-ptr',
  'wrap_array-ptr-local',
  'wrap_array-ptr',
  'wrap_array-range',
]
examples_dox = 'examples.dox.in'
dox_i = 0
foreach case : examples
  executable('y3c-example-' + case,
    case + '.cc',
    dependencies: [
      y3c_dep,
    ],
  )
  examples_dox = configure_file(
    input: examples_dox,
    output: 'examples.dox.' + dox_i.to_string(),
    configuration: configuration_data({
      'line': '//! \example ' + case + '.cc\n@line@',
    }),
  )
  dox_i += 1
endforeach
