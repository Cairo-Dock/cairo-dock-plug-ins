#!/bin/sh

read -p "Enter applet's name : " AppletName
if test -e "$AppletName"; then
	echo "Directory $AppletName already exists here; delete it before."
	exit 1
fi
export LibName=`echo $AppletName | tr "-" "_"`
export UpperName=`echo $LibName | tr "[a-z]" "[A-Z]"`
export LowerName=`echo $LibName | tr "[A-Z]" "[a-z]"`

read -p "Enter your name : " MyName
read -p "Will your applet draw its icon dynamically (like the clock or dustbin applets for exemple) [y/N] ?" AppletIcon


echo "creation de l'arborescence de l'applet $AppletName ..."
cp -r template "$AppletName"
find "$AppletName" -name ".bzr" -execdir rm -rf .bzr \; > /dev/null


cd "$AppletName"

cd data
sed -i "s/CD_APPLET_NAME/$AppletName/g" CMakeLists.txt
sed -i "s/pkgdatadir/${LowerName}datadir/g" CMakeLists.txt

sed -i "s/CD_APPLET_LABEL/$AppletName/g" template.conf.in
if test "x$AppletIcon" = "xy" -o "x$AppletIcon" = "xY"; then
	sed -i "/Image filename/{N;N;d}" template.conf.in
fi
sed -i "s/CD_PKG/$UpperName/g" template.conf.in
sed -i "s/CD_APPLET_NAME/$AppletName/g" template.conf.in
mv template.conf.in "$AppletName.conf.in"


cd ../src
sed -i "s/CD_PKG/$UpperName/g" CMakeLists.txt
sed -i "s/pkgdatadir/${LowerName}datadir/g" CMakeLists.txt
sed -i "s/pkguserdirname/${LowerName}userdirname/g" CMakeLists.txt

sed -i "s/CD_APPLET_NAME/$AppletName/g" applet-init.c
sed -i "s/CD_MY_NAME/$MyName/g" applet-init.c
if test "x$AppletIcon" = "xy" -o "x$AppletIcon" = "xY"; then
	sed -i "/CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE/d" applet-init.c
fi


cd ../..

echo "" >> CMakeLists.txt
echo "############# $UpperName #################" >> CMakeLists.txt
echo "if (\"\${enable-${LowerName}}\" STREQUAL \"yes\")" >> CMakeLists.txt
echo "set (GETTEXT_$UpperName \${GETTEXT_PLUGINS})" >> CMakeLists.txt
echo "set (VERSION_$UpperName \"0.0.1\")" >> CMakeLists.txt
echo "set (PACKAGE_$UpperName \"cd-$AppletName\")" >> CMakeLists.txt
echo "set (logoutdatadir \"\${pluginsdatadir}/$AppletName\")" >> CMakeLists.txt
echo "configure_file ($AppletName/data/$AppletName.conf.in \${CMAKE_CURRENT_SOURCE_DIR}/$AppletName/data/$AppletName.conf)" >> CMakeLists.txt
echo "add_subdirectory ($AppletName)" >> CMakeLists.txt
echo "endif()" >> CMakeLists.txt

cmake -DCMAKE_INSTALL_PREFIX=/usr -D$AppletName=yes
make

echo "Applet $AppletName has been generated."
echo "Now its' your turn ! type 'sudo make install' to install your applet."
