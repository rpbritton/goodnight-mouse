project('goodnight_mouse', 'c')

project_source_files = []
project_dependencies = []
project_build_args = ['-Wall']

subdir('src')

if get_option('build_level') == 'debug'
    # todo: add debug envar
    project_build_args += '-g'
endif

project_target = executable(
    meson.project_name(),
    project_source_files,
    dependencies: project_dependencies,
    c_args : project_build_args,
)
