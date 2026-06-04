include("C:/Projects/_busJamPuzzle/build/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/BusJamPuzzle-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtdeclarative")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "C:/Projects/_busJamPuzzle/build/BusJamPuzzle.exe"
    GENERATE_QT_CONF
)
