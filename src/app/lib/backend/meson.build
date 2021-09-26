if get_option('library_backend') == 'xcb'
    subdir('xcb')
    subdir('legacy')
    # todo: should now only be used in backend.h
    # make it BACKEND=XCB or something more generic
    project_build_args += '-DUSE_XCB=1'
else
    subdir('legacy')
endif
