project(
  'GraphRanker', 'c',
  version: '1.0.0',
  meson_version: '>=0.52.0',
  default_options: [
	'buildtype=debug',
	'c_std=gnu11'
  ]
)

cc_args = [
  '-DEVAL',
  '-Wall',
  '-Werror',
  '-std=gnu11',
  '-pipe',
  '-static',
  '-s',
  '-lm'
]

exe = executable(
  meson.project_name(),
  'graphranker.c',
  c_args: cc_args
)

test_env = environment({'SOURCE_ROOT': meson.source_root()})
test_runner = find_program('./tests/runner.py')
test(
  'open',
  test_runner,
  args: [exe],
  env: test_env,
  suite: 'open'
)
