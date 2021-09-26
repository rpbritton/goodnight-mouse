if get_option('library_backend') == 'xcb'
    project_build_args += '-DLIBRARY_BACKEND_XCB=1'
    subdir('xcb')
    subdir('legacy')
else
    project_build_args += '-DLIBRARY_BACKEND_LEGACY=1'
    subdir('legacy')
endif
