backend_sources = [
  'eval.c',
  'eval.h',
  'lexer.c',
  'lexer.h',
  'parser.c',
  'parser.h',
  'parsetree.c',
  'parsetree.h',
  'constants.h',
]

plugin_sources = backend_sources + [
  'calculator.c',
  xfce_revision_h,
]

plugin_deps = [
  glib,
  gtk,
  libxfce4util,
  libxfce4ui,
  libxfce4panel,
  libm,
]

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

plugin_lib = shared_module(
  'calculator',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-calculator-plugin'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: plugin_deps,
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

i18n.merge_file(
  input: 'calculator.desktop.in',
  output: 'calculator.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)

if get_option('tests')
  calctest = executable(
    'calctest',
    sources: backend_sources + ['calctest.c'],
    dependencies: plugin_deps,
  )
endif
