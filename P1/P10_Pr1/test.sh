DIRECTORIO_TMP=tmp
DIRECTORIO_SALIDA=out
FICHERO_MTAR=mytar.mtar
A=a.txt
B=b.txt
C=c.dat
echo "En mi consola no me funciona el sh, si funciona aquí bien, sino compilarlo paso a paso que así si funciona."
if [ ! -e ./mytar ]; then
     echo "[ ERROR ] No encontramos el archivo mytar"
    exit -1 
fi
if [ ! -x ./mytar ]; then
	echo "[ ERROR ] No podemos ejecutar"
    exit -1 
fi
if [ -d "$DIRECTORIO_TMP" ]; then
	rm -rf -- $DIRECTORIO_TMP  
	echo "$DIRECTORIO_TMP borrado [OK]"
fi
mkdir $DIRECTORIO_TMP 
cd $DIRECTORIO_TMP
echo "$DIRECTORIO_TMP nuevo creado."
if [ ! -e $A ]; then
	touch $A
	echo "Hello World!" > $A
  	echo "$A creado. [OK]"
fi
if [ ! -e $B ]; then
	touch $B 
	head -10 /etc/passwd > $B 
  	echo "$B creado. [OK]"
fi
if [ ! -e $C ]; then
	touch $C
	head -c 1024 /dev/urandom > $C
  	echo "$C creado. [OK]"
fi
./../mytar -c -f $FICHERO_MTAR $A $B $C
echo "Creando $FICHERO_MTAR ."
if [ ! -d $DIRECTORIO_SALIDA ]; then
	mkdir $DIRECTORIO_SALIDA 
	echo "Nuevo directorio de salida creado. [OK]"
fi
cp ./$FICHERO_MTAR ./$DIRECTORIO_SALIDA/$FICHERO_MTAR
cd $DIRECTORIO_SALIDA
./../../mytar -x -f $FICHERO_MTAR
echo "Extrayendo archivos en $DIRECTORIO_SALIDA. "
echo "Checkeo final: "
if diff ../$A $A >/dev/null ; then
  	echo "$A correcto. [OK]"
else
  	echo "$A fallo. [ ERROR | EXIT]"
  	exit -1
fi
if diff ../$B $B >/dev/null ; then
  	echo "$B correcto. [OK]"
else
  	echo "$B fallo. [ ERROR | EXIT ]"
  	exit -1
fi
if diff ../$C $C >/dev/null ; then
  	echo "$C correcto. [OK]"
else
  	echo "$C fallo. [ ERROR | EXIT ]"
	exit -1
fi
echo "Correcto todo. Test finalizado."
exit 0