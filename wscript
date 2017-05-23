#!/usr/bin/env python
from subprocess import check_output, CalledProcessError

def depends(ctx):
    ctx('logger')
    ctx('halco')


def options(opt):
    opt.load('compiler_cxx')
    opt.load('gtest')


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('gtest')

    cfg.env.LINKFLAGS_HALDLS_LIBRARIES = [
        '-fvisibility=hidden',
        '-fvisibility-inlines-hidden',
    ]


def build(bld):
    bld(
        target = 'haldls_inc',
        export_includes = 'include',
    )

    bld.shlib(
        target = 'haldls_container_v2',
        source = bld.path.ant_glob('src/container/v2/*.cpp'),
        install_path = '${PREFIX}/lib',
        use = ['haldls_inc', 'halco_hicann_dls_v2_inc', 'halco_hicann_dls_v2'],
        uselib = 'HALDLS_LIBRARIES',
    )

    bld(
        target = 'haldls_container_v2_tests',
        features = 'gtest cxx cxxprogram',
        source = bld.path.ant_glob('tests/container/v2/test-*.cpp'),
        use = ['haldls_container_v2', 'GTEST'],
        install_path = '${PREFIX}/bin',
    )
