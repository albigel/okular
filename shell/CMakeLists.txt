
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/../
)

# okular

set(okular_SRCS
   main.cpp
   okular_main.cpp
   shell.cpp
   shellutils.cpp
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../ui/data/icons/*-apps-okular.png")
ecm_add_app_icon(okular_SRCS ICONS ${ICONS_SRCS})

add_executable(okular ${okular_SRCS})

target_link_libraries(okular KF5::Parts KF5::WindowSystem KF5::Crash)
if(TARGET KF5::Activities)
    target_compile_definitions(okular PUBLIC -DWITH_KACTIVITIES=1)

	target_link_libraries(okular KF5::Activities)
endif()

install(TARGETS okular ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


# okular shell data files

install( PROGRAMS org.kde.okular.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES shell.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/okular )
install( FILES org.kde.okular.appdata.xml DESTINATION  ${KDE_INSTALL_METAINFODIR} )
