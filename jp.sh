#!/bin/bash
ruta="../examples/"
_in=".in"
_out=".out"
_t=".t"
_err=".err"
_asl=".asl"
separator="=============================="

echo "It is assumed you are running the script inside asl folder, paths are relative"
select fitxer in $(ls $ruta | grep $_asl);
do
    echo "You have selected $REPLY) $fitxer" 
    nom_fitxer=${fitxer%.asl} 
        
    [[ -e asl ]] || { echo "asl executable doesn't exist, please make it" && exit 1; }

    [[ $nom_fitxer =~ chkt ]] &&  ./asl $ruta$fitxer | egrep ^L > out.temp && diff $ruta$nom_fitxer$_err out.temp
    [[ $nom_fitxer =~ genc ]] && echo "GENC"

    # Verbose mode
    if [[ $# -gt 0 && $1 == "-v" ]]
    then
        echo ".ASL:"; cat -n $ruta$fitxer
        echo; echo $separator; echo;
        echo ".ERR:"; cat $ruta$nom_fitxer$_err
        echo; echo $separator; echo;
        echo ".TEMP:"; cat out.temp;
    fi
    rm -f out.temp
done
