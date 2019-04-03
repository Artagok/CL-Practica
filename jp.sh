#!/bin/bash

# CONSTANTS
ruta="../examples/"
_in=".in"
_out=".out"
_t=".t"
_err=".err"
_asl=".asl"
separator="===================="
red_color="\033[01;38;5;196m"
green_color="\033[01;38;5;118m"
no_color="\033[00m"


check_type() {

    ./asl $ruta$fitxer | egrep ^L > out.temp;
    diff $ruta$nom_fitxer$_err out.temp > diff.temp
    [[ $? == 0 ]] && 
        echo -e "${green_color}OK: NO DIFF!${no_color}\n" ||
        echo -e "${red_color}$(cat diff.temp)${no_color}\n"
}

print_verbose_mode() {
    
    echo -e "${separator} INPUT ${separator}\n"
    cat -n $ruta$fitxer; echo;
    echo -e "${separator} EXPECTED OUTPUT ${separator}\n"
    cat $ruta$nom_fitxer$_err; echo;
    echo -e "${separator} YOUR OUTPUT ${separator}\n"
    cat out.temp; echo;
}

clean() {

    rm -f out.temp diff.temp
}

echo "It is assumed you are running the script inside asl folder, paths are relative"
select fitxer in $(ls $ruta | grep $_asl);
do
    echo -e "You have selected $REPLY) ${fitxer}\n"
    nom_fitxer=${fitxer%.asl} 
        
    [[ -e asl ]] || { echo "asl executable doesn't exist, please make it" && exit 1; }

    # === CHECK TYPE ===
    if [[ $nom_fitxer =~ chkt ]]
    then
        check_type
    fi

    # === GEN CODE ===
    [[ $nom_fitxer =~ genc ]] && echo "GENC"

    # === VERBOSE MODE ===
    if [[ $# -gt 0 && $1 == "-v" ]]
    then
        print_verbose_mode
    fi

    clean
done
