# https://github.com/kstenerud/meson-examples/blob/master/executable/meson.build

project('netherearth',
        'cpp',
        version : '0.51',
        default_options : ['cpp_std=c++17', 'warning_level=3']
       )

project_source_files = [
  '3dobject.cpp',
  'ai.cpp',
  'bitmap.cpp',
  'building.cpp',
  'buildingblock.cpp',
  'buildingfactory.cpp',
  'buildingsimple.cpp',
  'buildingwarbase.cpp',
  'bullet.cpp',
  'bulletcannon.cpp',
  'bulletmissile.cpp',
  'bulletphaser.cpp',
  'camera.cpp',
  'cmc.cpp',
  'constructionscreen.cpp',
  'explosion.cpp',
  'glprintf.cpp',
  'light.cpp',
  'main.cpp',
  'mainmenu.cpp',
  'map.cpp',
  'menu.cpp',
  'myglutaux.cpp',
  'nether.cpp',
  'optionsscreen.cpp',
  'particle.cpp',
  'piece3dobject.cpp',
  'quaternion.cpp',
  'radar.cpp',
  'resources.cpp',
  'robot.cpp',
  'robots.cpp',
  'shadow3dobject.cpp',
  'ship.cpp',
  'soundmanager.cpp',
  'stats.cpp',
  'statusbutton.cpp',
  'vector.cpp'
]

sexp_proj = subproject('sexp_cpp')
sexp_dep = sexp_proj.get_variable('sexp_cpp_dep')

project_dependencies = [
  dependency('OpenGL'),
  dependency('glu'),
  dependency('glut'),
  dependency('sdl'),
  dependency('SDL_mixer'),
  dependency('yaml-cpp'),
  sexp_dep
]

executable(meson.project_name(),
           project_source_files,
           dependencies : project_dependencies,
          )
