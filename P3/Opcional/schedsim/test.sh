#!/bin/bash
directorio="res"
ok=false
while [ "$ok" != "true" ]
do
	echo "Ponga el fichero que desea leer: "
	read fichero
	fichero=("./examples/"$fichero)
		if [ -f "$fichero" ]
		then
		ok=true
		else
			echo "El fichero que desea leer no existe, repitalo."
		fi
done
ok=false
while [ "$ok" != "true" ]
do
	echo "Maximo de CPU's en la ejecucion: "
	read maxCPU
		if (( maxCPU >= 1 &&  maxCPU <= 8 )); then
			ok=true
		else
			echo "Entre 1 y 8, repitalo de nuevo."
		fi
done
if [ -d "$directorio" ]; then
	rm -rf -- $directorio 
fi
mkdir $directorio 
i=0
for nameSched in $(./schedsim -L);
	do
	if (( i < 2 )); then #leemos las dos primeras lineas de cabecera.
	echo "Leyendo cabezera... $i "
	else
	echo "Ejecutando el algoritmo $nameSched "
	for cpus in $(seq 1 $maxCPU);
		do
		./schedsim -n $cpus -i $fichero -s $nameSched -p 1
		((cp = cpus - 1))
		for j in $(seq 0 $cp);
			do
			a=(CPU_$j.log)
			b=($directorio"/"$nameSched"__CPU_"$j.log)
			mv $a $b
			cd "../gantt-gplot/"
			chmod +x ./generate_gantt_chart "../schedsim/"$b
			cd "../schedsim/"
			done
		done
	fi
	((i+=1))
	done
exit 0
