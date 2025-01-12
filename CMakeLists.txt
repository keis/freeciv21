# Win32 et al requires cmake version 3.16 because we use a newer command
#   "file(GET_RUNTIME_DEPENDENCIES ..." that is only available at v3.16+
# On other platforms (*nix, MacOS) we need to support older 3.12+ for the
#   server and other components
if(WIN32 OR MSYS OR MINGW)
  cmake_minimum_required(VERSION 3.16...3.20 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 3.12...3.17 FATAL_ERROR)
endif()

# Set vcpkg if exists. Used by MacOS and Visual Studio
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Microsoft VCPKG detected, setting toolset specific settings.")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# Detect MS Visual Studio
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  message(STATUS "Microsoft Visual Studio detected, setting toolset specific settings.")
  set(CMAKE_GENERATOR_TOOLSET "ClangCl")
  message(STATUS "VS Cmake Generator Toolset: ${CMAKE_GENERATOR_TOOLSET}")
  set(CMAKE_GENERATOR_PLATFORM "x64")
  message(STATUS "VS Cmake Generator Platform: ${CMAKE_GENERATOR_PLATFORM}")
endif()

# Set up cmake search path
if (EMSCRIPTEN)
  message(STATUS "Using Emscripten-specific settings")
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/emscripten)
endif()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Always generate the compilation database by default
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Set the build defaults
include(cmake/FreecivBuildOptions.cmake)

# Add support for Auto Revision
include(AutoRevision)

# Set some variables from AutoRevision for use here and later
list(GET FC21_REV_TAG_LIST 0 FC21_MAJOR_VERSION)
list(GET FC21_REV_TAG_LIST 1 FC21_MINOR_VERSION)
list(GET FC21_REV_TAG_LIST 2 FC21_LABEL_VERSION)
list(GET FC21_REV_TAG_LIST 3 FC21_EMER_VERSION)
set(FC21_PATCH_VERSION ${FC21_REV_HEAD_HASH_D})

# Set some project variables to reuse later
set(FREECIV21_VERSION_MAJOR ${FC21_MAJOR_VERSION})
set(FREECIV21_VERSION_MINOR ${FC21_MINOR_VERSION})
set(FREECIV21_VERSION_PATCH ${FC21_PATCH_VERSION})
set(FREECIV21_VERSION "${FREECIV21_VERSION_MAJOR}.${FREECIV21_VERSION_MINOR}.${FREECIV21_VERSION_PATCH}")

# Set project
project(freeciv21 VERSION ${FREECIV21_VERSION} LANGUAGES C CXX)

