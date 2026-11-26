python = import('python').find_installation('python3')

generated_files = custom_target(
  'generate-layouts',
  output: [
    'iso-a-full.svg',
    'iso-e-full.svg',
    'iso-b-full.svg',
    'iso-j-full.svg',
    'iso-k-full.svg',
    'iso-l-full.svg',
  ],
  command: [
    python,
    files('generate_layouts.py'),
    meson.current_build_dir(),
  ],
  install: true,
  install_dir: get_option('datadir') / meson.project_name() / 'layouts',
)
