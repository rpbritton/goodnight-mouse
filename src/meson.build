project_source_files += files(
    'main.c',
    'config.c',
)

project_dependencies += [
    dependency('glib-2.0'),
    dependency('atspi-2'),
    dependency('gobject-2.0'),
    dependency('gtk+-3.0'),
]

project_build_args += [
    '-DG_LOG_DOMAIN="' + meson.project_name() + '"',
]

subdir('app')
