#!/bin/sh

read -p "Enter applet's name : " AppletName
if test -e $AppletName; then
	echo "Directory $AppletName already exists here; delete it before."
	exit 1
fi
read -p "Enter your name : " MyName
read -p "Enter an e-mail adress to contact you for bugs or congratulations : " MyMail
read -p "Enter the default label of your applet (Just type enter to not have a label) :" AppletLabel
read -p "Enter the default icon of your applet (Just type enter if you'll draw it dynamically) :" AppletIcon


echo "creation de l'arborescence de l'applet $AppletName ..."
cp -r template $AppletName


cd $AppletName
sed "s/CD_APPLET_NAME/$AppletName/g" configure.ac > tmp
sed "s/CD_MY_NAME/$MyName/g" tmp > configure.ac
sed "s/CD_MY_MAIL/$MyMail/g" configure.ac > tmp
mv tmp configure.ac


cd data
if test "x$AppletLabel" = "x"; then
	sed "/CD_APPLET_LABEL/d" template.conf.in > tmp
else
	sed "s/CD_APPLET_LABEL/$AppletLabel/g" template.conf.in > tmp
fi
if test "$AppletIcon" = "x"; then
	sed "/CD_APPLET_ICON/d" tmp > template.conf.in
else
	sed "s/CD_APPLET_ICON/$AppletIcon/g" tmp > template.conf.in
fi
mv template.conf.in "$AppletName.conf.in"

sed "s/CD_APPLET_NAME/$AppletName/g" readme > tmp
sed "s/CD_MY_NAME/$MyName/g" tmp > readme

sed "s/CD_APPLET_NAME/$AppletName/g" Makefile.am > tmp
mv tmp Makefile.am
rm -f tmp


cd ../src
sed "s/CD_APPLET_NAME/$AppletName/g" Makefile.am > tmp
mv tmp Makefile.am

sed "s/CD_APPLET_NAME/$AppletName/g" applet-init.c > tmp
mv tmp applet-init.c

if test "x$AppletLabel" = "x"; then
	sed "s/CD_APPLET_LABEL/NULL/g" applet-config.c > tmp
else
	sed "s/CD_APPLET_LABEL/\"$AppletLabel\"/g" applet-config.c > tmp
fi
if test "x$AppletLabel" = "x"; then
	sed "s/CD_APPLET_ICON/NULL/g" tmp > applet-config.c
else
	sed "s/CD_APPLET_ICON/\"$AppletIcon\"/g" tmp > applet-config.c
fi

sed "s/CD_APPLET_NAME/$AppletName/g" applet-notifications.c > tmp
sed "s/CD_MY_NAME/$MyName/g" tmp > applet-notifications.c
rm -f tmp


cd ../po
sed "s/CD_APPLET_NAME/$AppletName/g" fr.po > tmp
sed "s/CD_MY_NAME/$MyName/g" tmp > fr.po
rm -f tmp
