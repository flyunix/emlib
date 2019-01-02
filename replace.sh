#!/bin/bash

SUB_SEGM="pool_t;POOL_;pool_;caching_pool;"
SEGMENT="IDECL;INLINE;DECL;IDEF;DEF;size_t;ssize_t;bool_t;TRUE;FALSE;CHECK_STACK;list;str_t;sys_info;assert;bzero;${SUB_SEGM}" 
SED_EXP=

isUpper() 
{
    word="$1"
    echo ${word} | grep '^[A-Z]' >/dev/null

    if [[ "$?" == "0" ]]; then
        return 1;
    else
        return 0;
    fi
}

oldIFS=$IFS;
IFS=";"

for item in $SEGMENT;
do
    echo Item: "${item}"
    isUpper ${item}

    if [ $? -eq 1 ];then
        EXP="s/PJ_${item}/EM_${item}/I;"
    else 
        EXP="s/pj_${item}/em_${item}/i;"
    fi
    SED_EXP="${EXP}${SED_EXP}"
done

DATA_TYPE="int8;int32;int64;uint8;uint32;uint64"

for dtype in $DATA_TYPE;
do
    echo dtype: "${dtype}"
    SED_EXP="s/pj_${dtype}_t/${dtype}/i;${SED_EXP}"
done

IFS=${oldIFS}
echo -e "SED_EXP: ${SED_EXP}\n"

find ./ \( -name ".git" -prune -o -name "CMakeFiles" -prune -o -name "replace.sh" -prune \) -o \( -type f -print0 \) | xargs -0 -I {} sed -i "${SED_EXP}" {}
