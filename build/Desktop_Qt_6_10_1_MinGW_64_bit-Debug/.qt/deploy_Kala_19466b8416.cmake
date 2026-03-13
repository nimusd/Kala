include("C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Kala-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug/Kala.exe"
    GENERATE_QT_CONF
)
