# FindSpdlog
# -------
# Finds the Spdlog library
#
# This will define the following target:
#
#   ${APP_NAME_LC}::Spdlog   - The Spdlog library

macro(buildSpdlog)

  find_package(Fmt REQUIRED QUIET)

  if(APPLE)
    set(EXTRA_ARGS "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}")
  endif()

  if(WIN32 OR WINDOWS_STORE)
    set(patches "${CMAKE_SOURCE_DIR}/tools/depends/target/${${CMAKE_FIND_PACKAGE_NAME}_MODULE_LC}/001-windows-pdb-symbol-gen.patch")
    generate_patchcommand("${patches}")

    set(EXTRA_ARGS -DSPDLOG_WCHAR_SUPPORT=ON
                   -DSPDLOG_WCHAR_FILENAMES=ON)

    set(EXTRA_DEFINITIONS SPDLOG_WCHAR_FILENAMES
                          SPDLOG_WCHAR_TO_UTF8_SUPPORT)
  endif()

  set(SPDLOG_VERSION ${${${CMAKE_FIND_PACKAGE_NAME}_MODULE}_VER})
  # spdlog debug uses postfix d for all platforms
  set(${${CMAKE_FIND_PACKAGE_NAME}_MODULE}_DEBUG_POSTFIX d)

  set(CMAKE_ARGS -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
                 -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                 -DSPDLOG_BUILD_EXAMPLE=OFF
                 -DSPDLOG_BUILD_TESTS=OFF
                 -DSPDLOG_BUILD_BENCH=OFF
                 -DSPDLOG_FMT_EXTERNAL=ON
                 ${EXTRA_ARGS})

  # Set definitions that will be set in the built cmake config file
  # We dont import the config file if we build internal (chicken/egg scenario)
  set(_spdlog_definitions SPDLOG_COMPILED_LIB
                          SPDLOG_FMT_EXTERNAL
                          ${EXTRA_DEFINITIONS})

  BUILD_DEP_TARGET()

  add_dependencies(${${CMAKE_FIND_PACKAGE_NAME}_MODULE_LC} ${APP_NAME_LC}::Fmt)
endmacro()

