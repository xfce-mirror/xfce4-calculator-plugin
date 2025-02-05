test_env = environment()
test_env.prepend('PATH', fs.parent(calctest.full_path()))

tests = [
  'test-abs',
  'test-acos',
  'test-asin',
  'test-atan',
  'test-cos',
  'test-cube-root',
  'test-div-zero',
  'test-div-zero-negative',
  'test-div-zero-zero',
  'test-e-notation',
  'test-exp',
  'test-hex',
  'test-log',
  'test-log10',
  'test-log10-negative',
  'test-log10-zero',
  'test-log2',
  'test-minus',
  'test-modulo',
  'test-pi',
  'test-pow',
  'test-simple-expr',
  'test-sin',
  'test-sqrt',
  'test-sqrt-minus-one',
  'test-tan',
]

foreach test: tests
  test(test, find_program('@0@.awk'.format(test)), env: test_env)
endforeach
