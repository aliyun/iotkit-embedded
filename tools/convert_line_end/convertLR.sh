#!/bin/bash  
  
if [[ -z "$1" ]] || [[ ! -d "$1" ]]; then  
    echo "The directory is empty or not exist!"  
    echo "It will use the current directory."  
    nowdir=$(pwd)  
else  
    nowdir=$(cd $1; pwd)  
fi  
echo "$nowdir"  
  
function SearchCfile()  
{  
    cd $1  
    cfilelist=$(ls -l | grep "^-" | awk '{print $9}')  
    for cfilename in $cfilelist  
    do
        if [ "${cfilename#*.}" = "c" ] || [ "${cfilename#*.}" = "h" ]; then
            unix2dos $cfilename
           # dos2unix $cfilename
        fi
    done

    dirlist=$(ls)  
    for dirname in $dirlist 
    do  
        if [[ -d "$dirname" ]];then  
            cd $dirname  
            SearchCfile $(pwd)  
            cd ..  
        fi;  
    done;  
} 
  
SearchCfile $nowdir


