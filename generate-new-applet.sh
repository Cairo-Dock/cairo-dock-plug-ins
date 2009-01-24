#!/bin/sh

read -p "Enter applet's name : " AppletName
if test -e $AppletName; then
	echo "Directory $AppletName already exists here; delete it before."
	exit 1
fi
export LibName=`echo $AppletName | tr "-" "_"`
export UpperName=`echo $LibName | tr "[a-z]" "[A-Z]"`

read -p "Enter your name : " MyName
read -p "Enter an e-mail adress to contact you for bugs or congratulations : " MyMail
read -p "Enter the default label of your applet (Just type enter to leave it empty for the moment) :" AppletLabel
read -p "Will your applet draw its icon dynamically (like the clock or dustbin applets for exemple) [y/N] ?" AppletIcon


echo "creation de l'arborescence de l'applet $AppletName ..."
cp -r template $AppletName
find $AppletName -name ".svn" -execdir rm -rf .svn \; > /dev/null


cd $AppletName
sed -i "s/CD_APPLET_NAME/$AppletName/g" configure.ac
sed -i "s/CD_MY_NAME/$MyName/g" configure.ac
sed -i "s/CD_MY_MAIL/$MyMail/g" configure.ac
sed -i "s/CD_PKG/$UpperName/g" configure.ac

cd data
if test "x$AppletLabel" = "x"; then
	sed -i "s/CD_APPLET_LABEL/$AppletName/g" template.conf.in
else
	sed -i "s/CD_APPLET_LABEL/$AppletLabel/g" template.conf.in
fi
if test "x$AppletIcon" = "xy" -o "x$AppletIcon" = "xY"; then
	sed -i "/Icon's name/{N;N;d}" template.conf.in
fi
sed -i "s/CD_PKG/$UpperName/g" template.conf.in
mv template.conf.in "$AppletName.conf.in"

sed -i "s/CD_APPLET_NAME/$AppletName/g" readme.in
sed -i "s/CD_MY_NAME/$MyName/g" readme.in

sed -i "s/CD_APPLET_NAME/$AppletName/g" Makefile.am
sed -i "s/CD_PKG/$UpperName/g" readme.in
sed -i "s/CD_PKG/$UpperName/g" readme.in

cd ../src
sed -i "s/CD_APPLET_NAME/$AppletName/g" Makefile.am
sed -i "s/CD_LIB_NAME/$LibName/g" Makefile.am
sed -i "s/CD_PKG/$UpperName/g" Makefile.am

sed -i "s/CD_APPLET_NAME/$AppletName/g" applet-init.c

sed -i "s/CD_APPLET_NAME/$AppletName/g" applet-notifications.c
sed -i "s/CD_MY_NAME/$MyName/g" applet-notifications.c
sed -i "s/CD_PKG/$UpperName/g" Makevars
sed -i "s/CD_PKG/$UpperName/g" Makefile.in.in


cd ../po
sed -i "s/CD_APPLET_NAME/$AppletName/g" fr.po
sed -i "s/CD_MY_NAME/$MyName/g" fr.po


cd ..

autoreconf -isvf && ./configure --prefix=/usr && make

cd po
../../../cairo-dock/po/generate-translation.sh

echo "now it's your turn ! type 'sudo make install' to install it."
