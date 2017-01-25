
# When adding new QML files that should be built into the plugin,
# add them to this variable and they will be listed into a generated
# resource file.
#
# To have the plugin register them as types, add an entries to the
# qmldir array in qt3dquick3dextrasplugin.cpp
QML_FILES = \
    $$PWD/qml/DefaultEffect.qml \
    $$PWD/qml/DefaultAlphaEffect.qml \
    $$PWD/qml/ForwardRenderer.qml \
    $$PWD/qml/SkyboxEntity.qml \
    $$PWD/qml/TextureMaterial.qml \
    $$PWD/qml/OrbitCameraController.qml \
    $$PWD/qml/FirstPersonCameraController.qml \
    $$PWD/qml/NormalDiffuseMapAlphaEffect.qml \
    $$PWD/qml/LevelOfDetailLoader.qml
