workpath=/home/xiaohu_v/github/joker/src
pic_in=/home/xiaohu_v/github/joker/pic_in
pic_out=/home/xiaohu_v/github/joker/pic_out
tmpdir=/home/xiaohu_v/github/joker/tmpd

mkdir -p $tmpdir
cd $workpath
python2.7 deal.py $pic_in $pic_out $tmpdir
