project_root = meson.project_source_root()

wl = import('wayland')
protocols = [
  wl.scan_xml(
    files(
      project_root / 'protocols/xdg-shell.xml',
      project_root / 'protocols/wlr-layer-shell-unstable-v1.xml'
    )
  ),

  wl.scan_xml(
    files(
      project_root / 'protocols/tablet-v2.xml',
      project_root / 'protocols/cursor-shape-v1.xml'
    )
  ),
]


deps = [
  dependency('wayland-client'),
  dependency('libxml-2.0'),
  dependency('xkbcommon'),
  dependency('cairo'),
  dependency('pangocairo'),
]


executable(
  meson.project_name(),
  'main.c',
  'key.c',
  'uinput.c',
  'svg.c',
  'xkb.c',
  'draw.c',
  'board.c',
  'widget.c',
  'button.c',
  'togglebutton.c',
  'label.c',
  'dropdown.c',
  'utils.c',
  'config.c',
  protocols,
  dependencies: deps,
  install: true
)