# Gather all the tailored settings we need for Windows builds early.
if(WIN32 OR MSYS OR MINGW)
  # We need to alter the out of box values of these variables for Win32 et al builds
  set(CMAKE_INSTALL_DATAROOTDIR ".")
  set(CMAKE_INSTALL_BINDIR ".")
  set(PROJECT_NAME "data")
  set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/")
  get_filename_component(MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
endif()

add_compile_definitions(PUBLIC $<$<CONFIG:Debug>:FREECIV_DEBUG>)

# After project() because the list of languages has to be known
include(FreecivDependencies)
include(FreecivHelpers)

# Put all executables at the root of the build tree
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# We do this after all targets with third-party code have been created, so
#   the options only apply to code we own.
include(EnableCompilerWarnings)

# Include subdirectories with the actual project definitions
add_subdirectory(utility)
add_subdirectory(common)
add_subdirectory(data)

if (FREECIV_BUILD_LIBCLIENT)
  add_subdirectory(client)
endif()
if (FREECIV_BUILD_LIBSERVER)
  add_subdirectory(server)
endif()

# Always add tools, conditionals are inside
add_subdirectory(tools)

# Add docs
add_subdirectory(docs)

# Use Auto Revision variables to convert some templates to real files at build time
if(NOT EXISTS "${CMAKE_BINARY_DIR}/utility/fc_version.h")
  configure_file(${CMAKE_SOURCE_DIR}/utility/fc_version.h.in
                utility/fc_version.h
                @ONLY NEWLINE_STYLE UNIX)
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/fc_version")
  configure_file(${CMAKE_SOURCE_DIR}/fc_version.in
                fc_version
                @ONLY NEWLINE_STYLE UNIX)
endif()

#############################################
# Installation configuration only below here.
#############################################

# Always install the base documentation
install(
  FILES
  AUTHORS
  COPYING
  INSTALL
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  COMPONENT freeciv21)

# Always install the Licenses
install(
  FILES
  dist/licenses/0-MSYS2-INDEX.txt
  dist/licenses/APACHE-2.txt
  dist/licenses/BSD-2-CLAUSE.txt
  dist/licenses/BSD-3-CLAUSE.txt
  dist/licenses/BZ2.txt
  dist/licenses/FTL.txt
  dist/licenses/GPL2.txt
  dist/licenses/GPL3.txt
  dist/licenses/IMAGEMAGICK.txt
  dist/licenses/LGPL-2.0-ONLY.txt
  dist/licenses/LGPL-2.0-OR-LATER.txt
  dist/licenses/LGPL-3.0-ONLY.txt
  dist/licenses/LICENSEREF-KDE-ACCEPTED-LGPL.txt
  dist/licenses/MAGICKWAND.txt
  dist/licenses/MIT.txt
  dist/licenses/OPENSSL.txt
  dist/licenses/PNG.txt
  dist/licenses/UNICODE.txt
  dist/licenses/ZLIB.txt
  DESTINATION ${CMAKE_INSTALL_DOCDIR}/licenses
  COMPONENT freeciv21)

# Common installation for all Win32 et al platforms
if(WIN32 OR MSYS OR MINGW)
  # Custom command files to run the applications
  install(
    FILES
    dist/freeciv21-server.cmd
    dist/client.ico
    dist/mp.ico
    dist/server.ico
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT freeciv21)
endif()

# MSYS2 and MINGW specific installation
if(MSYS OR MINGW)
  # Install OpenSSL library, not found with GET_RUNTIME_DEPENDENCIES
  if("$ENV{MSYSTEM}" STREQUAL "MINGW32")
    install(
      FILES
      ${MINGW_PATH}/libcrypto-1_1.dll
      ${MINGW_PATH}/libssl-1_1.dll
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT freeciv21)
  else()
    install(
      FILES
      ${MINGW_PATH}/libcrypto-1_1-x64.dll
      ${MINGW_PATH}/libssl-1_1-x64.dll
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT freeciv21)
  endif()

  # This allows us to determine the external libraries we need to include at install time
  #   dynamically instead of doing it manually.
  install(CODE [[
    message(STATUS "Collecting dependencies for freeciv21 executables...")
    set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL objdump)

    # Take a variable that is available at "install" time and repurpose
    string(REGEX REPLACE "objdump.exe" "" MINGW_PATH ${CMAKE_OBJDUMP})

    # Function to analyze the third party dll files linked to the exe's
    #   Uses the repurposed variable from above to tell the function where
    #   the dll files are located. Ignores dll's that come with Windows.
    file(GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR r_deps
      UNRESOLVED_DEPENDENCIES_VAR u_deps
      DIRECTORIES ${MINGW_PATH}
      PRE_EXCLUDE_REGEXES "^api-ms-*"
      POST_EXCLUDE_REGEXES "C:[\\\\/][Ww][Ii][Nn][Dd][Oo][Ww][Ss][\\\\/].*"
      EXECUTABLES
        "${CMAKE_INSTALL_PREFIX}/freeciv21-*.exe"
      )
      message(STATUS "Installing library dependencies for freeciv21 executables...")
      file(INSTALL DESTINATION ${CMAKE_INSTALL_PREFIX} MESSAGE_LAZY FILES ${r_deps})
    ]] COMPONENT freeciv21)

  # Qt5 Plugins and required DLLs
  #   Before installation, run a series of commands that copy each of the Qt
  #   runtime files to the appropriate directory for installation
  install(CODE [[

    message(STATUS "Collecting Qt dependencies for freeciv21 GUI executables...")

    # Take a variable that is available at "install" time and repurpose
    string(REGEX REPLACE "objdump.exe" "" MINGW_PATH ${CMAKE_OBJDUMP})

    # Run Qt's windeployqt.exe to find the required DLLs for the GUI apps.
    execute_process(
      COMMAND ${MINGW_PATH}/windeployqt.exe --no-translations --no-virtualkeyboard --no-compiler-runtime
        --no-webkit2 --no-angle --no-opengl-sw --list mapping ${CMAKE_INSTALL_PREFIX}
      OUTPUT_VARIABLE _output
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Run a loop to go thought the output and copy the files we need
    message(STATUS "Installing Qt library dependencies for freeciv21 GUI executables...")
    separate_arguments(_files WINDOWS_COMMAND ${_output})
      while(_files)
          list(GET _files 0 _src)
          list(GET _files 1 _dest)
          execute_process(
            COMMAND cp ${_src} "${CMAKE_INSTALL_PREFIX}/${_dest}"
          )
          message(STATUS "Installing: ${CMAKE_INSTALL_PREFIX}/${_dest}")
          list(REMOVE_AT _files 0 1)
      endwhile()
    ]] COMPONENT freeciv21)
elseif(WIN32)
  # The Visual Studio generator places all files and associated DLL libraries
  #  into a build directory. So we just grab those for install.
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT freeciv21
    FILES_MATCHING PATTERN *.dll PATTERN *.pdb)

  # QT platform plugin must be only one directory level away from the EXEs
  #   so we ensure that here. This does create a duplicate file in install
  #   directory, but since we are not packaging in Visual Studio it isn't
  #   issue.
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/plugins/platforms
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT freeciv21
    FILES_MATCHING PATTERN *.dll)

  # Grab a few files that get missed
  install(
    FILES
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/SDL2.dll
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT freeciv21)
endif()

