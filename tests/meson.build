run_phoc_tests = false

# Deps for all tests
test_stub_deps = files(
  'stubs/phosh.c',
  'stubs/app-tracker.c',
  'stubs/lockscreen-manager.c',
  'stubs/toplevel.c',
  'stubs/toplevel-manager.c',
  'stubs/thumbnail.c',
)

if get_option('tests')

subdir('system')
subdir('user')

screenshot_langs = ['ar_AE.UTF-8', 'de_DE.UTF-8', 'ja_JP.UTF-8', 'uk_UA.UTF-8']

phoc = find_program('phoc', required: get_option('phoc_tests'), disabler: true)
run_phoc_tests =  phoc.found() and not get_option('phoc_tests').disabled()

test_env_common = environment()
test_env_common.set('G_TEST_SRCDIR', meson.current_source_dir())
test_env_common.set('G_TEST_BUILDDIR', meson.current_build_dir())
test_env_common.set('G_DEBUG', 'gc-friendly,fatal-warnings')
test_env_common.set('GSETTINGS_BACKEND', 'memory')
test_env_common.set('GSETTINGS_SCHEMA_DIR', '@0@/data'.format(meson.build_root()))
test_env_common.set('PYTHONDONTWRITEBYTECODE', 'yes')
test_env_common.set('MALLOC_CHECK_', '2')
test_env_common.set('NO_AT_BRIDGE', '1')

# For -Db_sanitize=address
test_env_common.set('LSAN_OPTIONS',
                    'suppressions=@0@/data/leak-suppress.txt'.format(meson.source_root()))
# as our libraries lack frame pointers since otherwise back traces are incomplete
test_env_common.set('ASAN_OPTIONS', 'fast_unwind_on_malloc=0')

# xdg setup for the app-grid:
test_env_unit = test_env_common
test_env_unit.set('XDG_CONFIG_HOME', '@0@/user/config/'.format(meson.current_source_dir()))
test_env_unit.set('XDG_CONFIG_DIRS', '@0@/system/config/'.format(meson.current_source_dir()))
test_env_unit.set('XDG_DATA_HOME', '@0@/user/share/'.format(meson.current_source_dir()))
# Ideally we would just set it so that we have a known set of .desktop etc
# but then we can't find the system gschemas
test_env_unit.prepend('XDG_DATA_DIRS', '@0@/system/share/'.format(meson.current_source_dir()))

test_cflags = [
  '-DTEST_BUILD_DIR="@0@"'.format(meson.build_root()),
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
  '-DTEST_OUTPUT_DIR="@0@"'.format(meson.current_build_dir()),
  '-DTEST_PHOC_INI="@0@/data/phoc.ini"'.format(meson.source_root()),
  # Some tests want an installed tree (e.g. for .mo, this is where it should
  # be located:
  '-DTEST_INSTALLED="@0@/install"'.format(meson.build_root()),
  '-DTEST_TOOLS="@0@/tools"'.format(meson.build_root()),
]

test_link_args = [
  '-fPIC',
]

testlib_sources = [
  'testlib.c',
  'testlib-head-stub.c',
  'testlib-full-shell.c',
  'testlib-calls-mock.c',
  'testlib-mpris-mock.c',
]

testlib = static_library('phoshtest', testlib_sources,
  dependencies: [phosh_deps, phosh_tool_dep],
  c_args: test_cflags)
testlib_dep = declare_dependency(
  include_directories: include_directories('.'),
  dependencies: phosh_tool_dep,
  link_with: testlib)

tests = [
  'activity',
  'app-grid-button',
  'app-list-model',
  'connectivity-info',
  'css',
  'fading-label',
  'favourite-model',
  'head',
  'keypad',
  'media-player',
  'mount-notification',
  'notification',
  'notification-content',
  'notification-frame',
  'notification-list',
  'notification-source',
  'overview',
  'plugin-loader',
  'quick-setting',
  'status-icon',
  'util',
]

tests_phoc = [
  'app-auth-prompt',
  'background',
  'drag-surface',
  'end-session-dialog',
  'layer-surface',
  'lockshield',
  'notification-banner',
  'osd-window',
  'system-modal',
  'system-modal-dialog',
  'timestamp-label',
]

tests_manager = [
  [ 'calls-manager', true, ],
  [ 'gtk-mount-manager', false, ],
  [ 'idle-manager', false, ],
  [ 'monitor-manager', true, ],
  [ 'notify-manager', true, ],
  [ 'screenshot-manager', true, ],
]

# Unit tests
foreach test : tests
  t = executable('test-@0@'.format(test),
                 test_stub_deps + ['test-@0@.c'.format(test)],
                 c_args: test_cflags,
                 pie: true,
                 link_args: test_link_args,
                 dependencies: testlib_dep)
  test(test, t,
       env: test_env_unit,
       suite: ['unit'])
endforeach

if run_phoc_tests
  test_env_phoc = test_env_common
  # Make sure this is valid when running the compositor
  test_env_phoc.set('XDG_RUNTIME_DIR', meson.current_build_dir())

  # Unit tests
  foreach test : tests_phoc
    t = executable('test-@0@'.format(test),
                   test_stub_deps + ['test-@0@.c'.format(test)],
                   c_args: test_cflags,
                   pie: true,
                   link_args: test_link_args,
                   dependencies: testlib_dep)
    test(test, t,
         suite: ['unit', 'phoc'],
         env: test_env_phoc,
         is_parallel: false)
  endforeach

  # Integration tests using the full libphosh
  t = executable('test-shell',
                 'test-shell.c',
                 c_args: test_cflags,
                 pie: true,
                 link_args: test_link_args,
                 dependencies: [phosh_tool_dep, phosh_dep, testlib_dep])
  test('shell', t,
       env: test_env_phoc,
       suite: ['integration'],
       is_parallel : false)
  foreach item : tests_manager
    test = item.get(0)
    parallel = item.get(1)

    t = executable('test-@0@'.format(test),
                   ['test-@0@.c'.format(test), generated_dbus_sources],
                   c_args: test_cflags,
                   pie: true,
                   link_args: test_link_args,
                   dependencies: [phosh_tool_dep, phosh_dep, testlib_dep])
    test(test, t,
         env: test_env_phoc,
         suite: ['integration'],
         is_parallel : parallel)
  endforeach

  # Tests for manual validation
  t = executable('test-take-screenshots',
                   ['test-take-screenshots.c', generated_dbus_sources],
                   c_args: test_cflags,
                   pie: true,
                   link_args: test_link_args,
                   dependencies: [phosh_tool_dep, phosh_dep, testlib_dep])
  foreach lang : screenshot_langs
    lang_env = test_env_phoc
    lang_env.set('LANGUAGE', lang)
    lang_env.set('LC_ALL', lang)
    lang_env.set('XDG_DATA_HOME', '@0@/user/share/'.format(meson.current_source_dir()))
    test('screenshots-@0@'.format(lang), t,
       env: lang_env,
       suite: ['screenshots'])
  endforeach

endif

endif # tests