if(NOT TARGET ${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME})
  include(cmake/scripts/common/ModuleHelpers.cmake)

  # If there is a potential this library can be built internally
  # Check its dependencies to allow forcing this lib to be built if one of its
  # dependencies requires being rebuilt
  if(ENABLE_INTERNAL_SPDLOG)
    # Dependency list of this find module for an INTERNAL build
    set(${CMAKE_FIND_PACKAGE_NAME}_DEPLIST Fmt)

    check_dependency_build(${CMAKE_FIND_PACKAGE_NAME} "${${CMAKE_FIND_PACKAGE_NAME}_DEPLIST}")
  endif()

  set(${CMAKE_FIND_PACKAGE_NAME}_MODULE_LC spdlog)
  SETUP_BUILD_VARS()

  # Check for existing SPDLOG. If version >= SPDLOG-VERSION file version, dont build
  find_package(SPDLOG CONFIG QUIET
                             HINTS ${DEPENDS_PATH}/lib/cmake
                             ${${CORE_PLATFORM_LC}_SEARCH_CONFIG})

  if((SPDLOG_VERSION VERSION_LESS ${${${CMAKE_FIND_PACKAGE_NAME}_MODULE}_VER} AND ENABLE_INTERNAL_SPDLOG) OR
     ((CORE_SYSTEM_NAME STREQUAL linux OR CORE_SYSTEM_NAME STREQUAL freebsd) AND ENABLE_INTERNAL_SPDLOG) OR
     (DEFINED ${CMAKE_FIND_PACKAGE_NAME}_FORCE_BUILD))

    buildSpdlog()
  else()
    if(TARGET spdlog::spdlog)
      # This is for the case where a distro provides a non standard (Debug/Release) config type
      # eg Debian's config file is spdlogConfigTargets-none.cmake
      # convert this back to either DEBUG/RELEASE or just RELEASE
      # we only do this because we use find_package_handle_standard_args for config time output
      # and it isnt capable of handling TARGETS, so we have to extract the info
      get_target_property(_SPDLOG_CONFIGURATIONS spdlog::spdlog IMPORTED_CONFIGURATIONS)
      foreach(_spdlog_config IN LISTS _SPDLOG_CONFIGURATIONS)
        # Some non standard config (eg None on Debian)
        # Just set to RELEASE var so select_library_configurations can continue to work its magic
        string(TOUPPER ${_spdlog_config} _spdlog_config_UPPER)
        if((NOT ${_spdlog_config_UPPER} STREQUAL "RELEASE") AND
           (NOT ${_spdlog_config_UPPER} STREQUAL "DEBUG"))
          get_target_property(SPDLOG_LIBRARY_RELEASE spdlog::spdlog IMPORTED_LOCATION_${_spdlog_config_UPPER})
        else()
          get_target_property(SPDLOG_LIBRARY_${_spdlog_config_UPPER} spdlog::spdlog IMPORTED_LOCATION_${_spdlog_config_UPPER})
        endif()
      endforeach()

      get_target_property(SPDLOG_INCLUDE_DIR spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
    else()
      find_package(PkgConfig QUIET)
      # Fallback to pkg-config and individual lib/include file search
      if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_SPDLOG spdlog QUIET)

        # Only add -D definitions. Skip -I include as we do a find_path for the header anyway
        foreach(_spdlog_cflag IN LISTS PC_SPDLOG_CFLAGS)
          if(${_spdlog_cflag} MATCHES "^-D(.*)")
            list(APPEND _spdlog_definitions ${CMAKE_MATCH_1})
          endif()
        endforeach()

        set(SPDLOG_VERSION ${PC_SPDLOG_VERSION})
      endif()

      find_path(SPDLOG_INCLUDE_DIR NAMES spdlog/spdlog.h
                                   HINTS ${DEPENDS_PATH}/include ${PC_SPDLOG_INCLUDEDIR}
                                   ${${CORE_PLATFORM_LC}_SEARCH_CONFIG})

      find_library(SPDLOG_LIBRARY_RELEASE NAMES spdlog
                                          HINTS ${DEPENDS_PATH}/lib ${PC_SPDLOG_LIBDIR}
                                          ${${CORE_PLATFORM_LC}_SEARCH_CONFIG})
      find_library(SPDLOG_LIBRARY_DEBUG NAMES spdlogd
                                        HINTS ${DEPENDS_PATH}/lib ${PC_SPDLOG_LIBDIR}
                                        ${${CORE_PLATFORM_LC}_SEARCH_CONFIG})
    endif()
  endif()

  include(SelectLibraryConfigurations)
  select_library_configurations(SPDLOG)
  unset(SPDLOG_LIBRARIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Spdlog
                                    REQUIRED_VARS SPDLOG_LIBRARY SPDLOG_INCLUDE_DIR
                                    VERSION_VAR SPDLOG_VERSION)

  if(Spdlog_FOUND)
    # cmake target and not building internal
    if(TARGET spdlog::spdlog AND NOT TARGET spdlog)
      add_library(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} ALIAS spdlog::spdlog)

    else()
      add_library(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} UNKNOWN IMPORTED)
      if(SPDLOG_LIBRARY_RELEASE)
        set_target_properties(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} PROPERTIES
                                                                         IMPORTED_CONFIGURATIONS RELEASE
                                                                         IMPORTED_LOCATION_RELEASE "${SPDLOG_LIBRARY_RELEASE}")
      endif()
      if(SPDLOG_LIBRARY_DEBUG)
        set_target_properties(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} PROPERTIES
                                                                         IMPORTED_LOCATION_DEBUG "${SPDLOG_LIBRARY_DEBUG}")
        set_property(TARGET ${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} APPEND PROPERTY
                                                                              IMPORTED_CONFIGURATIONS DEBUG)
      endif()
      set_target_properties(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} PROPERTIES
                                                                       INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}")

      if(_spdlog_definitions)
        # We need to append in case the cmake config already has definitions
        set_target_properties(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} PROPERTIES
                                                                         INTERFACE_COMPILE_DEFINITIONS "${_spdlog_definitions}")
      endif()
    endif()

    if(TARGET spdlog)
      add_dependencies(${APP_NAME_LC}::${CMAKE_FIND_PACKAGE_NAME} spdlog)
    endif()

    # Add internal build target when a Multi Config Generator is used
    # We cant add a dependency based off a generator expression for targeted build types,
    # https://gitlab.kitware.com/cmake/cmake/-/issues/19467
    # therefore if the find heuristics only find the library, we add the internal build
    # target to the project to allow user to manually trigger for any build type they need
    # in case only a specific build type is actually available (eg Release found, Debug Required)
    # This is mainly targeted for windows who required different runtime libs for different
    # types, and they arent compatible
    if(_multiconfig_generator)
      if(NOT TARGET spdlog)
        buildSpdlog()
        set_target_properties(spdlog PROPERTIES EXCLUDE_FROM_ALL TRUE)
      endif()
      add_dependencies(build_internal_depends spdlog)
    endif()
  else()
    if(Spdlog_FIND_REQUIRED)
      message(FATAL_ERROR "Spdlog libraries were not found. You may want to try -DENABLE_INTERNAL_SPDLOG=ON")
    endif()
  endif()
endif()
