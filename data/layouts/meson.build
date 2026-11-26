python = import('python').find_installation('python3')

generated_files = custom_target(
  'generate-layouts',
  output: [
    'iso-a.svg',
    'iso-e.svg',
    'iso-b.svg',
    'iso-j.svg',
    'iso-k.svg',
    'iso-l.svg',
  ],
  command: [
    python,
    files('generate_layouts.py'),
    meson.current_build_dir(),
  ],
  install: true,
  install_dir: get_option('datadir') / meson.project_name() / 'layouts',
)
