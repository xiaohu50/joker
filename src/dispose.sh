basepath=$(cd `dirname $0`; pwd -P)
source ${basepath}/config.sh

suspiciousDir=${logpath}/suspicious_caught
spamDir=${logpath}/spam_caught
checktagF=${logpath}/checktag

hour=`date "+%H"`
minute=`date "+%M"`
hour=`echo "ibase=10; obase=10; ${hour}"|bc`
minute=`echo "ibase=10; obase=10; ${minute}"|bc`
timetag=`echo $((${hour}*6 + ${minute}/10)) | awk '{printf "%05d", $1}'`
if [ -e ${urlfile} ]; then
        cp ${urlfile} ${urlfilepath}/url${timetag}
else
        echo "" > ${urlfilepath}/url${timetag}
fi

cmdcheck1="wget -b -i ${logpath}/url"
psinfo=`ps -ef | grep -v "grep" | grep "${cmdcheck1}"`
echo "$psinfo" | grep "${cmdcheck1}" > /dev/null 2>&1
[ $? -eq 0 ] && echo "${cmdcheck1} has run. $psinfo" && exit

cmdcheck2="/bin/sh ${jokerpath}/joke.sh"
psinfo=`ps -ef | grep -v "grep" | grep "${cmdcheck2}"`
echo "$psinfo" | grep "${cmdcheck2}" > /dev/null 2>&1
[ $? -eq 0 ] && echo "${cmdcheck2} has run. $psinfo" && exit

urlnow=${logpath}/url_bk
cat ${urlfilepath}/url00* > ${urlnow}
echo ${urlnow}

mkdir -p ${suspiciousDir}
mkdir -p ${spamDir}
rm -rf ${suspiciousDir}/*
rm -rf ${spamDir}/*
cd ${jokerpath}

if [ -e ${checktagF} ]; then
	checktag=`cat ${checktagF}`
	if [ ${checktag} == "left" ]; then
		echo "right" > ${checktagF}
	else
		echo "left" > ${checktagF}
	fi
else
	checktag="left"
	echo "right" > ${checktagF}
fi

for((index=${begin};index<=${end};index++))
do
	/bin/sh ${jokerpath}/joke.sh $checktag $index $cores $logpath $jokerpath ${urlnow} ${suspiciousDir} ${spamDir} ${timetag} &
done


wait

#bkpath should be in hard disk
#bkpath=${logpath}/backup

mkdir -p ${bkpath}
if [ -e ${logpath}/url_old ]; then
	python ${jokerpath}/getdetail.py ${logpath}/url_old ${suspiciousDir} ${spamDir}  ${bkpath}/${ip}_suspicious_detail_${timetag} ${bkpath}/${ip}_spam_detail_${timetag} 
fi

cp ${urlnow} ${logpath}/url_old


if [ -e ${bkpath}/${ip}_suspicious_detail_${timetag} ]; then
	rsync -r ${bkpath}/${ip}_suspicious_detail_${timetag} 10.75.6.86::SpamPic/suspicious_detail
fi
if [ -e ${bkpath}/${ip}_spam_detail_${timetag} ]; then
	rsync -r ${bkpath}/${ip}_spam_detail_${timetag} 10.75.6.86::SpamPic/spam_detail
fi





rm -f ${urlfilepath}/url00*
