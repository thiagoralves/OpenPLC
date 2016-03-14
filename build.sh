#!/bin/bash
echo Building OpenPLC environment:
echo [OPLC_COMPILER]
cd OPLC_Compiler_source
g++ *.cpp -o OPLC_Compiler &> /dev/null
echo [LADDER]
cd ..
cp ./OPLC_Compiler_source/OPLC_Compiler ./
./OPLC_Compiler ./ladder_files/blank_ladder.ld &> /dev/null
rm -f ./core/ladder.cpp
mv ./ladder.cpp ./core/

echo [OPLC_STARTER]
cd OPLC_starter_source
g++ *.cpp -o OPLC_starter
cd ..
cp ./OPLC_starter_source/OPLC_starter ./

cd core
rm -f ./hardware_layer.cpp
echo The OpenPLC needs a driver to be able to control physical or virtual hardware.
echo Please select the driver you would like to use:
OPTIONS="Blank Fischertechnik RaspberryPi Simulink Unipi"
select opt in $OPTIONS; do
	if [ "$opt" = "Blank" ]; then
		cp ./hardware_layers/blank.cpp ./hardware_layer.cpp
		echo [OPENPLC]
		g++ -pthread *.cpp -o openplc
		exit
	elif [ "$opt" = "Fischertechnik" ]; then
		cp ./hardware_layers/fischertechnik.cpp ./hardware_layer.cpp
		echo [OPENPLC]
		g++ -lrt -lwiringPi -lpthread *.cpp -o openplc
		exit
	elif [ "$opt" = "RaspberryPi" ]; then
		cp ./hardware_layers/raspberrypi.cpp ./hardware_layer.cpp
		echo [OPENPLC]
		g++ -lrt -lwiringPi -lpthread *.cpp -o openplc
		exit
	elif [ "$opt" = "Simulink" ]; then
		cp ./hardware_layers/simulink.cpp ./hardware_layer.cpp
		echo [OPENPLC]
		g++ -pthread *.cpp -o openplc
		exit
	elif [ "$opt" = "Unipi" ]; then
		cp ./hardware_layers/unipi.cpp ./hardware_layer.cpp
		echo [OPENPLC]
		g++ -lrt -lwiringPi -lpthread *.cpp -o openplc
		exit
	else
		#clear
		echo bad option
	fi
done
