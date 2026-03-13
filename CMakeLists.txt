cmake_minimum_required(VERSION 3.19)
project(Kala LANGUAGES C CXX)

# Fetch RTAudio from GitHub
include(FetchContent)
FetchContent_Declare(
    rtaudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG 6.0.1
)
set(RTAUDIO_API_WASAPI ON CACHE BOOL "Build WASAPI API" FORCE)
set(RTAUDIO_API_ASIO OFF CACHE BOOL "Build ASIO API" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
FetchContent_MakeAvailable(rtaudio)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent Network)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(Kala
    WIN32 MACOSX_BUNDLE
    main.cpp
    kalamain.cpp
    kalamain.h
    kalamain.ui
    sounitbuilder.ui
    container.ui
    sounitbuilder.h sounitbuilder.cpp
    container.h container.cpp
    canvas.h canvas.cpp
    scorecanvas.ui
    scorecanvaswindow.h scorecanvaswindow.cpp
    trackselect.h trackselect.cpp
    scorecanvas.h scorecanvas.cpp
    scorecanvascommands.h scorecanvascommands.cpp
    sounitbuildercommands.h sounitbuildercommands.cpp
    timeline.h timeline.cpp
    frequencylabels.h frequencylabels.cpp
    note.h note.cpp
    vibrato.h
    vibratoeditordialog.h vibratoeditordialog.cpp
    phrase.h phrase.cpp
    dynamicscurvedialog.h dynamicscurvedialog.cpp
    expressivecurveapplydialog.h expressivecurveapplydialog.cpp
    curve.h curve.cpp
    scale.h scale.cpp
    harmonicgenerator.h harmonicgenerator.cpp
    audioengine.h audioengine.cpp
    track.h track.cpp
    trackmanager.h trackmanager.cpp
    rendersegment.h
    noterender.h
    spectrum.h spectrum.cpp
    spectrumtosignal.h spectrumtosignal.cpp
    signalmixer.h signalmixer.cpp
    rolloffprocessor.h rolloffprocessor.cpp
    spectrumblender.h spectrumblender.cpp
    formantbody.h formantbody.cpp
    breathturbulence.h breathturbulence.cpp
    noisecolorfilter.h noisecolorfilter.cpp
    physicssystem.h physicssystem.cpp
    envelopeengine.h envelopeengine.cpp
    driftengine.h driftengine.cpp
    gateprocessor.h gateprocessor.cpp
    easing.h easing.cpp
    easingapplicator.h easingapplicator.cpp
    easingdialog.h easingdialog.cpp easingdialog.ui
    easingpreviewwidget.h easingpreviewwidget.cpp
    karplusstrongattack.h karplusstrongattack.cpp
    attackgenerator.h attackgenerator.cpp
    lfo.h lfo.cpp
    frequencymapper.h frequencymapper.cpp
    wavetablesynth.h wavetablesynth.cpp
    bandpasseq.h bandpasseq.cpp
    combfilter.h combfilter.cpp
    lowhighpassfilter.h lowhighpassfilter.cpp
    irconvolution.h irconvolution.cpp
    recordermodel.h recordermodel.cpp
    bowedmodel.h bowedmodel.cpp
    saxophonemodel.h saxophonemodel.cpp
    tailprocessor.h tailprocessor.cpp
    sounitgraph.h sounitgraph.cpp
    sounitvariation.h sounitvariation.cpp
    spectrumvisualizer.h spectrumvisualizer.cpp
    envelopevisualizer.h envelopevisualizer.cpp
    dnaeditordialog.h dnaeditordialog.cpp
    envelopelibraryDialog.h envelopelibraryDialog.cpp
    envelopecurvecanvas.h envelopecurvecanvas.cpp
    compositionsettings.h compositionsettings.cpp
    compositionsettingsdialog.h compositionsettingsdialog.cpp
    gotodialog.h gotodialog.cpp
    keyboardshortcutsdialog.h keyboardshortcutsdialog.cpp
    scaledialog.h scaledialog.cpp scaledialog.ui
    tempotimesignature.h tempotimesignature.cpp
    tempodialog.h tempodialog.cpp tempodialog.ui
    tempochangedialog.h tempochangedialog.cpp tempochangedialog.ui
    exportaudiodialog.h exportaudiodialog.cpp
    rendercache.h rendercache.cpp
    containersettings.h containersettings.cpp
    helpdialog.h helpdialog.cpp
    audiodevicenotifier.h audiodevicenotifier.cpp
    dialogNewProject.ui
    llmconfig.h
    llmclient.h llmclient.cpp
    kalatools.h kalatools.cpp
    kalaagent.h kalaagent.cpp
    companionpanel.h companionpanel.cpp

)

# FFTW3 (single-precision float build, pre-built DLL for Windows/MinGW)
set(FFTW3_DIR "${CMAKE_SOURCE_DIR}/fftw-3.3.5-dll64")
target_include_directories(Kala PRIVATE "${FFTW3_DIR}")
target_link_libraries(Kala PRIVATE "${FFTW3_DIR}/libfftw3f.dll.a")

# Copy FFTW DLL to build output directory
add_custom_command(TARGET Kala POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFTW3_DIR}/libfftw3f-3.dll"
        "$<TARGET_FILE_DIR:Kala>"
    COMMENT "Copying libfftw3f-3.dll to build output..."
)

target_link_libraries(Kala
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Concurrent
        Qt::Network
        rtaudio
)
target_link_libraries(Kala PRIVATE Qt6::Widgets)
target_link_libraries(Kala PRIVATE Qt6::Widgets)
target_link_libraries(Kala PRIVATE Qt6::Widgets)
target_link_libraries(Kala PRIVATE Qt6::Widgets)

# Link ole32 for COM (IMMNotificationClient device change detection)
if(WIN32)
    target_link_libraries(Kala PRIVATE ole32)
endif()

# Windows application icon - compile RC manually to avoid include path issues
if(WIN32)
    set(RC_OBJ "${CMAKE_CURRENT_BINARY_DIR}/kala_icon.obj")
    add_custom_command(
        OUTPUT ${RC_OBJ}
        COMMAND windres "${CMAKE_SOURCE_DIR}/kala.rc" -o "${RC_OBJ}"
        DEPENDS "${CMAKE_SOURCE_DIR}/kala.rc" "${CMAKE_SOURCE_DIR}/kala.ico"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM
    )
    target_sources(Kala PRIVATE ${RC_OBJ})
endif()

# Copy help documentation to build output
add_custom_command(TARGET Kala POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/help"
        "$<TARGET_FILE_DIR:Kala>/help"
    COMMENT "Copying help documentation..."
)

include(GNUInstallDirs)

install(TARGETS Kala
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Kala
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