# Unix/Linux specific install steps
if(UNIX AND NOT APPLE)
  # Install MetaInfo and Desktop files for the applications asked for at configure
  if(FREECIV_ENABLE_CLIENT)
    install(
      FILES
      dist/net.longturn.freeciv21.client.metainfo.xml
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
      COMPONENT freeciv21
    )
    configure_file(${CMAKE_SOURCE_DIR}/dist/net.longturn.freeciv21.client.desktop.in
                   net.longturn.freeciv21.client.desktop
                   @ONLY NEWLINE_STYLE UNIX)
    install(
      FILES
      ${CMAKE_BINARY_DIR}/net.longturn.freeciv21.client.desktop
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
      COMPONENT freeciv21
    )
  endif(FREECIV_ENABLE_CLIENT)

  if(FREECIV_ENABLE_SERVER)
    install(
      FILES
      dist/net.longturn.freeciv21.server.metainfo.xml
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
      COMPONENT freeciv21
    )
    configure_file(${CMAKE_SOURCE_DIR}/dist/net.longturn.freeciv21.server.desktop.in
                   net.longturn.freeciv21.server.desktop
                   @ONLY NEWLINE_STYLE UNIX)
    install(
      FILES
      ${CMAKE_BINARY_DIR}/net.longturn.freeciv21.server.desktop
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
      COMPONENT freeciv21
    )
  endif(FREECIV_ENABLE_SERVER)

  if(FREECIV_ENABLE_FCMP_QT)
    install(
      FILES
      dist/net.longturn.freeciv21.modpack.metainfo.xml
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
      COMPONENT freeciv21
    )
    configure_file(${CMAKE_SOURCE_DIR}/dist/net.longturn.freeciv21.modpack-qt.desktop.in
                   net.longturn.freeciv21.modpack-qt.desktop
                   @ONLY NEWLINE_STYLE UNIX)
    install(
      FILES
      ${CMAKE_BINARY_DIR}/net.longturn.freeciv21.modpack-qt.desktop
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
      COMPONENT freeciv21
    )
  endif(FREECIV_ENABLE_FCMP_QT)

 if(FREECIV_ENABLE_RULEDIT)
    install(
      FILES
      dist/net.longturn.freeciv21.ruledit.metainfo.xml
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
      COMPONENT tool_ruledit
    )
    configure_file(${CMAKE_SOURCE_DIR}/dist/net.longturn.freeciv21.ruledit.desktop.in
                   net.longturn.freeciv21.ruledit.desktop
                   @ONLY NEWLINE_STYLE UNIX)
    install(
      FILES
      ${CMAKE_BINARY_DIR}/net.longturn.freeciv21.ruledit.desktop
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
      COMPONENT tool_ruledit
    )
  endif(FREECIV_ENABLE_RULEDIT)
endif(UNIX AND NOT APPLE)

# Packaging (CPack)
include(CPackConfig)
