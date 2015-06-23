checktag=$1
index=$2
cores=$3
logpath=$4
jokerpath=$5
urlfile=$6
suspiciousDir=$7
spamDir=$8
timetag=$9

pic_cache_left=${logpath}/pic_cache_left${index}
pic_cache_right=${logpath}/pic_cache_right${index}
tmpdir=${logpath}/tmp_cache${index}
mkdir -p ${tmpdir}
mkdir -p $pic_cache_left
mkdir -p $pic_cache_right
rm -rf ${tmpdir}/*

echo $checktag, $index, $cores, $logpath, $jokerpath

cd ${jokerpath}
python geturl.py $index $cores $urlfile ${logpath}/url${index}

if [ $checktag == "left" ]
then
	pic_write=${pic_cache_left}
	pic_read=${pic_cache_right}
else
	pic_write=${pic_cache_right}
	pic_read=${pic_cache_left}
fi

echo "write in ${pic_write}"
echo "read in ${pic_read}"

cd ${pic_write}
rm -rf ./*
wget -b -i ${logpath}/url${index} -o wget.log


cd ${jokerpath}
bad_uid=${logpath}/bad_uid${index}
spam_dict=${jokerpath}/spam.dict
python deal.py $pic_read $tmpdir $spam_dict $bad_uid ${suspiciousDir} ${spamDir}

