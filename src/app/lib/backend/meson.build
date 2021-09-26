if get_option('library_backend') == 'xcb'
    project_build_args += '-DLIBRARY_BACKEND=xcb'
    subdir('xcb')
    subdir('legacy')
else
    project_build_args += '-DLIBRARY_BACKEND=legacy'
    subdir('legacy')
endif
