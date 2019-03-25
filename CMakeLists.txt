cmake_minimum_required (VERSION 3.2)
project (netherearth)

set (netherearth_VERSION_MAJOR 0)
set (netherearth_VERSION_MINOR 51)
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set (CMAKE_CXX_STANDARD 17)
# set (OpenGL_GL_PREFERENCE GLVND)

# find_package (GLUT)

find_library (GLLIB NAMES OpenGL)
find_library (GLULIB NAMES GLU)
find_library (GLUTLIB NAMES glut)
find_library (SDLLIB NAMES SDL)
find_library (SDLMIXERLIB NAMES SDL_mixer)
link_libraries(stdc++fs)

# include_directories ("${PROJECT_BINARY_DIR}")
add_executable (${PROJECT_NAME} ${SOURCES})
target_sources (${PROJECT_NAME}
  PUBLIC 3dobject
  PUBLIC ai
  PUBLIC bitmap
  PUBLIC building
  PUBLIC bullet
  PUBLIC bulletcannon
  PUBLIC bulletmissile
  PUBLIC bulletphaser
  PUBLIC camera
  PUBLIC cmc
  PUBLIC constructionscreen
  PUBLIC explosion
  PUBLIC glprintf
  PUBLIC light
  PUBLIC main
  PUBLIC mainmenu
  PUBLIC map
  PUBLIC menu
  PUBLIC myglutaux
  PUBLIC nether
  PUBLIC optionsscreen
  PUBLIC particle
  PUBLIC piece3dobject
  PUBLIC quaternion
  PUBLIC radar
  PUBLIC resources
  PUBLIC robot
  PUBLIC robots
  PUBLIC shadow3dobject
  PUBLIC ship
  PUBLIC soundmanager
  PUBLIC stats
  PUBLIC statusbutton
  PUBLIC vector
)
target_link_libraries (
  ${PROJECT_NAME}
  ${GLLIB}
  ${GLULIB}
  ${GLUTLIB}
  ${SDLLIB}
  ${SDLMIXERLIB})
